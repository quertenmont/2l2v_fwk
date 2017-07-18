
//
// Pietro Vischia, <pietro.vischia@gmail.com>
//
// Lepton trigger/ID/iso efficiency studies, inspired by https://indico.cern.ch/event/367179/contribution/7/3/material/slides/0.pdf
//

#include <iostream>
#include <boost/shared_ptr.hpp>

#include "DataFormats/FWLite/interface/Handle.h"
#include "DataFormats/FWLite/interface/Event.h"
#include "DataFormats/FWLite/interface/ChainEvent.h"
#include "DataFormats/Common/interface/MergeableCounter.h"

//Load here all the dataformat that we will need
#include "SimDataFormats/PileupSummaryInfo/interface/PileupSummaryInfo.h"
#include "SimDataFormats/GeneratorProducts/interface/LHEEventProduct.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/PatCandidates/interface/Photon.h"
#include "DataFormats/PatCandidates/interface/MET.h"
#include "DataFormats/PatCandidates/interface/Tau.h"
#include "DataFormats/PatCandidates/interface/PackedTriggerPrescales.h"
#include "DataFormats/PatCandidates/interface/GenericParticle.h"

#include "CondFormats/JetMETObjects/interface/JetResolution.h"
#include "CondFormats/JetMETObjects/interface/JetCorrectionUncertainty.h"
#include "CondFormats/JetMETObjects/interface/JetCorrectorParameters.h"

#include "FWCore/FWLite/interface/AutoLibraryLoader.h"
#include "FWCore/PythonParameterSet/interface/MakeParameterSets.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "PhysicsTools/Utilities/interface/LumiReWeighting.h"
//#include "TauAnalysis/SVfitStandalone/interface/SVfitStandaloneAlgorithm.h" //for svfit

#include "UserCode/llvv_fwk/interface/MacroUtils.h"
#include "UserCode/llvv_fwk/interface/HiggsUtils.h"
#include "UserCode/llvv_fwk/interface/SmartSelectionMonitor.h"
#include "UserCode/llvv_fwk/interface/TMVAUtils.h"
#include "UserCode/llvv_fwk/interface/LeptonEfficiencySF.h"
#include "UserCode/llvv_fwk/interface/PDFInfo.h"
#include "UserCode/llvv_fwk/interface/MuScleFitCorrector.h"
#include "UserCode/llvv_fwk/interface/BtagUncertaintyComputer.h"
#include "UserCode/llvv_fwk/interface/GammaWeightsHandler.h"

#include "UserCode/llvv_fwk/interface/PatUtils.h"


#include "TSystem.h"
#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TProfile.h"
#include "TProfile2D.h"
#include "TEventList.h"
#include "TROOT.h"
#include "TNtuple.h"
#include <Math/VectorUtil.h>

using namespace std;


int main (int argc, char *argv[])
{
  //##############################################
  //########    GLOBAL INITIALIZATION     ########
  //##############################################

  // check arguments
  if (argc < 2)
    {
      std::cout << "Usage : " << argv[0] << " parameters_cfg.py" << std::endl;
      exit (0);
    }
  
  // load framework libraries
  gSystem->Load ("libFWCoreFWLite");
  AutoLibraryLoader::enable ();
  
  // configure the process
  const edm::ParameterSet & runProcess = edm::readPSetsFrom (argv[1])->getParameter < edm::ParameterSet > ("runProcess");

  bool debug = runProcess.getParameter<bool>("debug");
  bool isMC = runProcess.getParameter < bool > ("isMC");
  double xsec = runProcess.getParameter < double >("xsec");
  int mctruthmode = runProcess.getParameter < int >("mctruthmode");
  
  TString suffix = runProcess.getParameter < std::string > ("suffix");
  std::vector < std::string > urls = runProcess.getParameter < std::vector < std::string > >("input");
  TString baseDir = runProcess.getParameter < std::string > ("dirName");
  TString dtag=runProcess.getParameter<std::string>("dtag");
//  if (mctruthmode != 0) //FIXME
//    {
//      outFileUrl += "_filt";
//      outFileUrl += mctruthmode;
//    }
  TString outUrl = runProcess.getParameter<std::string>("outfile");
  
  bool
    filterOnlyEE       (false),
    filterOnlyMUMU     (false),
    filterOnlyEMU      (false),
    filterOnlySINGLEE  (false),
    filterOnlySINGLEMU (false);
  if (!isMC)
    {
      if (dtag.Contains ("DoubleEle")) filterOnlyEE       = true;
      if (dtag.Contains ("DoubleMu"))  filterOnlyMUMU     = true;
      if (dtag.Contains ("MuEG"))      filterOnlyEMU      = true;
      if (dtag.Contains ("SingleMu"))  filterOnlySINGLEMU = true;
      if (dtag.Contains ("SingleEle")) filterOnlySINGLEE  = true;
    }
  
  bool isSingleMuPD (!isMC && dtag.Contains ("SingleMu")); // Do I really need this?
  bool isV0JetsMC (isMC && (dtag.Contains ("DYJetsToLL_50toInf") || dtag.Contains ("WJets")));
  bool isWGmc (isMC && dtag.Contains ("WG"));
  bool isZGmc (isMC && dtag.Contains ("ZG"));
  bool isMC_ZZ = isMC && (string (dtag.Data ()).find ("TeV_ZZ") != string::npos);
  bool isMC_WZ = isMC && (string (dtag.Data ()).find ("TeV_WZ") != string::npos);
  
  TString outTxtUrl = outUrl + ".txt";
  FILE *outTxtFile = NULL;
  if (!isMC) outTxtFile = fopen (outTxtUrl.Data (), "w");
  printf ("TextFile URL = %s\n", outTxtUrl.Data ());
  
  //tree info
  TString dirname = runProcess.getParameter < std::string > ("dirName");
  
  //systematics
  bool runSystematics = runProcess.getParameter < bool > ("runSystematics");
  std::vector < TString > varNames (1, "");
  if (runSystematics)
    {
      varNames.push_back ("_jerup" ); varNames.push_back ("_jerdown" );
      varNames.push_back ("_jesup" ); varNames.push_back ("_jesdown" );
      varNames.push_back ("_umetup"); varNames.push_back ("_umetdown");
      varNames.push_back ("_lesup" ); varNames.push_back ("_lesdown" );
      varNames.push_back ("_puup"  ); varNames.push_back ("_pudown"  );
      varNames.push_back ("_btagup"); varNames.push_back ("_btagdown");
      if (isMC_ZZ)
        {
          varNames.push_back ("_zzptup"); varNames.push_back ("_zzptdown");
        }
      if (isMC_WZ)
        {
          varNames.push_back ("_wzptup"); varNames.push_back ("_wzptdown");
        }
    }
  
  size_t nvarsToInclude = varNames.size ();

  std::vector < std::string > allWeightsURL = runProcess.getParameter < std::vector < std::string > >("weightsFile");
  std::string weightsDir (allWeightsURL.size ()? allWeightsURL[0] : "");
  

  //##############################################
  //########    INITIATING HISTOGRAMS     ########
  //##############################################
  SmartSelectionMonitor mon;

  //generator level control : add an underflow entry to make sure the histo is kept
  //((TH1F*)mon.addHistogram( new TH1F( "higgsMass_raw",     ";Higgs Mass [GeV];Events", 500,0,1500) ))->Fill(-1.0,0.0001);

  //event selection
  TH1F *h = (TH1F *) mon.addHistogram (new TH1F ("eventflow", ";;Events", 6, 0, 6));
  h->GetXaxis ()->SetBinLabel (1, "#geq 2 iso leptons");
  h->GetXaxis ()->SetBinLabel (2, "M_{ll} veto");
  h->GetXaxis ()->SetBinLabel (3, "#geq 2 jets");
  h->GetXaxis ()->SetBinLabel (4, "E_{T}^{miss}");
  h->GetXaxis ()->SetBinLabel (5, "op. sign");
  h->GetXaxis ()->SetBinLabel (6, "#geq 2 b-tags");
  h = (TH1F *) mon.addHistogram (new TH1F ("eventflowslep", ";;Events", 6, 0, 6));
  h->GetXaxis ()->SetBinLabel (1, "1 iso lepton");
  h->GetXaxis ()->SetBinLabel (2, "#geq 2 jets");
  h->GetXaxis ()->SetBinLabel (3, "E_{T}^{miss}");
  h->GetXaxis ()->SetBinLabel (4, "#geq 1 b-tag");
  h->GetXaxis ()->SetBinLabel (5, "1 #tau");
  h->GetXaxis ()->SetBinLabel (6, "op. sign");


  // Setting up control categories
  std::vector < TString > controlCats;
  controlCats.clear ();
  controlCats.push_back("step1");
  controlCats.push_back("step2");
  controlCats.push_back("step3");
  controlCats.push_back("step4");
  controlCats.push_back("step5");
  controlCats.push_back("step6");
  
  for (size_t k = 0; k < controlCats.size (); ++k)
    {
      TString icat (controlCats[k]);

      //pu control to be completed
      mon.addHistogram (new TH1F (icat+"nvtx", ";Vertices;Events", 50, 0, 50));
      mon.addHistogram (new TH1F (icat+"nvtxraw", ";Vertices;Events", 50, 0, 50));
      mon.addHistogram (new TH1F (icat+"rho", ";#rho;Events", 50, 0, 25));
      

      //tau control to be completed
      TH1 *htaus = mon.addHistogram (new TH1F (icat + "ntaus", ";Tau multiplicity;Events", 5, 0, 5));
      for (int ibin = 1; ibin <= htaus->GetXaxis ()->GetNbins (); ibin++)
        {
          TString label ("");
          if (ibin == h->GetXaxis ()->GetNbins ())
            label += "#geq";
          else
            label += "=";
          label += (ibin - 1);
          htaus->GetXaxis ()->SetBinLabel (ibin, label);
        }
      mon.addHistogram( new TH1F(icat+"tauleadpt",     ";p_{T}^{#tau};Events", 50,0,500    ));
      mon.addHistogram( new TH1F(icat+"tauleadeta",    ";#eta^{#tau};Events",  50,-2.6,2.6 ));
      mon.addHistogram( new TH1F(icat+"taupt",         ";p_{T}^{#tau};Events", 50,0,500    ));
      mon.addHistogram( new TH1F(icat+"taueta",        ";#eta^{#tau};Events",  50,-2.6,2.6 ));
      mon.addHistogram( new TH1F(icat+"taucharge",     ";p_{T}^{#tau};Events", 5,-2,2      ));
      mon.addHistogram( new TH1F(icat+"taudz",         ";dz^{#tau};Events",    50,0,10     ));
      mon.addHistogram( new TH1F(icat+"tauvz",         ";vz^{#tau};Events",    50,0,10     ));
      mon.addHistogram( new TH1F(icat+"tauemfraction", ";emf^{#tau};Events",   50, 0., 5.  ));
      mon.addHistogram( new TH1F(icat+"taudizeta"    , ";dZ^{#tau};Events",    50, 0., 10. ));



      //lepton control
      mon.addHistogram( new TH1F(icat+"inclusivept", ";Transverse momentum [GeV];Events",               50, 0, 500    ));
      mon.addHistogram( new TH1F(icat+"leadpt",      ";Transverse momentum [GeV];Events",               50, 0, 500    ));
      mon.addHistogram( new TH1F(icat+"leadeta",     ";Pseudo-rapidity;Events",                         50, 0, 2.6    ));
      mon.addHistogram( new TH1F(icat+"trailerpt",   ";Transverse momentum [GeV];Events",               50, 0, 500    ));
      mon.addHistogram( new TH1F(icat+"trailereta",  ";Pseudo-rapidity;Events",                         50, 0, 2.6    ));
      mon.addHistogram( new TH1F(icat+"pte",          ";Electron transverse momentum [GeV];Events",     50,0,500      ));
      mon.addHistogram( new TH1F(icat+"ptmu",         ";Muon transverse momentum [GeV];Events",         50,0,500      ));
      mon.addHistogram( new TH1F(icat+"qt",           ";Transverse momentum [GeV];Events / (1 GeV)",    1500, 0, 1500 ));
      //mon.addHistogram( new TH1F(icat+"qtraw",        ";Transverse momentum [GeV];Events / (1 GeV)",    1500, 0, 1500 ));


      // Dilepton control
      mon.addHistogram( new TH1F(icat+"sumpt",        ";Sum of lepton transverse momenta [GeV];Events",                    50,0,500   ));
      mon.addHistogram( new TH1F(icat+"mll",          ";Dilepton invariant mass [GeV];Events",                             50,0,250   ));
      mon.addHistogram( new TH1F(icat+"ptll",         ";Dilepton transverse momentum [GeV];Events",                        100,0,1000 ));
      mon.addHistogram( new TH1F(icat+"yll",          ";Rapidity;Events",                               50, 0, 3      ));
      mon.addHistogram( new TH1F(icat+"dilarccosine", ";#theta(l,l') [rad];Events",                                        50,0,3.2   ));
      mon.addHistogram( new TH1F(icat+"mtsum",        ";M_{T}(l^{1},E_{T}^{miss})+M_{T}(l^{2},E_{T}^{miss}) [GeV];Events", 100,0,1000 ));
      mon.addHistogram( new TH1F(icat+"ht",           ";H_{T} [GeV];Events",                                               50,0,1000  ));
      mon.addHistogram( new TH1F(icat+"htb",          ";H_{T} (bjets) [GeV];Events",                                       50,0,1000  ));
      mon.addHistogram( new TH1F(icat+"htnol",        "; H_[T] (no leptons) [GeV];Events",                                 50,0,1000  ));
      mon.addHistogram( new TH1F(icat+"htbnol",       "; H_[T] (bjets, no leptons) [GeV];Events",                          50,0,1000  ));



      mon.addHistogram( new TH1F(icat+"emva", "; e-id MVA; Electrons", 50, 0.95,1.0) );
      //      mon.addHistogram( new TH1F(icat+"met",";Missing transverse energy [GeV];Events",50,0,500) );
      mon.addHistogram( new TH1F(icat+"metnotoppt",";Missing transverse energy [GeV];Events",50,0,500) );


      // Jet controls to be completed
      mon.addHistogram( new TH1F(icat+"nbjets",      ";b-jet multiplicity;Events", 6,0,6) );

      //extra leptons in the event
      // third lepton pt etc

      mon.addHistogram (new TH1F (icat + "csv", ";Combined Secondary Vertex;Jets", 50, 0., 1.));
      mon.addHistogram (new TH1F (icat + "csvb", ";Combined Secondary Vertex;Jets", 50, 0., 1.));
      mon.addHistogram (new TH1F (icat + "csvc", ";Combined Secondary Vertex;Jets", 50, 0., 1.));
      mon.addHistogram (new TH1F (icat + "csvothers", ";Combined Secondary Vertex;Jets", 50, 0., 1.));
      TH1 *hbtags = mon.addHistogram (new TH1F (icat + "nbtags", ";b-tag multiplicity;Events", 5, 0, 5));
      TH1 *hbtagsJP = mon.addHistogram (new TH1F (icat + "nbtagsJP", ";b-tag multiplicity;Events", 5, 0, 5));
      mon.addHistogram (new TH1F (icat + "leadjetpt", ";Transverse momentum [GeV];Events", 50, 0, 1000));
      mon.addHistogram (new TH1F (icat + "trailerjetpt", ";Transverse momentum [GeV];Events", 50, 0, 1000));
      mon.addHistogram (new TH1F (icat + "fwdjeteta", ";Pseudo-rapidity;Events", 25, 0, 5));
      mon.addHistogram (new TH1F (icat + "cenjeteta", ";Pseudo-rapidity;Events", 25, 0, 5));
      TH1 *hjets = mon.addHistogram (new TH1F (icat + "njets", ";Jet multiplicity;Events", 5, 0, 5));
      for (int ibin = 1; ibin <= hjets->GetXaxis ()->GetNbins (); ibin++)
        {
          TString label ("");
          if (ibin == h->GetXaxis ()->GetNbins ())
            label += "#geq";
          else
            label += "=";
          label += (ibin - 1);
          hjets->GetXaxis ()->SetBinLabel (ibin, label);
          hbtags->GetXaxis ()->SetBinLabel (ibin, label);
          hbtagsJP->GetXaxis ()->SetBinLabel (ibin, label);
        }
     
      mon.addHistogram (new TH1F (icat + "mindphijmet", ";min #Delta#phi(jet,E_{T}^{miss});Events", 40, 0, 4));
      mon.addHistogram (new TH1F (icat + "mindphijmetNM1", ";min #Delta#phi(jet,E_{T}^{miss});Events", 40, 0, 4));
      mon.addHistogram (new TH1D (icat + "balance", ";E_{T}^{miss}/q_{T};Events", 25, 0, 2.5));
      mon.addHistogram (new TH1D (icat + "balanceNM1", ";E_{T}^{miss}/q_{T};Events", 25, 0, 2.5));
      mon.addHistogram (new TH1F (icat + "axialmet", ";Axial missing transvere energy [GeV];Events", 50, -100, 400));
      mon.addHistogram (new TH1F (icat + "axialmetNM1", ";Axial missing transvere energy [GeV];Events", 50, -100, 400));
      mon.addHistogram (new TH1F (icat + "met", ";Missing transverse energy [GeV];Events", 50, 0., 1000.));
      mon.addHistogram (new TH1F (icat + "recoMet", ";Missing transverse energy [GeV];Events", 50, 0., 1000.));
      mon.addHistogram (new TH1F (icat + "mt", ";Transverse mass;Events", 50, 0., 500.));
      mon.addHistogram (new TH1F (icat + "mtresponse", ";Transverse mass response;Events", 100, 0, 2));
      mon.addHistogram (new TH1F (icat + "mtcheckpoint", ";Transverse mass [GeV];Events", 160, 150, 1750));
      mon.addHistogram (new TH1F (icat + "metcheckpoint", ";Missing transverse energy [GeV];Events", 100, 0, 500));



    }                           // End of loop on controlCats

  //
  // STATISTICAL ANALYSIS
  //
  TH1F *Hoptim_systs = (TH1F *) mon.addHistogram (new TH1F ("optim_systs", ";syst;", nvarsToInclude, 0, nvarsToInclude));
  for (size_t ivar = 0; ivar < nvarsToInclude; ivar++) Hoptim_systs->GetXaxis ()->SetBinLabel (ivar + 1, varNames[ivar]);



  //##############################################
  //######## GET READY FOR THE EVENT LOOP ########
  //##############################################

  fwlite::ChainEvent ev (urls);
  const size_t totalEntries = ev.size ();

  //MC normalization (to 1/pb)
  double xsecWeight = xsec / totalEntries;
  if(!isMC) xsecWeight = 1.0;
  if(debug){
    cout << "DEBUG: xsec: " << xsec << endl;
    cout << "DEBUG: xsecWeight: " << xsecWeight << endl;
    cout << "DEBUG: totalEntries: " << totalEntries << endl;
  }
  
  
  //jet energy scale and uncertainties 
  TString jecDir = runProcess.getParameter < std::string > ("jecDir");
  gSystem->ExpandPathName (jecDir);
  FactorizedJetCorrector *jesCor = utils::cmssw::getJetCorrector (jecDir, isMC);
  JetCorrectionUncertainty *totalJESUnc = new JetCorrectionUncertainty ((jecDir + "/MC_Uncertainty_AK5PFchs.txt").Data ());

  //muon energy scale and uncertainties
  MuScleFitCorrector *muCor = getMuonCorrector (jecDir, dtag);

  //lepton efficiencies
  LeptonEfficiencySF lepEff;

  //b-tagging: beff and leff must be derived from the MC sample using the discriminator vs flavor
  //the scale factors are taken as average numbers from the pT dependent curves see:
  //https://twiki.cern.ch/twiki/bin/viewauth/CMS/BtagPOG#2012_Data_and_MC_EPS13_prescript
  BTagSFUtil btsfutil;
  float beff (0.68), sfb (0.99), sfbunc (0.015);
  float leff (0.13), sfl (1.05), sflunc (0.12);


  TString
    electronIdMainTag("egmGsfElectronIDs:cutBasedElectronID-Spring15-25ns-V1-standalone-loose"),
    electronIdVetoTag("egmGsfElectronIDs:cutBasedElectronID-Spring15-25ns-V1-standalone-tight");

  //pileup weighting
  edm::LumiReWeighting * LumiWeights = NULL;
  utils::cmssw::PuShifter_t PuShifters;
  double PUNorm[] = { 1, 1, 1 };
  if (isMC)
    {
      std::vector < double >dataPileupDistributionDouble = runProcess.getParameter < std::vector < double >>("datapileup");
      std::vector < float >dataPileupDistribution;
      for (unsigned int i = 0; i < dataPileupDistributionDouble.size (); i++)
        {
          dataPileupDistribution.push_back (dataPileupDistributionDouble[i]);
        }
      std::vector < float >mcPileupDistribution;
      utils::getMCPileupDistributionFromMiniAOD (ev, dataPileupDistribution.size (), mcPileupDistribution);
      while (mcPileupDistribution.size () < dataPileupDistribution.size ()) mcPileupDistribution.push_back (0.0);
      while (mcPileupDistribution.size () > dataPileupDistribution.size ()) dataPileupDistribution.push_back (0.0);
      gROOT->cd ();             //THIS LINE IS NEEDED TO MAKE SURE THAT HISTOGRAM INTERNALLY PRODUCED IN LumiReWeighting ARE NOT DESTROYED WHEN CLOSING THE FILE
      LumiWeights = new edm::LumiReWeighting (mcPileupDistribution, dataPileupDistribution);
      PuShifters = utils::cmssw::getPUshifters (dataPileupDistribution, 0.05);
      utils::getPileupNormalization (mcPileupDistribution, PUNorm, LumiWeights, PuShifters);
    }

  gROOT->cd ();                 //THIS LINE IS NEEDED TO MAKE SURE THAT HISTOGRAM INTERNALLY PRODUCED IN LumiReWeighting ARE NOT DESTROYED WHEN CLOSING THE FILE
  
  //higgs::utils::EventCategory eventCategoryInst(higgs::utils::EventCategory::EXCLUSIVE2JETSVBF); //jet(0,>=1)+vbf binning
  
  
  //##############################################
  //########           EVENT LOOP         ########
  //##############################################
  //loop on all the events
  printf ("Progressing Bar     :0%%       20%%       40%%       60%%       80%%       100%%\n");
  printf ("Scanning the ntuple :");
  int treeStep (totalEntries / 50);
  //DuplicatesChecker duplicatesChecker;
  //int nDuplicates(0);
  for (size_t iev = 0; iev < totalEntries; iev++)
    {
      if (iev % treeStep == 0)
        {
          printf (".");
          fflush (stdout);
        }

      //##############################################   EVENT LOOP STARTS   ##############################################
      ev.to (iev);              //load the event content from the EDM file
      //if(!isMC && duplicatesChecker.isDuplicate( ev.run, ev.lumi, ev.event) ) { nDuplicates++; continue; }

      //apply trigger and require compatibilitiy of the event with the PD
      edm::TriggerResultsByName tr = ev.triggerResultsByName ("HLT");
      if (!tr.isValid ())
        return false;

      std::vector<TString> myTrigNames; myTrigNames.clear();

      myTrigNames.push_back("HLT_IsoMu17_eta2p1_v*"                                 );  // 1000 1 (L1 prescale, HLT prescale) for v1
      myTrigNames.push_back("HLT_IsoMu17_eta2p1_LooseIsoPFTau20_v*"                 );  // 1    1 
      myTrigNames.push_back("HLT_IsoMu17_eta2p1_MediumIsoPFTau40_Trk1_eta2p1_Reg_v*");  // 1    1 
      myTrigNames.push_back("HLT_IsoMu20_eta2p1_IterTrk02_v*"                       );  // 1000 0 
      myTrigNames.push_back("HLT_IsoMu24_eta2p1_IterTrk02_v*"                       );  // 1    1 
      myTrigNames.push_back("HLT_IsoMu24_eta2p1_LooseIsoPFTau20_v*"                 );  // 1    1 
      myTrigNames.push_back("HLT_IsoMu27_IterTrk02_v*"                              );  // 1    1 
      myTrigNames.push_back("HLT_IsoTkMu27_IterTrk02_v*"                            );  // 1    1  
      myTrigNames.push_back("HLT_Ele22_eta2p1_WP85_Gsf_v*"                          ); // 3000 1 (L1 prescale, HLT prescale) for v1
      myTrigNames.push_back("HLT_Ele22_eta2p1_WP85_Gsf_LooseIsoPFTau20_v*"          ); // 1    1 
      myTrigNames.push_back("HLT_Ele23_CaloIdL_TrackIdL_IsoVL_v*"                   ); // 4000 1 
      myTrigNames.push_back("HLT_Ele27_eta2p1_WP85_Gsf_v*"                          ); // 1000 0 
      myTrigNames.push_back("HLT_Ele32_eta2p1_WP85_Gsf_v*"                          ); // 1    1 
      myTrigNames.push_back("HLT_Ele32_eta2p1_WP85_Gsf_LooseIsoPFTau20_v*"          ); // 1    1 
      myTrigNames.push_back("HLT_Ele35_eta2p1_WP85_Gsf_v*"                          ); // 1    1 
      myTrigNames.push_back("HLT_Ele40_eta2p1_WP85_Gsf_v*"                          ); // 1    1     


      std::vector<bool> singleMuonTrigger;     singleMuonTrigger.clear();
      std::vector<bool> singleElectronTrigger; singleElectronTrigger.clear();

      singleMuonTrigger.push_back(utils::passTriggerPatterns (tr, "HLT_IsoMu17_eta2p1_v*"                                 ));  // 1000 1 (L1 prescale, HLT prescale) for v1
      singleMuonTrigger.push_back(utils::passTriggerPatterns (tr, "HLT_IsoMu17_eta2p1_LooseIsoPFTau20_v*"                 ));  // 1    1 
      singleMuonTrigger.push_back(utils::passTriggerPatterns (tr, "HLT_IsoMu17_eta2p1_MediumIsoPFTau40_Trk1_eta2p1_Reg_v*"));  // 1    1 
      singleMuonTrigger.push_back(utils::passTriggerPatterns (tr, "HLT_IsoMu20_eta2p1_IterTrk02_v*"                       ));  // 1000 0 
      singleMuonTrigger.push_back(utils::passTriggerPatterns (tr, "HLT_IsoMu24_eta2p1_IterTrk02_v*"                       ));  // 1    1 
      singleMuonTrigger.push_back(utils::passTriggerPatterns (tr, "HLT_IsoMu24_eta2p1_LooseIsoPFTau20_v*"                 ));  // 1    1 
      singleMuonTrigger.push_back(utils::passTriggerPatterns (tr, "HLT_IsoMu27_IterTrk02_v*"                              ));  // 1    1 
      singleMuonTrigger.push_back(utils::passTriggerPatterns (tr, "HLT_IsoTkMu27_IterTrk02_v*"                            ));  // 1    1  
      
      singleElectronTrigger.push_back(utils::passTriggerPatterns (tr, "HLT_Ele22_eta2p1_WP85_Gsf_v*"                )); // 3000 1 (L1 prescale, HLT prescale) for v1
      singleElectronTrigger.push_back(utils::passTriggerPatterns (tr, "HLT_Ele22_eta2p1_WP85_Gsf_LooseIsoPFTau20_v*")); // 1    1 
      singleElectronTrigger.push_back(utils::passTriggerPatterns (tr, "HLT_Ele23_CaloIdL_TrackIdL_IsoVL_v*"         )); // 4000 1 
      singleElectronTrigger.push_back(utils::passTriggerPatterns (tr, "HLT_Ele27_eta2p1_WP85_Gsf_v*"                )); // 1000 0 
      singleElectronTrigger.push_back(utils::passTriggerPatterns (tr, "HLT_Ele32_eta2p1_WP85_Gsf_v*"                )); // 1    1 
      singleElectronTrigger.push_back(utils::passTriggerPatterns (tr, "HLT_Ele32_eta2p1_WP85_Gsf_LooseIsoPFTau20_v*")); // 1    1 
      singleElectronTrigger.push_back(utils::passTriggerPatterns (tr, "HLT_Ele35_eta2p1_WP85_Gsf_v*"                )); // 1    1 
      singleElectronTrigger.push_back(utils::passTriggerPatterns (tr, "HLT_Ele40_eta2p1_WP85_Gsf_v*"                )); // 1    1     
      
      bool keepevent(false);
      for(size_t i=0; i<singleMuonTrigger.size(); ++i)
        if(singleMuonTrigger[i]) keepevent=true;
      for(size_t i=0; i<singleElectronTrigger.size(); ++i)
        if(singleElectronTrigger[i]) keepevent=true;
      if(!keepevent) continue;                                  //ONLY RUN ON THE EVENTS THAT PASS OUR TRIGGERS
      
      //##############################################   EVENT PASSED THE TRIGGER   #######################################
      
      //load all the objects we will need to access
      edm::TriggerResults triggerBits;
      fwlite::Handle<edm::TriggerResults> triggerBitsHandle;
      triggerBitsHandle.getByLabel(ev, "TriggerResults","","HLT"); // ?
      if(triggerBitsHandle.isValid()) triggerBits = *triggerBitsHandle;
      const edm::TriggerNames& trigNames = ev.triggerNames(triggerBits);

      pat::TriggerObjectStandAloneCollection triggerObjects;
      fwlite::Handle<pat::TriggerObjectStandAloneCollection> triggerObjectsHandle;
      triggerObjectsHandle.getByLabel(ev, "selectedPatTrigger" );
      if(triggerObjectsHandle.isValid()) triggerObjects = *triggerObjectsHandle;

      for(pat::TriggerObjectStandAlone obj : triggerObjects){
        obj.unpackPathNames(trigNames);
        for(size_t i=0; i<myTrigNames.size(); ++i)
          if(utils::passTriggerPatterns(tr, myTrigNames[i].Data())){
            //cout << "Trigger "<< myTrigNames[i] << " both: "<< obj.hasPathName(myTrigNames[i].Data(), true, true) << ", L3: " << obj.hasPathName(myTrigNames[i].Data(), false, true) << ", LF: " << obj.hasPathName(myTrigNames[i].Data(), true, false) << ", none: " << obj.hasPathName(myTrigNames[i].Data(), false, false) <<  " Object: pt " << obj.pt() << ", eta " << obj.eta() << ", phi " << obj.phi() << endl;
          }
        
      }


      reco::VertexCollection vtx;
      fwlite::Handle < reco::VertexCollection > vtxHandle;
      vtxHandle.getByLabel (ev, "offlineSlimmedPrimaryVertices");
      if (vtxHandle.isValid() ) vtx = *vtxHandle;

      double rho = 0;
      fwlite::Handle < double >rhoHandle;
      rhoHandle.getByLabel (ev, "fixedGridRhoFastjetAll");
      if (rhoHandle.isValid() ) rho = *rhoHandle;

      reco::GenParticleCollection gen;
      fwlite::Handle < reco::GenParticleCollection > genHandle;
      genHandle.getByLabel (ev, "prunedGenParticles");
      if (genHandle.isValid() ) gen = *genHandle;

      // Save time and don't load the rest of the objects when selecting by mctruthmode :)
      bool hasTop(false);
      int
        ngenLeptonsStatus3(0),
        ngenTausStatus3(0),
        ngenQuarksStatus3(0);
      //double tPt(0.), tbarPt(0.); // top pt reweighting - dummy value results in weight equal to 1 if not set in loop
      //float wgtTopPt(1.0), wgtTopPtUp(1.0), wgtTopPtDown(1.0);
      if(isMC)
        {
          //if(iev != 500) continue;
          for(size_t igen=0; igen<gen.size(); igen++){
            if(gen[igen].status()!=1 && abs(gen[igen].pdgId())!=6) continue;
            //            cout << "Particle " << igen << " has " << gen[igen].numberOfDaughters() << ", pdgId " << gen[igen].pdgId() << " and status " << gen[igen].status() << ", pt " << gen[igen].pt() << ", eta " << gen[igen].eta() << ", phi " << gen[igen].phi() << endl;
            int absid=abs(gen[igen].pdgId());
            if(absid==6 && gen[igen].status()==22){ // particles of the hardest subprocess 22 : intermediate (intended to have preserved mass)
              hasTop=true;
              //if(isTTbarMC){
              //  if(gen[igen].get("id") > 0) tPt=gen[igen].pt();
              //  else                        tbarPt=gen[igen].pt();
              //}
            }
            if(gen[igen].status()==22) continue; // Now we want only status 1 particles ;)

            if(absid==11 || absid==13) ngenLeptonsStatus3++;
            if(absid==15             ) ngenTausStatus3++;
            if(absid<=5              ) ngenQuarksStatus3++;
          }
          
          // mctruthmode reference table:
          //    ttbar dileptons --> 1
          //    ttbar ltau      --> 2
          //    ttbar ljets     --> 3
          //    ttbar hadrons   --> 4
          //cout << "mctruthmode " << mctruthmode << ", hasTop " << hasTop << ", ngenLeptonsStatus3: " << ngenLeptonsStatus3 << ", ngenTausStatus3: " << ngenTausStatus3 << ", ngetQuarksStatus3: " << ngenQuarksStatus3 << endl;
          if(mctruthmode==1 && (ngenLeptonsStatus3!=2                        || !hasTop )) continue;
          //else{
          //  if(mctruthmode==1) cout << "Run/LS/Ev: " << ev.eventAuxiliary().run() << "/" << ev.eventAuxiliary().luminosityBlock() << "/" << ev.eventAuxiliary().event() << "has mctruthmode 1 and will be analyzed" << endl;
          //  
          //}
          if(mctruthmode==2 && (ngenLeptonsStatus3!=1 || ngenTausStatus3!=1  || !hasTop )) continue;
          if(mctruthmode==3 && (ngenLeptonsStatus3!=1 || ngenQuarksStatus3<4 || !hasTop )) continue; // Check about the "<4"
          if(mctruthmode==4 && (ngenLeptonsStatus3!=0 || ngenTausStatus3!=0  || !hasTop )) continue;

        }

      //      if(tPt>0 && tbarPt>0 && topPtWgt)
      //        {
      //          topPtWgt->computeWeight(tPt,tbarPt);
      //          topPtWgt->getEventWeight(wgtTopPt, wgtTopPtUp, wgtTopPtDown);
      //          wgtTopPtUp /= wgtTopPt;
      //          wgtTopPtDown /= wgtTopPt;
      //        }
      




      pat::MuonCollection muons;
      fwlite::Handle < pat::MuonCollection > muonsHandle;
      muonsHandle.getByLabel (ev, "slimmedMuons");
      if (muonsHandle.isValid() ) muons = *muonsHandle;

      pat::ElectronCollection electrons;
      fwlite::Handle < pat::ElectronCollection > electronsHandle;
      electronsHandle.getByLabel (ev, "slimmedElectrons");
      if (electronsHandle.isValid() ) electrons = *electronsHandle;

      pat::JetCollection jets;
      fwlite::Handle < pat::JetCollection > jetsHandle;
      jetsHandle.getByLabel (ev, "slimmedJets");
      if (jetsHandle.isValid() ) jets = *jetsHandle;

      pat::PhotonCollection photons;
      fwlite::Handle < pat::PhotonCollection > photonsHandle;
      photonsHandle.getByLabel (ev, "slimmedPhotons");
      if (photonsHandle.isValid() ) photons = *photonsHandle;

      pat::METCollection mets;
      fwlite::Handle < pat::METCollection > metsHandle;
      metsHandle.getByLabel (ev, "slimmedMETs");
      if (metsHandle.isValid() ) mets = *metsHandle;
      LorentzVector met = mets[0].p4 ();

      if(debug && false){
        // MET try:
        double mypt = mets[0].shiftedPt(pat::MET::METUncertainty::JetEnUp);
        cout << "MET = " << mets[0].pt() << ", JetEnUp: " << mypt << endl;
        LorentzVector myshiftedMet = mets[0].shiftedP4(pat::MET::METUncertainty::JetEnUp);
        cout << "MET = " << mets[0].pt() << ", JetEnUp: " << myshiftedMet.pt() << endl;
      }

      pat::TauCollection taus;
      fwlite::Handle < pat::TauCollection > tausHandle;
      tausHandle.getByLabel (ev, "slimmedTaus");
      if (tausHandle.isValid() ) taus = *tausHandle;

      if (isV0JetsMC)
        {
          fwlite::Handle < LHEEventProduct > lheEPHandle;
          lheEPHandle.getByLabel (ev, "externalLHEProducer");
          mon.fillHisto ("nup", "", lheEPHandle->hepeup ().NUP, 1);
          if (lheEPHandle->hepeup ().NUP > 5)  continue;
          mon.fillHisto ("nupfilt", "", lheEPHandle->hepeup ().NUP, 1);
        }
      
      //
      // DERIVE WEIGHTS TO APPLY TO SAMPLE
      //

      //pileup weight
      float weight = 1.0;
      double TotalWeight_plus = 1.0;
      double TotalWeight_minus = 1.0;
      float puWeight (1.0);

      if(isMC)
        {
          int ngenITpu = 0;
          
          fwlite::Handle < std::vector < PileupSummaryInfo > >puInfoH;
          puInfoH.getByLabel (ev, "addPileupInfo");
          for (std::vector < PileupSummaryInfo >::const_iterator it = puInfoH->begin (); it != puInfoH->end (); it++)
            {
              if (it->getBunchCrossing () == 0) ngenITpu += it->getPU_NumInteractions ();
            }
          
          puWeight = LumiWeights->weight (ngenITpu) * PUNorm[0];
          weight = xsecWeight; //* puWeight; // Temporarily disabled PU reweighing, it's wrong to scale to the 2012 data distribution.
          TotalWeight_plus =  PuShifters[utils::cmssw::PUUP]  ->Eval (ngenITpu) * (PUNorm[2]/PUNorm[0]);
          TotalWeight_minus = PuShifters[utils::cmssw::PUDOWN]->Eval (ngenITpu) * (PUNorm[1]/PUNorm[0]);
        }
      
      
      //
      //
      // BELOW FOLLOWS THE ANALYSIS OF THE MAIN SELECTION WITH N-1 PLOTS
      //
      //


      
      //
      // LEPTON ANALYSIS
      //
      
      //start by merging electrons and muons
      std::vector < patUtils::GenericLepton > leptons;
      for(size_t l = 0; l < electrons.size (); l++) leptons.push_back (patUtils::GenericLepton (electrons[l] ));
      for(size_t l = 0; l < muons.size (); l++)     leptons.push_back (patUtils::GenericLepton (muons[l]     ));
      std::sort (leptons.begin (), leptons.end (), utils::sort_CandidatesByPt);

      // Acquire electron ID
      patUtils::ElectronId elIdTight(ev, patUtils::llvvElecId::Tight);
      patUtils::ElectronId elIdLoose(ev, patUtils::llvvElecId::Loose);

      LorentzVector muDiff (0, 0, 0, 0);
      std::vector < patUtils::GenericLepton > selLeptons, selSingleLepLeptons; // Different main lepton definitions
      double nVetoE(0), nVetoMu(0);
      for (size_t ilep = 0; ilep < leptons.size (); ilep++)
        {
          bool 
            passKin(true),             passId(true),              passIso(true),
            passSingleLepKin(true),    passSingleLepId(true),     passSingleLepIso(true),
            passVetoSingleLepKin(true),passVetoSingleLepId(true), passVetoSingleLepIso(true);
          
          int lid = leptons[ilep].pdgId ();
          
          //apply muon corrections
          if (abs (lid) == 13)
            {
              if (muCor)
                {
                  TLorentzVector p4 (leptons[ilep].px(), leptons[ilep].py(), leptons[ilep].pz(), leptons[ilep].energy());
                  muCor->applyPtCorrection (p4, lid < 0 ? -1 : 1);
                  if (isMC) muCor->applyPtSmearing (p4, lid < 0 ? -1 : 1, false);
                  muDiff -= leptons[ilep].p4();
                  leptons[ilep].setP4(LorentzVector(p4.Px(), p4.Py(), p4.Pz(), p4.E()));
                  muDiff += leptons[ilep].p4();
                }
            }
          
          //no need for charge info any longer
          lid = abs (lid);
          TString lepStr(lid == 13 ? "mu" : "e");
          
          // don't want to mess with photon ID // //veto nearby photon (loose electrons are many times photons...)
          // don't want to mess with photon ID // double minDRlg(9999.);
          // don't want to mess with photon ID // for(size_t ipho=0; ipho<selPhotons.size(); ipho++)
          // don't want to mess with photon ID //   minDRlg=TMath::Min(minDRlg,deltaR(leptons[ilep].p4(),selPhotons[ipho].p4()));
          // don't want to mess with photon ID // if(minDRlg<0.1) continue;
          
          //kinematics
          float leta = fabs (lid == 11 ? leptons[ilep].el.superCluster ()->eta() : leptons[ilep].eta());
          

          // Dileptons main kin
          if (leptons[ilep].pt () < 20.)                      passKin = false;
          if (leta > (lid == 11 ? 2.5 : 2.4))                passKin = false;
          if (lid == 11 && (leta > 1.4442 && leta < 1.5660)) passKin = false; // Crack veto
          
          // Single lepton main + veto kin
          if (leptons[ilep].pt () < (lid==11 ? 35. : 30.))   passSingleLepKin = false;
          if (leta > (lid == 11 ? 2.5 : 2.1))                { passSingleLepKin = false; passVetoSingleLepKin = false; }
          if (lid == 11 && (leta > 1.4442 && leta < 1.5660)) { passSingleLepKin = false; passVetoSingleLepKin = false; } // Crack veto
          
          // Single lepton veto kin
          if (leptons[ilep].pt () < (lid==11 ? 20. : 10.))   passVetoSingleLepKin = false;
          

          
          //Cut based identification 
          passId          = lid == 11 ? (leptons[ilep].el.electronID(electronIdMainTag)==7) : patUtils::passId (leptons[ilep].mu, vtx[0], patUtils::llvvMuonId::Loose);
          passSingleLepId = lid == 11 ? (leptons[ilep].el.electronID(electronIdMainTag)==7) : patUtils::passId (leptons[ilep].mu, vtx[0], patUtils::llvvMuonId::Tight);
          passVetoSingleLepId = passId;

          //isolation
          passIso          = lid == 11 ? true : patUtils::passIso (leptons[ilep].mu, patUtils::llvvMuonIso::Loose); // Try tight iso for dilepton
          passSingleLepIso = lid == 11 ? true : patUtils::passIso (leptons[ilep].mu, patUtils::llvvMuonIso::Tight); // Try tight iso for dilepton
          passVetoSingleLepIso = passIso;
          
          if (passKin          && passId          && passIso)          selLeptons.push_back(leptons[ilep]);
          if (passSingleLepKin && passSingleLepId && passSingleLepIso) selSingleLepLeptons.push_back(leptons[ilep]);
          else if(passVetoSingleLepKin && passVetoSingleLepId && passVetoSingleLepIso) lid==11 ? nVetoE++ : nVetoMu++;
          
        }
      std::sort(selLeptons.begin(),   selLeptons.end(),   utils::sort_CandidatesByPt);
      std::sort(selSingleLepLeptons.begin(), selSingleLepLeptons.end(), utils::sort_CandidatesByPt);
      LorentzVector recoMET = met - muDiff;
      

      //select the taus
      pat::TauCollection selTaus;
      int ntaus (0);
      for (size_t itau = 0; itau < taus.size(); ++itau)
        {
          pat::Tau & tau = taus[itau];
          if (tau.pt() < 20. || fabs (tau.eta()) > 2.3) continue;
          
          bool overlapWithLepton(false);
          for(int l1=0; l1<(int)selSingleLepLeptons.size();++l1){
            if(deltaR(tau, selSingleLepLeptons[l1])<0.1){overlapWithLepton=true; break;}
          }
          if(overlapWithLepton) continue;
          
          //      if(!tau.isPFTau()) continue; // Only PFTaus // It should be false for slimmedTaus
          //      if(tau.emFraction() >=2.) continue;
          
          if (!tau.tauID ("byMediumCombinedIsolationDeltaBetaCorr3Hits")) continue;
          //if (!tau.tauID ("againstMuonTight3"))                           continue;
          //if (!tau.tauID ("againstElectronMediumMVA5"))                   continue;
         
          selTaus.push_back(tau);
          ntaus++;
        }
      std::sort (selTaus.begin(), selTaus.end(), utils::sort_CandidatesByPt);
      
      
      
      //
      //JET/MET ANALYSIS
      //
      //add scale/resolution uncertainties and propagate to the MET      
      //utils::cmssw::updateJEC(jets,jesCor,totalJESUnc,rho,vtx.size(),isMC);  //FIXME if still needed
      //std::vector<LorentzVector> met=utils::cmssw::getMETvariations(recoMet,jets,selLeptons,isMC); //FIXME if still needed
      
      //select the jets
      pat::JetCollection
        selJets, selBJets,
        selSingleLepJets, selSingleLepBJets;
      int njets (0), nbtags (0), nbtagsJP (0);
      float mindphijmet (9999.);
      for (size_t ijet = 0; ijet < jets.size(); ijet++)
        {
          if (jets[ijet].pt() < 15 || fabs (jets[ijet].eta()) > 4.7) continue;
          
          //mc truth for this jet
          const reco::GenJet * genJet = jets[ijet].genJet();
          TString jetType (genJet && genJet->pt() > 0 ? "truejetsid" : "pujetsid");
          
          //cross-clean with selected leptons and photons
          double minDRlj (9999.), minDRlg (9999.), minDRljSingleLep(9999.);

          for (size_t ilep = 0; ilep < selLeptons.size(); ilep++)
            minDRlj = TMath::Min(minDRlj, deltaR (jets[ijet], selLeptons[ilep]));
          // don't want to mess with photon ID // for(size_t ipho=0; ipho<selPhotons.size(); ipho++)
          // don't want to mess with photon ID //   minDRlg = TMath::Min( minDRlg, deltaR(jets[ijet],selPhotons[ipho]) );
          //          if (minDRlj < 0.4 /*|| minDRlg<0.4 */ ) continue;

          for (size_t ilep = 0; ilep < selSingleLepLeptons.size(); ilep++)
            minDRljSingleLep = TMath::Min(minDRljSingleLep, deltaR (jets[ijet], selSingleLepLeptons[ilep]));

          //jet id
          bool passPFloose = true;      //FIXME --> Need to be updated according to te latest recipe;
          float PUDiscriminant = jets[ijet].userFloat ("pileupJetId:fullDiscriminant");
          bool passLooseSimplePuId = true;      //FIXME --> Need to be updated according to the latest recipe
          if (jets[ijet].pt() > 30)
            {
              mon.fillHisto (jetType, "", fabs (jets[ijet].eta()), 0);
              if (passPFloose)                        mon.fillHisto (jetType, "", fabs (jets[ijet].eta()), 1);
              if (passLooseSimplePuId)                mon.fillHisto (jetType, "", fabs (jets[ijet].eta()), 2);
              if (passPFloose && passLooseSimplePuId) mon.fillHisto (jetType, "", fabs (jets[ijet].eta()), 3);
            }
          if (!passPFloose || !passLooseSimplePuId || jets[ijet].pt() <30 || fabs(jets[ijet].eta()) > 2.5) continue;


          if (minDRlj >= 0.4){
            selJets.push_back(jets[ijet]);
            njets++;
          }
          
          double minDRtj(9999.);
          for(size_t itau=0; itau<selTaus.size(); ++itau)
            {
              minDRtj = TMath::Min(minDRtj, deltaR(jets[ijet], selTaus[itau]));
            }
          if(minDRtj >0.4 && minDRljSingleLep >= 0.4) selSingleLepJets.push_back(jets[ijet]);
          

          
          float dphijmet = fabs (deltaPhi (met.phi(), jets[ijet].phi()));
          if (dphijmet < mindphijmet) mindphijmet = dphijmet;
          bool hasCSVtag (jets[ijet].bDiscriminator("combinedInclusiveSecondaryVertexV2BJetTags") > 0.814);
          // Apparently this V2 has the following preliminary operating points:
          // These preliminary operating points were derived from ttbar events:
          //   - Loose : 0.423 (corresponding to 10.1716% DUSG mistag efficiency)
          //   - Medium : 0.814 (corresponding to 1.0623% DUSG mistag efficiency)
          //   - Tight : 0.941 (corresponding to 0.1144% DUSG mistag efficiency)

          //update according to the SF measured by BTV
          if (isMC)
            {
              int flavId = jets[ijet].partonFlavour();
              if      (abs (flavId) == 5) btsfutil.modifyBTagsWithSF(hasCSVtag, sfb,   beff);
              else if (abs (flavId) == 4) btsfutil.modifyBTagsWithSF(hasCSVtag, sfb/5, beff);
              else                        btsfutil.modifyBTagsWithSF(hasCSVtag, sfl,   leff);
            }
          if(!hasCSVtag) continue;

          if(minDRlj > 0.4){
            nbtags++;
            selBJets.push_back(jets[ijet]);
          }
          if(minDRtj >0.4 && minDRljSingleLep>0.4) selSingleLepBJets.push_back(jets[ijet]);
          
        }
      std::sort (selJets.begin(),  selJets.end(),  utils::sort_CandidatesByPt);
      std::sort (selBJets.begin(), selBJets.end(), utils::sort_CandidatesByPt);
      std::sort (selSingleLepJets.begin(),  selSingleLepJets.end(),  utils::sort_CandidatesByPt);
      std::sort (selSingleLepBJets.begin(), selSingleLepBJets.end(), utils::sort_CandidatesByPt);
      
      //
      // ASSIGN CHANNEL
      //
      std::vector < TString > chTags; chTags.clear();
      int 
        dilId (1),
        slepId(0);
      LorentzVector dileptonSystem (0, 0, 0, 0);
      if (selLeptons.size() >= 2)
        {
          for (size_t ilep = 0; ilep < 2; ilep++)
            {
              dilId *= selLeptons[ilep].pdgId();
              int id (abs (selLeptons[ilep].pdgId()));
              dileptonSystem += selLeptons[ilep].p4();
            }
          
        }
      
      if(selSingleLepLeptons.size()>0)
        slepId=selSingleLepLeptons[0].pdgId();
      
      // Event classification. Single lepton triggers are first in order to ensure that the lower threshold dilepton triggers do not steal events from the single lepton category. emu trigger is last in order to ensure that it does not break the balance between ee and mumu
      
///       if(      abs(slepId) == 13 && muTrigger && nVetoE==0 && nVetoMu==0 ) chTags.push_back("singlemu");
///       else if( abs(slepId) == 11 && eTrigger  && nVetoE==0 && nVetoMu==0 ) chTags.push_back("singlee");
///       else if( abs(dilId)==121 && eeTrigger  )                             chTags.push_back("ee");
///       else if( abs(dilId)==169 && mumuTrigger)                             chTags.push_back("mumu");
///       else if( abs(dilId)==143 && emuTrigger )                             chTags.push_back("emu");
///       else                                                                 chTags.push_back("unclassified");
      
      // keep in mind the eventCategory thingy for more refined categorization // TString evCat=eventCategoryInst.GetCategory(selJets,dileptonSystem);
      std::vector < TString > tags (1, "all");
      for (size_t ich = 0; ich < chTags.size(); ich++)
        {
          tags.push_back (chTags[ich]);
          //tags.push_back( chTags[ich]+evCat );
        }

      if (chTags.size() == 0 || chTags.size() >1 ){ cout << "ALARM! chTags.size() == " << chTags.size() << ", and that should NEVER happen!!!" << endl; continue; }// That should never happen ("unclassified" is always present)
      

      // Dilepton full analysis
      if( tags[1] == "ee"|| tags[1] == "emu" || tags[1] == "mumu"){
        
        if(selLeptons.size()!=2) continue; // Save time
        // Apply lepton efficiencies
        //for(size_t ilep=0; ilep<2; ++ilep){
        //  int id (abs (selLeptons[ilep].pdgId()));
        //  weight *= isMC ? lepEff.getLeptonEfficiency(selLeptons[ilep].pt(), selLeptons[ilep].eta(), id, id == 11 ? "loose" : "loose").first : 1.0;
        //}
        
        // Event selection booleans
        bool passMllVeto(tags[1] == "emu" ? dileptonSystem.mass()>12. : (fabs(dileptonSystem.mass()-91.)>15 && dileptonSystem.mass()>12. ) );
        bool passJetSelection(selJets.size()>1);
        bool passMetSelection(recoMET.pt()>40.);
        bool passOS(selLeptons[0].pdgId() * selLeptons[1].pdgId() < 0 );
        bool passBtagsSelection(selBJets.size()>1);
       

        // Setting up control categories and fill up event flow histo
        std::vector < TString > ctrlCats; ctrlCats.clear ();
                                                                                                 { ctrlCats.push_back("step1"); mon.fillHisto("eventflow", tags, 0, weight); }
        if(passMllVeto   )                                                                       { ctrlCats.push_back("step2"); mon.fillHisto("eventflow", tags, 1, weight); }
        if(passMllVeto && passJetSelection )                                                     { ctrlCats.push_back("step3"); mon.fillHisto("eventflow", tags, 2, weight); }
        if(passMllVeto && passJetSelection && passMetSelection )                                 { ctrlCats.push_back("step4"); mon.fillHisto("eventflow", tags, 3, weight); }
        if(passMllVeto && passJetSelection && passMetSelection && passOS )                       { ctrlCats.push_back("step5"); mon.fillHisto("eventflow", tags, 4, weight); }
        if(passMllVeto && passJetSelection && passMetSelection && passOS && passBtagsSelection ) { ctrlCats.push_back("step6"); mon.fillHisto("eventflow", tags, 5, weight); }
        

        // Fill the control plots
        for(size_t k=0; k<ctrlCats.size(); ++k){
          
          TString icat(ctrlCats[k]);

          mon.fillHisto(icat+"nvtxraw",    tags, vtx.size(),                 weight/puWeight);
          mon.fillHisto(icat+"nvtx",       tags, vtx.size(),                 weight);
          mon.fillHisto(icat+"rho",        tags, rho,                        weight);


          // Lepton and dilepton control
          mon.fillHisto(icat+"leadpt",      tags, selLeptons[0].pt(),         weight);
          mon.fillHisto(icat+"trailerpt",   tags, selLeptons[1].pt(),         weight);
          mon.fillHisto(icat+"leadeta",     tags, fabs (selLeptons[0].eta()), weight);
          mon.fillHisto(icat+"trailereta",  tags, fabs (selLeptons[1].eta()), weight);

          float thetall(utils::cmssw::getArcCos<patUtils::GenericLepton>(selLeptons[0],selLeptons[1]));
          float sumpt(selLeptons[0].pt()+selLeptons[1].pt());
          float mtsum(0);///utils::cmssw::getMT<patUtils::GenericLepton,LorentzVector>(selLeptons[0],met)+utils::cmssw::getMT<patUtils::GenericLepton,LorentzVector>(selLeptons[1],met));

          mon.fillHisto(icat+"yll",          tags, fabs(dileptonSystem.Rapidity()), weight);
          mon.fillHisto(icat+"mll",          tags, dileptonSystem.mass(),           weight);
          mon.fillHisto(icat+"ptll",         tags, dileptonSystem.pt(),             weight);
          mon.fillHisto(icat+"dilarccosine", tags, thetall,                         weight);
          mon.fillHisto(icat+"sumpt",        tags, sumpt,                           weight);
          mon.fillHisto(icat+"mtsum",        tags, mtsum,                           weight);
          mon.fillHisto(icat+"qt",           tags, dileptonSystem.pt(),             weight, true);
          // mon.fillHisto("qtraw",    tags, dileptonSystem.pt(),weight/triggerPrescale,true);                                                                                      

          float
            ht    (sumpt),
            htb   (sumpt),
            htnol (0),
            htbnol(0);

          for(size_t ilep=0; ilep<2; ++ilep){
            float lepid(fabs(selLeptons[ilep].pdgId()));
            float leppt(selLeptons[ilep].pt());
            mon.fillHisto(icat+"inclusivept",       tags, leppt, weight);
            if(lepid==11) mon.fillHisto(icat+"pte", tags, leppt, weight);
            if(lepid==13) mon.fillHisto(icat+"ptmu",tags, leppt, weight);
          }


          // Tau control ??? 
          mon.fillHisto (icat+"ntaus",      tags, ntaus,                      weight);
          if(ntaus > 0) mon.fillHisto ("leadtaupt", tags, selTaus[0].pt(), weight);

          for(size_t ijet=0; ijet<selJets.size(); ++ijet)
            {
              ht+=selJets[ijet].pt();
              htnol+=selJets[ijet].pt();
              if(ijet<selBJets.size()) // Hack in order to have only one loop. Exploits the fact that selBjets are a subset of selJets.
                {
                  htb+=selBJets[ijet].pt();
                  htbnol+=selBJets[ijet].pt();
                }
              
              float csv (selJets[ijet].bDiscriminator("combinedSecondaryVertexBJetTags"));
              mon.fillHisto ("csv", tags, csv, weight);
              if (!isMC) continue;
              int flavId = selJets[ijet].partonFlavour();
              TString jetFlav ("others");
              if (abs (flavId) == 5) jetFlav = "b";
              else if (abs (flavId) == 4) jetFlav = "c";
              mon.fillHisto ("csv" + jetFlav, tags, csv, weight);
            }
          
          ht+=met.pt();
          htb+=met.pt();
          htnol+=met.pt();
          htbnol+=met.pt();

          mon.fillHisto(icat+"ht",     tags, ht,              weight);
          mon.fillHisto(icat+"htb",    tags, htb,             weight);
          mon.fillHisto(icat+"htnol",  tags, htnol,           weight);
          mon.fillHisto(icat+"htbnol", tags, htbnol,          weight);
          
          mon.fillHisto(icat+"nbjets", tags, selBJets.size(), weight);
          
          
        }
        

      } // End dilepton full analysis
      
      
      // Single lepton full analysis
      if(tags[1] == "singlemu" || tags[1] == "singlee"){

        if(selSingleLepLeptons.size()!=1) continue;
        //int id (abs (selSingleLepLeptons[0].pdgId()));
        //weight *= isMC ? lepEff.getLeptonEfficiency(selSingleLepLeptons[0].pt(), selSingleLepLeptons[0].eta(), id, id == 11 ? "loose" : "tight").first : 1.0;        
        
        // Event selection booleans
        bool passJetSelection(selSingleLepJets.size()>1);
        bool passMetSelection(recoMET.pt()>40.);
        bool passBtagsSelection(selSingleLepBJets.size()>0);
        bool passTauSelection(selTaus.size()==1);
        bool passOS(selTaus.size()>0 ? selSingleLepLeptons[0].pdgId() * selTaus[0].pdgId() < 0 : 0);

        // Setting up control categories and fill up event flow histo
        std::vector < TString > ctrlCats;
        ctrlCats.clear ();
                                                                                                      { ctrlCats.push_back ("step1"); mon.fillHisto("eventflowslep", tags, 0, weight); }
        if(passJetSelection   )                                                                       { ctrlCats.push_back ("step2"); mon.fillHisto("eventflowslep", tags, 1, weight); }
        if(passJetSelection && passMetSelection )                                                     { ctrlCats.push_back ("step3"); mon.fillHisto("eventflowslep", tags, 2, weight); }
        if(passJetSelection && passMetSelection && passBtagsSelection )                               { ctrlCats.push_back ("step4"); mon.fillHisto("eventflowslep", tags, 3, weight); }
        if(passJetSelection && passMetSelection && passBtagsSelection && passTauSelection )           { ctrlCats.push_back ("step5"); mon.fillHisto("eventflowslep", tags, 4, weight); }
        if(passJetSelection && passMetSelection && passBtagsSelection && passTauSelection && passOS ) { ctrlCats.push_back ("step6"); mon.fillHisto("eventflowslep", tags, 5, weight); }
        

        // Fill the control plots
        for(size_t k=0; k<ctrlCats.size(); ++k){
          
          TString icat(ctrlCats[k]);

          mon.fillHisto (icat+"nvtxraw",    tags, vtx.size(),                          weight/puWeight);
          mon.fillHisto (icat+"nvtx",       tags, vtx.size(),                          weight);
          mon.fillHisto (icat+"rho",        tags, rho,                                 weight);
          mon.fillHisto (icat+"leadpt",     tags, selSingleLepLeptons[0].pt(),         weight);
          mon.fillHisto (icat+"trailerpt",  tags, selSingleLepLeptons[1].pt(),         weight);
          mon.fillHisto (icat+"leadeta",    tags, fabs (selSingleLepLeptons[0].eta()), weight);
          mon.fillHisto (icat+"trailereta", tags, fabs (selSingleLepLeptons[1].eta()), weight);
          mon.fillHisto (icat+"ntaus",      tags, ntaus,                               weight);
          if(ntaus > 0) mon.fillHisto ("leadtaupt", tags, selTaus[0].pt(),             weight);


// dilepton only           mon.fillHisto (icat+"zmass", tags, dileptonSystem.mass(),           weight);
// dilepton only           mon.fillHisto (icat+"zy",    tags, fabs(dileptonSystem.Rapidity()), weight);
// dilepton only           mon.fillHisto (icat+"zpt",   tags, dileptonSystem.pt(),             weight);
// dilepton only           //these two are used to reweight photon -> Z, the 3rd is a control
// dilepton only           mon.fillHisto (icat+"qt",    tags, dileptonSystem.pt(),             weight, true);
// dilepton only           ///     mon.fillHisto("qtraw",    tags, dileptonSystem.pt(),weight/triggerPrescale,true); 

          for (size_t ijet = 0; ijet < selSingleLepJets.size(); ijet++)
            {
              if (selSingleLepJets[ijet].pt() < 30 || fabs (selSingleLepJets[ijet].eta()) > 2.5) continue;
              
              float csv (selSingleLepJets[ijet].bDiscriminator("combinedSecondaryVertexBJetTags"));
              mon.fillHisto ("csv", tags, csv, weight);
              if (!isMC) continue;
              int flavId = selSingleLepJets[ijet].partonFlavour();
              TString jetFlav ("others");
              if (abs (flavId) == 5) jetFlav = "b";
              else if (abs (flavId) == 4) jetFlav = "c";
              mon.fillHisto ("csv" + jetFlav, tags, csv, weight);
            }
          
        }
        
      } // End single lepton full analysis
      
      continue; // Quick break (statistical analysis will come later)
      
      //
      // HISTOS FOR STATISTICAL ANALYSIS (include systematic variations)
      //
      //Fill histogram for posterior optimization, or for control regions
      for (size_t ivar = 0; ivar < nvarsToInclude; ivar++)
        {
          float iweight = weight;       //nominal

          //energy scale/resolution
          bool varyJesUp (varNames[ivar] == "_jesup");
          bool varyJesDown (varNames[ivar] == "_jesdown");
          bool varyJerUp (varNames[ivar] == "_jerup");
          bool varyJerDown (varNames[ivar] == "_jerdown");
          bool varyUmetUp (varNames[ivar] == "_umetup");
          bool varyUmetDown (varNames[ivar] == "_umetdown");
          bool varyLesUp (varNames[ivar] == "_lesup");
          bool varyLesDown (varNames[ivar] == "_lesdown");

          //pileup variations
          if (varNames[ivar] == "_puup")   iweight *= TotalWeight_plus;
          if (varNames[ivar] == "_pudown") iweight *= TotalWeight_minus;

          //btag
          bool varyBtagUp (varNames[ivar] == "_btagup");
          bool varyBtagDown (varNames[ivar] == "_btagdown");

          //Here were the Q^2 variations on VV pT spectum


          //recompute MET/MT if JES/JER was varied
          LorentzVector zvv = mets[0].p4();
          //FIXME
          //      if(varyJesUp)    zvv = mets[0].shiftedP4(pat::MET::METUncertainty::JetEnUp);
          //      if(varyJesDown)  zvv = mets[0].shiftedP4(pat::MET::METUncertainty::JetEnDown);
          //      if(varyJerUp)    zvv = mets[0].shiftedP4(pat::MET::METUncertainty::JetResUp);
          //      if(varyJerDown)  zvv = mets[0].shiftedP4(pat::MET::METUncertainty::JetResDown);
          //      if(varyUmetUp)   zvv = mets[0].shiftedP4(pat::MET::METUncertainty::UnclusteredEnUp);
          //      if(varyUmetDown) zvv = mets[0].shiftedP4(pat::MET::METUncertainty::UnclusteredEnDown);
          //      if(varyLesUp)    zvv = met[utils::cmssw::LESUP]; //FIXME  must vary all leptons separately: MuonEnUp/MuonEnDown/ElectronEnUp/ElectronEnDown/TauEnUp/TauEnDown
          //      if(varyLesDown)  zvv = met[utils::cmssw::LESDOWN];

          pat::JetCollection tightVarJets;
          bool passLocalBveto (true);///passBtags);
          for (size_t ijet = 0; ijet < jets.size(); ijet++)
            {

              float eta = jets[ijet].eta();
              if (fabs (eta) > 4.7) continue;
              float pt = jets[ijet].pt();
              //FIXME
              //        if(varyJesUp)    pt=jets[ijet].getVal("jesup");
              //        if(varyJesDown)  pt=jets[ijet].getVal("jesdown");
              //        if(varyJerUp)    pt=jets[ijet].getVal("jerup");
              //        if(varyJerDown)  pt=jets[ijet].getVal("jerdown");
              if (pt < 30) continue;

              //cross-clean with selected leptons and photons
              double minDRlj (9999.), minDRlg (9999.);
              for (size_t ilep = 0; ilep < selLeptons.size(); ilep++)
                minDRlj = TMath::Min (minDRlj, deltaR (jets[ijet].p4(), selLeptons[ilep].p4()));
              // don't want to mess with photon ID // for(size_t ipho=0; ipho<selPhotons.size(); ipho++)
              // don't want to mess with photon ID //   minDRlg = TMath::Min( minDRlg, deltaR(jets[ijet].p4(),selPhotons[ipho].p4()) );
              if (minDRlj < 0.4 /*|| minDRlg<0.4 */ ) continue;

              //jet id
              bool passPFloose = true;  //FIXME
              int simplePuId = true;    //FIXME
              bool passLooseSimplePuId = true;  //FIXME
              if (!passPFloose || !passLooseSimplePuId) continue;

              //jet is selected
              tightVarJets.push_back (jets[ijet]);

              //check b-tag
              if (pt < 30 || fabs (eta) > 2.5) continue;
              if (!isMC) continue;
              if (!varyBtagUp && !varyBtagDown) continue;
              int flavId = jets[ijet].partonFlavour();
              bool hasCSVtag (jets[ijet].bDiscriminator("combinedSecondaryVertexBJetTags") > 0.405);
              if (varyBtagUp)
                {
                  if (abs (flavId) == 5)      btsfutil.modifyBTagsWithSF(hasCSVtag, sfb + sfbunc,     beff);
                  else if (abs (flavId) == 4) btsfutil.modifyBTagsWithSF(hasCSVtag, sfb/5 + 2*sfbunc, beff);
                  else                        btsfutil.modifyBTagsWithSF(hasCSVtag, sfl + sflunc,     leff);
                }
              else if (varyBtagDown)
                {
                  if (abs (flavId) == 5)      btsfutil.modifyBTagsWithSF(hasCSVtag, sfb - sfbunc,     beff);
                  else if (abs (flavId) == 4) btsfutil.modifyBTagsWithSF(hasCSVtag, sfb/5 - 2*sfbunc, beff);
                  else                        btsfutil.modifyBTagsWithSF(hasCSVtag, sfl - sflunc,     leff);
                }
              passLocalBveto |= hasCSVtag;
            }

          bool isZsideBand ((dileptonSystem.mass() > 40 && dileptonSystem.mass() < 70) || (dileptonSystem.mass() > 110 && dileptonSystem.mass() < 200));
          bool isZsideBandPlus ((dileptonSystem.mass() > 110 && dileptonSystem.mass() < 200));
          bool passPreselection (true);//passMass && passQt && passThirdLeptonVeto && passMinDphijmet && passLocalBveto);
          bool passPreselectionMbvetoMzmass (true);///passQt && passThirdLeptonVeto && passMinDphijmet);

          //re-assign the event category to take migrations into account
          //      TString evCat  = eventCategoryInst.GetCategory(tightVarJets,dileptonSystem);
          for (size_t ich = 0; ich < chTags.size(); ich++)
            {
              
              TString tags_full = chTags[ich];  //+evCat;
              float chWeight (iweight);
              
              //update weight and mass for photons
              LorentzVector idileptonSystem (dileptonSystem);
              
              //updet the transverse mass
              float mt = higgs::utils::transverseMass (idileptonSystem, zvv, true);
              
            }
        }
    }
  printf ("\n");

  //##############################################
  //########     SAVING HISTO TO FILE     ########
  //##############################################
  //save control plots to file
  printf ("Results save in %s\n", outUrl.Data());

  //save all to the file
  TFile *ofile = TFile::Open (outUrl, "recreate");
  mon.Write();
  ofile->Close();

  if (outTxtFile)
    fclose (outTxtFile);
}
