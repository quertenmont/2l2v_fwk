#include <iostream>
#include <boost/shared_ptr.hpp>
#include <fstream>

#include "UserCode/llvv_fwk/interface/MacroUtils.h"
#include "UserCode/llvv_fwk/interface/SmartSelectionMonitor.h"
#include "UserCode/llvv_fwk/interface/DataEventSummaryHandler.h"
#include "UserCode/llvv_fwk/interface/LxyAnalysis.h"
#include "UserCode/llvv_fwk/interface/UEAnalysis.h"
#include "UserCode/llvv_fwk/interface/BTVAnalysis.h"
#include "UserCode/llvv_fwk/interface/LeptonEfficiencySF.h"
#include "UserCode/llvv_fwk/interface/MuScleFitCorrector.h"
#include "UserCode/llvv_fwk/interface/BtagUncertaintyComputer.h"
#include "UserCode/llvv_fwk/interface/TopPtWeighter.h"
#include "UserCode/llvv_fwk/interface/PDFInfo.h"

#include "CondFormats/JetMETObjects/interface/JetResolution.h"
#include "CondFormats/JetMETObjects/interface/JetCorrectionUncertainty.h"
#include "CondFormats/JetMETObjects/interface/JetCorrectorParameters.h"

#include "FWCore/FWLite/interface/AutoLibraryLoader.h"
#include "FWCore/PythonParameterSet/interface/MakeParameterSets.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "PhysicsTools/Utilities/interface/LumiReWeighting.h"

#include "PhysicsTools/CondLiteIO/interface/RecordWriter.h"
#include "DataFormats/FWLite/interface/Record.h"
#include "DataFormats/FWLite/interface/EventSetup.h"
#include "DataFormats/FWLite/interface/ESHandle.h"
#include "CondFormats/PhysicsToolsObjects/interface/BinningPointByMap.h"
#include "RecoBTag/PerformanceDB/interface/BtagPerformance.h"
#include "PhysicsTools/FWLite/interface/CommandLineParser.h"

#include "TSystem.h"
#include "TFile.h"
#include "TChain.h"
#include "TChainElement.h"
#include "TCanvas.h"
#include "TString.h"
#include "TDirectory.h"
#include "TEventList.h"
#include "TRandom.h"

#include "TGraphErrors.h"

#include <iostream>

using namespace std;



int main(int argc, char* argv[])
{
  // load framework libraries
  gSystem->Load( "libFWCoreFWLite" );
  AutoLibraryLoader::enable();

  //check arguments
  if ( argc < 2 ) {
    std::cout << "Usage : " << argv[0] << " parameters_cfg.py" << std::endl;
    return 0;
  }
  
  //
  // configure
  //
  const edm::ParameterSet &runProcess = edm::readPSetsFrom(argv[1])->getParameter<edm::ParameterSet>("runProcess");
  std::vector<std::string> urls=runProcess.getParameter<std::vector<std::string> >("input");
  TString url = TString(urls[0]);
  TString baseDir    = runProcess.getParameter<std::string>("dirName");
  bool runSystematics = runProcess.getParameter<bool>("runSystematics");
  TString jecDir      = runProcess.getParameter<std::string>("jecDir");
  bool isMC          = runProcess.getParameter<bool>("isMC");
  int mcTruthMode    = runProcess.getParameter<int>("mctruthmode");
  double xsec        = runProcess.getParameter<double>("xsec");
  bool isV0JetsMC(isMC && (url.Contains("DYJetsToLL_50toInf") || url.Contains("WJets")));
  bool isTTbarMC(isMC && (url.Contains("TTJets") || url.Contains("_TT_") ));
  bool isSignal(isMC && (url.Contains("TBH") || url.Contains("HTB") ));
  TString out        = runProcess.getParameter<std::string>("outdir");
  bool saveSummaryTree = runProcess.getParameter<bool>("saveSummaryTree");
  std::vector<string>  weightsFile = runProcess.getParameter<std::vector<string> >("weightsFile");

  TString wFile("");
  if(weightsFile.size()) wFile = TString(gSystem->ExpandPathName(weightsFile[0].c_str()));

  //jet energy scale uncertainties
  gSystem->ExpandPathName(jecDir);
  FactorizedJetCorrector *jesCor        = utils::cmssw::getJetCorrector(jecDir,isMC);
  JetCorrectionUncertainty *totalJESUnc = new JetCorrectionUncertainty((jecDir+"/MC_Uncertainty_AK5PFchs.txt").Data());

  //muon energy scale and uncertainties
  MuScleFitCorrector *muCor=getMuonCorrector(jecDir,url);

  
  //pdf info
  PDFInfo *mPDFInfo=0;
  if(isMC)
    {
      TString pdfUrl(url);
      pdfUrl.ReplaceAll(".root","_pdf.root");
      pdfUrl.ReplaceAll("/MC","/pdf/MC");
      mPDFInfo=new PDFInfo(pdfUrl,"cteq66.LHgrid");
      /// no need ///      for(int i=0; i<mPDFInfo->numberPDFs(); i++)
      /// no need ///	{
      /// no need ///	  TString var("_"); var+=i;
      /// no need ///	  mon.addHistogram( new TH1F("vbfcandjetdeta"+var    , ";|#Delta #eta|;Jets",                             50,0,10) );
      /// no need ///	  mon.addHistogram( new TH1F("vbfcandjet1eta"+var    , ";#eta;Jets",                                      50,0,5) );
      /// no need ///	  mon.addHistogram( new TH1F("vbfcandjet1pt"+var     , ";p_{T} [GeV];Jets",                               50,0,1000) ); //nptBins,jetptaxis) );
      /// no need ///	  mon.addHistogram( new TH1F("vbfcandjet2eta"+var    , ";#eta;Jets",                                      50,0,5) );
      /// no need ///	  mon.addHistogram( new TH1F("vbfcandjet2pt"+var     , ";p_{T} [GeV];Jets",                               50,0,500) ); //nptBins,jetptaxis) );
      /// no need ///	  mon.addHistogram( new TH1F("vbfspt"+var            , ";Relative balance (#Delta^{rel}_{p_{T}});Events", 50,0,1) );
      /// no need ///	}
      cout << "Readout " << mPDFInfo->numberPDFs() << " pdf variations" << endl;
    }
  // FIXME: 
  mPDFInfo=0;

  // FIXME: add dy reweighting for data driven DY estimation. NOT NEEDED for now: use enhanced nuisance parameter from 0-jets control region. 


   //b-tag efficiencies read b-tag efficiency map
  std::map<std::pair<TString,TString>, std::pair<TGraphErrors *,TGraphErrors *> > btagEffCorr;
  if(weightsFile.size() && isMC)
    {
      TString btagEffCorrUrl(wFile); btagEffCorrUrl += "/btagEff.root";
      gSystem->ExpandPathName(btagEffCorrUrl);
      TFile *btagF=TFile::Open(btagEffCorrUrl);
      if(btagF!=0 && !btagF->IsZombie())
	{
	  TList *dirs=btagF->GetListOfKeys();
	  for(int itagger=0; itagger<dirs->GetEntries(); itagger++)
	    {
	      TString iDir(dirs->At(itagger)->GetName());
	      btagEffCorr[ std::pair<TString,TString>(iDir,"b") ] 
		= std::pair<TGraphErrors *,TGraphErrors *>( (TGraphErrors *) btagF->Get(iDir+"/beff"),(TGraphErrors *) btagF->Get(iDir+"/sfb") );
	      btagEffCorr[ std::pair<TString,TString>(iDir,"c") ] 
		= std::pair<TGraphErrors *,TGraphErrors *>( (TGraphErrors *) btagF->Get(iDir+"/ceff"),(TGraphErrors *) btagF->Get(iDir+"/sfc") );
	      btagEffCorr[ std::pair<TString,TString>(iDir,"udsg") ] 
		= std::pair<TGraphErrors *,TGraphErrors *>( (TGraphErrors *) btagF->Get(iDir+"/udsgeff"),(TGraphErrors *) btagF->Get(iDir+"/sfudsg") );
	    }
	}
      cout << btagEffCorr.size() << " b-tag correction factors have been read" << endl;
    }
  
  std::vector<double> btagBins; btagBins.clear();
  btagBins.push_back(35  );
  btagBins.push_back(45  );
  btagBins.push_back(55  );
  btagBins.push_back(65  );
  btagBins.push_back(75  );
  btagBins.push_back(90  );
  btagBins.push_back(110 );
  btagBins.push_back(140 );
  btagBins.push_back(185 );
  btagBins.push_back(235 );
  btagBins.push_back(290 );
  btagBins.push_back(360 );
  btagBins.push_back(450 );
  btagBins.push_back(550 );
  btagBins.push_back(700 );
  

  //
  // check input file
  //
  TFile *inF = TFile::Open(url);
  if(inF==0) return -1;
  if(inF->IsZombie()) return -1;
  TString proctag=gSystem->BaseName(url);
  Ssiz_t pos=proctag.Index(".root");
  proctag.Remove(pos,proctag.Length());

  bool filterOnlyEE(false), filterOnlyEMU(false), filterOnlyMUMU(false);
  if(!isMC)
    {
      if(url.Contains("DoubleEle")) filterOnlyEE=true;
      if(url.Contains("DoubleMu"))  filterOnlyMUMU=true;
      if(url.Contains("MuEG"))      filterOnlyEMU=true;
    }
  
  //
  // pileup reweighter
  //
  std::vector<double> dataPileupDistributionDouble = runProcess.getParameter< std::vector<double> >("datapileup");
  std::vector<float> dataPileupDistribution; for(unsigned int i=0;i<dataPileupDistributionDouble.size();i++){dataPileupDistribution.push_back(dataPileupDistributionDouble[i]);}
  std::vector<float> mcPileupDistribution;
  if(isMC){
    TString puDist(baseDir+"/pileup");
    TH1F* histo = (TH1F *) inF->Get(puDist);
    if(!histo)std::cout<<"pileup histogram is null!!!\n";
    for(int i=1;i<=histo->GetNbinsX();i++){mcPileupDistribution.push_back(histo->GetBinContent(i));}
    delete histo;
  }
  while(mcPileupDistribution.size()<dataPileupDistribution.size())  mcPileupDistribution.push_back(0.0);
  while(mcPileupDistribution.size()>dataPileupDistribution.size())dataPileupDistribution.push_back(0.0);
  gROOT->cd();  //THIS LINE IS NEEDED TO MAKE SURE THAT HISTOGRAM INTERNALLY PRODUCED IN LumiReWeighting ARE NOT DESTROYED WHEN CLOSING THE FILE
  edm::LumiReWeighting *LumiWeights= isMC ? new edm::LumiReWeighting(mcPileupDistribution,dataPileupDistribution): 0;
  utils::cmssw::PuShifter_t PuShifters;
  if(isMC) PuShifters=utils::cmssw::getPUshifters(dataPileupDistribution,0.05);

  //systematics variations for all selections steps
  std::vector<TString> systVars(1,"");
  if(runSystematics && isMC)
    {
      systVars.push_back("jerup" );     systVars.push_back("jerdown"   );
      systVars.push_back("jesup" );     systVars.push_back("jesdown"   );
      //systVars.push_back("lesup" );   systVars.push_back("lesdown"   );
      systVars.push_back("leffup");     systVars.push_back("leffdown"  );
      systVars.push_back("puup"  );     systVars.push_back("pudown"   );
      systVars.push_back("umetup");     systVars.push_back("umetdown" );
      systVars.push_back("btagup");     systVars.push_back("btagdown" );
      systVars.push_back("unbtagup");   systVars.push_back("unbtagdown" );
      if(isTTbarMC) {systVars.push_back("topptuncup"); systVars.push_back("topptuncdown"); }
      //      systVars.push_back(); systVars.push_back();

      if(isTTbarMC || isSignal) { systVars.push_back("pdfup"); systVars.push_back("pdfdown"); }
      cout << "Systematics will be computed for this analysis - this will take a bit" << endl;
    }
  
  //
  // control histograms
  //
  SmartSelectionMonitor controlHistos;
  TH1F* Hhepup        = (TH1F* )controlHistos.addHistogram(new TH1F ("heupnup"    , "hepupnup"    ,20,0,20) ) ;
  TH1F* Hcutflow      = (TH1F*) controlHistos.addHistogram(new TH1F ("cutflow"    , "cutflow"    ,5,0,5) ) ;
  TH1F* Hoptim_systs = (TH1F*) controlHistos.addHistogram(new TH1F ("optim_systs" , ";syst;", systVars.size(),0,systVars.size()) );



  //vertex multiplicity
  controlHistos.addHistogram( new TH1F ("nvertices", "; Vertex multiplicity; Events", 50, 0.,50.) );
  controlHistos.addHistogram( new TH1F ("nverticesUnweighted", "; Vertex multiplicity; Events", 50, 0.,50.) );

  TString labels[]={"1 leptons", "2 jets", " bjets" , "W mass", "Top quark mass", "E_{T}^{miss}"};
  int nsteps=sizeof(labels)/sizeof(TString);


  //event selection histogram - per systematic variation
  for(size_t ivar=0;ivar<systVars.size();++ivar){
    TString var=systVars[ivar];

    Hoptim_systs->GetXaxis()->SetBinLabel(ivar+1, var);
    
    TH1F *cutflowH = (TH1F *)controlHistos.addHistogram( new TH1F("evtflow"+var,";Cutflow;Events",nsteps,0,nsteps) );
    for(int ibin=0; ibin<nsteps; ibin++) cutflowH->GetXaxis()->SetBinLabel(ibin+1,labels[ibin]);
   
    TH1D *finalCutflowH = new TH1D("finalevtflow"+var,";Category;Events",6,0,6); 
    finalCutflowH->GetXaxis()->SetBinLabel(1,"=0 btags");
    finalCutflowH->GetXaxis()->SetBinLabel(2,"=1 btags");
    finalCutflowH->GetXaxis()->SetBinLabel(3,"=2 btags");
    finalCutflowH->GetXaxis()->SetBinLabel(4,"=3 btags");
    finalCutflowH->GetXaxis()->SetBinLabel(5,"=4 btags");
    finalCutflowH->GetXaxis()->SetBinLabel(6,"#geq5 btags");
    controlHistos.addHistogram( finalCutflowH );

    TH1D *finalCutflow2btagsH = new TH1D("finalevtflow2btags"+var,";Category;Events",4,2,6); 
    finalCutflow2btagsH->GetXaxis()->SetBinLabel(1,"=2 btags");
    finalCutflow2btagsH->GetXaxis()->SetBinLabel(2,"=3 btags");
    finalCutflow2btagsH->GetXaxis()->SetBinLabel(3,"=4 btags");
    finalCutflow2btagsH->GetXaxis()->SetBinLabel(4,"#geq5 btags");
    controlHistos.addHistogram( finalCutflow2btagsH );

    TH1D *finalCutflowH_0 = new TH1D("finalevtflow0"+var,";Category;Events",1,0,1); 
    finalCutflowH_0->GetXaxis()->SetBinLabel(1,"=0 jets");
    controlHistos.addHistogram( finalCutflowH_0 );
    TH1D *finalCutflowH_1 = new TH1D("finalevtflow1"+var,";Category;Events",1,0,1); 
    finalCutflowH_1->GetXaxis()->SetBinLabel(1,"=1 jets");
    controlHistos.addHistogram( finalCutflowH_1 );
    TH1D *finalCutflowH_2 = new TH1D("finalevtflow2"+var,";Category;Events",1,0,1); 
    finalCutflowH_2->GetXaxis()->SetBinLabel(1,"=2 jets");
    controlHistos.addHistogram( finalCutflowH_2 );
    TH1D *finalCutflowH_3 = new TH1D("finalevtflow3"+var,";Category;Events",1,0,1); 
    finalCutflowH_3->GetXaxis()->SetBinLabel(1,"=3 jets");
    controlHistos.addHistogram( finalCutflowH_3 );
    TH1D *finalCutflowH_4 = new TH1D("finalevtflow4"+var,";Category;Events",1,0,1); 
    finalCutflowH_4->GetXaxis()->SetBinLabel(1,"=4 jets");
    controlHistos.addHistogram( finalCutflowH_4 );
    TH1D *finalCutflowH_5 = new TH1D("finalevtflow5"+var,";Category;Events",1,0,1); 
    finalCutflowH_5->GetXaxis()->SetBinLabel(1,"#geq5 jets");
    controlHistos.addHistogram( finalCutflowH_5 );
    
    //    TString ctrlCats[]={"","eq1jets","lowmet","eq1jetslowmet","zlowmet","zeq1jets","zeq1jetslowmet","z"};
    TString ctrlCats[]={"","eq2leptons","eq1jets","eq2jets","geq2btags"//, // through the base cutflow
			                                               //"lowmet","eq1jetslowmet","zowmet","zeq1jets","zeq1jetslowmet","z" // for DY rescaling
    };
    for(size_t k=0;k<sizeof(ctrlCats)/sizeof(TString); k++)
      {

	if(var!="") continue; // Do not create unneeded syst histograms
	controlHistos.addHistogram( new TH1F(ctrlCats[k]+"emva"+var, "; e-id MVA; Electrons", 50, 0.95,1.0) );
	controlHistos.addHistogram( new TH1F(ctrlCats[k]+"mll"+var,";Dilepton invariant mass [GeV];Events",50,0,250) );
	controlHistos.addHistogram( new TH1F(ctrlCats[k]+"ptll"+var,";Dilepton transverse momentum [GeV];Events",50,0,250) );
	controlHistos.addHistogram( new TH1F(ctrlCats[k]+"pte"+var,";Electron transverse momentum [GeV];Events",50,0,500) );
	controlHistos.addHistogram( new TH1F(ctrlCats[k]+"ptmu"+var,";Muon transverse momentum [GeV];Events",50,0,500) );
	controlHistos.addHistogram( new TH1F(ctrlCats[k]+"ptlep"+var,";Lepton transverse momentum [GeV];Events",50,0,500) ); 
	controlHistos.addHistogram( new TH1F(ctrlCats[k]+"sumpt"+var,";Sum of lepton transverse momenta [GeV];Events",50,0,500) );
	controlHistos.addHistogram( new TH1F(ctrlCats[k]+"ptmin"+var,";Minimum lepton transverse momentum [GeV];Events",50,0,500) );

	controlHistos.addHistogram( new TH1F(ctrlCats[k]+"dphill"+var, ";D#phi(l,l');Events",100,0,4) );
	controlHistos.addHistogram( new TH1F(ctrlCats[k]+"drll"+var  , ";DR(l,l');Events",100,0,6)   );

	// for DY estimation
	controlHistos.addHistogram( new TH1F(ctrlCats[k]+"dilarccosine"+var,";#theta(l,l') [rad];Events",50,0,3.2) );
	controlHistos.addHistogram( new TH1F(ctrlCats[k]+"mtsum"+var,";M_{T}(l^{1},E_{T}^{miss})+M_{T}(l^{2},E_{T}^{miss}) [GeV];Events",100,0,1000) );


	controlHistos.addHistogram( new TH1F(ctrlCats[k]+"met"+var,";Missing transverse energy [GeV];Events",50,0,500) );
	controlHistos.addHistogram( new TH1F(ctrlCats[k]+"metnotoppt"+var,";Missing transverse energy [GeV];Events",50,0,500) );
	
	controlHistos.addHistogram( new TH1F(ctrlCats[k]+"ht"+var,";H_{T} [GeV];Events",50,0,1000) );
	controlHistos.addHistogram( new TH1F(ctrlCats[k]+"htb"+var,";H_{T} (bjets) [GeV];Events",50,0,1000) );
	controlHistos.addHistogram( new TH1F(ctrlCats[k]+"htnol"+var,"; H_[T] (no leptons) [GeV];Events",50,0,1000) );
	controlHistos.addHistogram( new TH1F(ctrlCats[k]+"htbnol"+var,"; H_[T] (bjets, no leptons) [GeV];Events",50,0,1000) );
	
	TH1F *h=(TH1F *)controlHistos.addHistogram( new TH1F(ctrlCats[k]+"njets"+var,";Jet multiplicity;Events",8,0,8) );
	TH1F *h2=(TH1F *)controlHistos.addHistogram( new TH1F(ctrlCats[k]+"njets"+var,";Jet multiplicity;Events",8,0,8) );
	TH1F *hb=(TH1F *)controlHistos.addHistogram( new TH1F(ctrlCats[k]+"nbjets"+var,";b-Jet multiplicity;Events",6,0,6) );
	for(int ibin=1; ibin<=h->GetXaxis()->GetNbins(); ibin++)
	  {
	    TString label( ibin==h->GetXaxis()->GetNbins() ? "#geq" : "=");
	    label += (ibin-1);
	    label += " jets";
	    h->GetXaxis()->SetBinLabel(ibin,label);
	    h2->GetXaxis()->SetBinLabel(ibin,label);
	    if(ibin<=hb->GetXaxis()->GetNbins()) hb->GetXaxis()->SetBinLabel(ibin,label);
	    if(ibin==1) continue;
	    label="jet"; label+=(ibin-1);
	 
	    //if(ibin>=3) continue; // unnecessary histos at the moment
	    controlHistos.addHistogram( new TH1F(ctrlCats[k]+"pt"+label+"pt"+var,";Transverse momentum [GeV];Events",50,0,250) );
	    controlHistos.addHistogram( new TH1F(ctrlCats[k]+"pt"+label+"nobsmearpt"+var,";Transverse momentum [GeV];Events",50,0,250) );
	    controlHistos.addHistogram( new TH1F(ctrlCats[k]+"pt"+label+"smearpt"+var,";Transverse momentum [GeV];Events",50,0,250) );
	    controlHistos.addHistogram( new TH1F(ctrlCats[k]+"pt"+label+"eta"+var,";Pseudo-rapidity;Events",50,0,2.5) );
	    TH1F *flav1H=(TH1F *)controlHistos.addHistogram( new TH1F(ctrlCats[k]+"pt"+label+"flav"+var,";Flavor;Events",5,0,5) );
	    controlHistos.addHistogram( new TH1F(ctrlCats[k]+"btag"+label+"pt"+var,";Transverse momentum [GeV];Events",50,0,250) );
	    controlHistos.addHistogram( new TH1F(ctrlCats[k]+"btag"+label+"ptnotoppt"+var,";Transverse momentum [GeV];Events",50,0,250) );
	    controlHistos.addHistogram( new TH1F(ctrlCats[k]+"btag"+label+"eta"+var,";Pseudo-rapidity;Events",50,0,2.5) );
	    TH1F *flav2H=(TH1F *)controlHistos.addHistogram( new TH1F(ctrlCats[k]+"btag"+label+"flav"+var,";Flavor;Events",5,0,5) );
	    for(int ibin=1; ibin<=5; ibin++)
	      {
		TString label("unmatched");
		if(ibin==2) label="g";
		if(ibin==3) label="uds";
		if(ibin==4) label="c";
		if(ibin==5) label="b";
		flav1H->GetXaxis()->SetBinLabel(ibin,label);
		flav2H->GetXaxis()->SetBinLabel(ibin,label);
	      }
	  }
	
      }

  }
  //lepton efficiencies
  LeptonEfficiencySF lepEff;

  //UEAnalysis ueAn(controlHistos); // FIXME: implement runSystematics here or add a different class for managing histos of interest :)
  //  BTVAnalysis btvAn(controlHistos,runSystematics);
  //  LxyAnalysis lxyAn(controlHistos,runSystematics);
  
  ///
  // process events file
  //
  DataEventSummaryHandler evSummary;
  if( !evSummary.attach( (TTree *) inF->Get(baseDir+"/data") ) )  { inF->Close();  return -1; }  
  const Int_t totalEntries=evSummary.getEntries();
  
  float cnorm=1.0;
  if(isMC){
    TH1F* cutflowH = (TH1F *) inF->Get(baseDir+"/cutflow");
    if(cutflowH) cnorm=cutflowH->GetBinContent(1);
  }
  Hcutflow->SetBinContent(1,cnorm);

  cout << "Processing: " << proctag << " @ " << url << endl
       << "Initial number of events: " << cnorm << endl
       << "Events in tree:           " << totalEntries << endl
       << " xSec x BR:               " << xsec << endl;



  //check if a summary should be saved
  Float_t evSummaryWeight(1.0);
  Float_t xsecWeight(isMC ? xsec/cnorm : 1.0);
  TFile *spyFile=0;
  TDirectory *spyDir=0;
  DataEventSummaryHandler *spyEvents=0;
  if(saveSummaryTree)
    {
      gSystem->Exec("mkdir -p " + out);
      gDirectory->SaveSelf();
      TString summaryName(out + "/" + proctag);
      if(mcTruthMode!=0) { summaryName += "_filt"; summaryName += mcTruthMode; } 
      summaryName += "_summary.root";
      gSystem->ExpandPathName(summaryName);
      cout << "Creating event summary file @ " << summaryName << endl;

      //open file
      spyEvents = new DataEventSummaryHandler;
      spyFile = TFile::Open(summaryName,"RECREATE");
      spyFile->rmdir(proctag);
      spyDir = spyFile->mkdir(proctag);
      TTree *outT = evSummary.getTree()->CloneTree(0);
      outT->SetTitle("Event summary");
      outT->SetDirectory(spyDir);
      outT->SetAutoSave(1000000);
      outT->Branch("weight",&evSummaryWeight,"weight/F"); 
      spyEvents->init(outT,false);
    }

  TopPtWeighter* topPtWgt=0;
  if(isTTbarMC){
    TString shapesDir("");
    if(weightsFile.size()) shapesDir=wFile;
    topPtWgt = new TopPtWeighter(proctag, out, shapesDir, evSummary.getTree() );
  }
  
  //
  // analyze (puf...)
  //
  DuplicatesChecker duplicatesChecker;
  int nDuplicates(0);
  for (int inum=0; inum < totalEntries; ++inum)
    {
      if(inum%500==0) { printf("\r [ %d/100 ]",int(100*float(inum)/float(totalEntries))); cout << flush; }
      evSummary.getEntry(inum);
      DataEventSummary &ev = evSummary.getEvent();
      if(!isMC && duplicatesChecker.isDuplicate( ev.run, ev.lumi, ev.event) ) { nDuplicates++; continue; }

      //pileup weight
      float weightNom(1.0),weightUp(1.0), weightDown(1.0);
      if(LumiWeights) {
	weightNom     = LumiWeights->weight(ev.ngenITpu);
	weightUp   = weightNom*PuShifters[utils::cmssw::PUUP]->Eval(ev.ngenITpu);
	weightDown = weightNom*PuShifters[utils::cmssw::PUDOWN]->Eval(ev.ngenITpu);
      }
      
      if(isV0JetsMC && ev.nup>5)                          continue;
      Hhepup->Fill(ev.nup,1);

      //MC truth (filtering for other ttbar)


      data::PhysicsObjectCollection_t gen=evSummary.getPhysicsObject(DataEventSummaryHandler::GENPARTICLES);
      double tPt(0.), tbarPt(0.); // top pt reweighting - dummy value results in weight equal to 1 if not set in loop 
      bool hasTop(false);
      int ngenLeptonsStatus3(0);
      float wgtTopPt(1.0), wgtTopPtUp(1.0), wgtTopPtDown(1.0);
      if(isMC)
	{
	  for(size_t igen=0; igen<gen.size(); igen++){
	    if(gen[igen].get("status")!=3) continue;
	    int absid=abs(gen[igen].get("id"));
	    if(absid==6){
	      hasTop=true;
	      if(isTTbarMC){
		if(gen[igen].get("id") > 0) tPt=gen[igen].pt();
		else                        tbarPt=gen[igen].pt();
	      }
	    }
	    if(absid!=11 && absid!=13 && absid!=15) continue;
	    ngenLeptonsStatus3++;
	  }
	  if(mcTruthMode==1 && (ngenLeptonsStatus3!=2 || !hasTop)) continue;
	  if(mcTruthMode==2 && (ngenLeptonsStatus3==2 || !hasTop)) continue;
	}

      if(tPt>0 && tbarPt>0 && topPtWgt)
	{
	  topPtWgt->computeWeight(tPt,tbarPt);
	  topPtWgt->getEventWeight(wgtTopPt, wgtTopPtUp, wgtTopPtDown);
	  wgtTopPtUp /= wgtTopPt;
	  wgtTopPtDown /= wgtTopPt;
	}
      
           
      Hcutflow->Fill(1,1);
      Hcutflow->Fill(2,weightNom);
      Hcutflow->Fill(3,weightUp);
      Hcutflow->Fill(4,weightDown);

      //float Q2Weight_plus(1.0), Q2Weight_down(1.0);
      float PDFWeight_plus(1.0), PDFWeight_down(1.0);
      if(isTTbarMC || isSignal)
	{
	  if(mPDFInfo)
	    {
	      std::vector<float> wgts=mPDFInfo->getWeights(inum);
	      for(size_t ipw=0; ipw<wgts.size(); ipw++)
		{
		  PDFWeight_plus = TMath::Max(PDFWeight_plus,wgts[ipw]);
		  PDFWeight_down = TMath::Min(PDFWeight_down,wgts[ipw]);
		}
	    }
	  ///if(Q2weightsGr.size()==2)
	  ///  {
	  ///    Q2Weight_plus = Q2weightsGr[0]->Eval(genll.pt());
	  ///    Q2Weight_down = Q2weightsGr[1]->Eval(genll.pt());
	  ///  }
	}
      

      
      for(size_t ivar=0; ivar<systVars.size(); ++ivar){
	TString var=systVars[ivar];
	
	//trigger bits
	bool eeTrigger   = ev.t_bits[0];
	bool emuTrigger  = ev.t_bits[4] || ev.t_bits[5];
	bool mumuTrigger = ev.t_bits[2] || ev.t_bits[3];
	if(filterOnlyEE)   {                   emuTrigger=false;  mumuTrigger=false; }
	if(filterOnlyEMU)  { eeTrigger=false;                     mumuTrigger=false; }
	if(filterOnlyMUMU) { eeTrigger=false;  emuTrigger=false;                     }
	

	data::PhysicsObjectCollection_t leptons=evSummary.getPhysicsObject(DataEventSummaryHandler::LEPTONS);
	data::PhysicsObjectCollection_t selLeptons;
	for(size_t ilep=0; ilep<leptons.size(); ilep++)
	  {
	    Int_t id=leptons[ilep].get("id");
	    bool passKin(true),passId(true),passIso(true);
	    if(abs(id)==11)
	      {
		float sceta=leptons[ilep].getVal("sceta");
		Float_t gIso    = leptons[ilep].getVal("gIso03");
		Float_t chIso   = leptons[ilep].getVal("chIso03");
		//Float_t puchIso = leptons[ilep].getVal("puchIso03");
		Float_t nhIso   = leptons[ilep].getVal("nhIso03");
		float relIso=(TMath::Max(nhIso+gIso-ev.rho*utils::cmssw::getEffectiveArea(11,sceta),Float_t(0.))+chIso)/leptons[ilep].pt();
		if(leptons[ilep].pt()<20 || leptons[ilep].pt() >80) passKin=false;
		if(fabs(leptons[ilep].eta())>1.5)              passKin=false;
		if(fabs(sceta)>1.4442 && fabs(sceta)<1.5660)   passKin=false;
		if(leptons[ilep].getFlag("isconv"))            passId=false;
		if(leptons[ilep].getVal("tk_d0")>0.4)          passId=false;
		if(leptons[ilep].getVal("tk_lostInnerHits")>0) passId=false;
		if(leptons[ilep].getVal("mvatrig")<0.5)        passId=false;
		if(relIso>0.15)                                passIso=false;
	      }
	    else
	      {

		if(muCor){
		  TLorentzVector p4(leptons[ilep].px(),leptons[ilep].py(),leptons[ilep].pz(),leptons[ilep].energy());
		  muCor->applyPtCorrection(p4 , id<0 ? -1 : 1 );
		  if(isMC) muCor->applyPtSmearing(p4, id<0 ? -1 : 1, false);
		  leptons[ilep].SetPxPyPzE(p4.Px(),p4.Py(),p4.Pz(),p4.E());
		}
		
		Int_t idbits    = leptons[ilep].get("idbits");
		bool isTight    = ((idbits >> 10) & 0x1);
		//bool isLoose    = ((idbits >> 8) & 0x1);
		Float_t gIso    = leptons[ilep].getVal("gIso04");
		Float_t chIso   = leptons[ilep].getVal("chIso04");
		Float_t puchIso = leptons[ilep].getVal("puchIso04");
		Float_t nhIso   = leptons[ilep].getVal("nhIso04");
		Float_t relIso=(TMath::Max(nhIso+gIso-0.5*puchIso,0.)+chIso)/leptons[ilep].pt();
		if(leptons[ilep].pt()<20 || leptons[ilep].pt() > 80) passKin=false;
		if(fabs(leptons[ilep].eta())>1.5)              passKin=false;
		if(!isTight)                                   passId=false;
		//if(!isLoose)                                   passId=false;
		if(relIso>0.12)                                passIso=false;
	      }
	    
	    if(!passKin || !passId || !passIso) continue;
	    selLeptons.push_back(leptons[ilep]);
	  }
	sort(selLeptons.begin(),selLeptons.end(),data::PhysicsObject_t::sortByPt);
	
	//select the leptons
	if(!eeTrigger && !emuTrigger && !mumuTrigger) continue;
	if(selLeptons.size()<2) continue;
	
	//apply data/mc correction factors
	ev.cat=1;
	float llScaleFactor(1.0), llScaleFactor_plus(1.0), llScaleFactor_minus(1.0);
	for(size_t ilep=0; ilep<2; ilep++)
	  {
	    ev.cat *= selLeptons[ilep].get("id");
	    int id(abs(selLeptons[ilep].get("id")));
	    std::pair<float,float> ieff= lepEff.getLeptonEfficiency( selLeptons[ilep].pt(), selLeptons[ilep].eta(), id,  id ==11 ? "loose" : "tight" );
	    llScaleFactor *= isMC ? ieff.first : 1.0;
	    llScaleFactor_plus  *= (1.0+ieff.second );
	    llScaleFactor_minus *= (1.0-ieff.second );
	  }
	
	
	//set the channel
	TString chName;
	//bool isOS(ev.cat<0);
	bool isSameFlavor(false);
	if     (abs(ev.cat)==11*11 && eeTrigger)   { chName="ee";  isSameFlavor=true;  if(ngenLeptonsStatus3>=2) llScaleFactor*=0.972; }
	else if(abs(ev.cat)==11*13 && emuTrigger)  { chName="emu";                     if(ngenLeptonsStatus3>=2) llScaleFactor*=0.968; }
	else if(abs(ev.cat)==13*13 && mumuTrigger) { chName="mumu"; isSameFlavor=true; if(ngenLeptonsStatus3>=2) llScaleFactor*=0.955; }
	else  continue;
	std::vector<TString> ch(1,chName);
	if(isSameFlavor) ch.push_back("ll");

	float weight(weightNom);	
	// PU shift
	if(var=="puup")   weight = weightUp;
	if(var=="pudown") weight = weightDown;
	
	weight *= llScaleFactor*wgtTopPt;

	if(var=="leffup")   weight *= llScaleFactor_plus;
	if(var=="leffdown") weight *= llScaleFactor_minus;

	if(var=="topptuncup")   weight *= wgtTopPtUp;
	if(var=="topptuncdown") weight *= wgtTopPtDown;
	
	if(var=="pdfup")    weight *= PDFWeight_plus;
        if(var=="pdfdown")  weight *= PDFWeight_down;
	
	//the met
	data::PhysicsObjectCollection_t recoMet=evSummary.getPhysicsObject(DataEventSummaryHandler::MET);
	//select the jets
	data::PhysicsObjectCollection_t jets=evSummary.getPhysicsObject(DataEventSummaryHandler::JETS);
	utils::cmssw::updateJEC(jets,jesCor,totalJESUnc,ev.rho,ev.nvtx,isMC);
	std::vector<LorentzVector> metVars=utils::cmssw::getMETvariations(recoMet[0],jets,selLeptons,isMC);
	
	int metIdx(0);
	if(var == "jerup")    metIdx=utils::cmssw::JERUP;   
	if(var == "jerdown")  metIdx=utils::cmssw::JERDOWN; 
	if(var == "jesup")    metIdx=utils::cmssw::JESUP;   
	if(var == "jesdown")  metIdx=utils::cmssw::JESDOWN; 
	if(var == "umetup")   metIdx=utils::cmssw::UMETUP;
	if(var == "umetdown") metIdx=utils::cmssw::UMETDOWN;
	LorentzVector met=metVars[metIdx];

	data::PhysicsObjectCollection_t looseJets,selJets,selbJets;
	for(size_t ijet=0; ijet<jets.size(); ijet++)
	  {
	    if(var == "jerup")   jets[ijet].setVal("pt", jets[ijet].getVal("jerup")   );
	    if(var == "jerdown") jets[ijet].setVal("pt", jets[ijet].getVal("jerdown") );
	    if(var == "jesup")   jets[ijet].setVal("pt", jets[ijet].getVal("jesup")   );
	    if(var == "jesdown") jets[ijet].setVal("pt", jets[ijet].getVal("jesdown") );
	    
	    //cross-clean with selected leptons 
	    double minDRlj(9999.);
	    for(size_t ilep=0; ilep<selLeptons.size(); ilep++)
	      minDRlj = TMath::Min( minDRlj, deltaR(jets[ijet],selLeptons[ilep]) );
	    if(minDRlj<0.4) continue;
	    
	    //require to pass the loose id
	    Int_t idbits=jets[ijet].get("idbits");
	    bool passPFloose( ((idbits>>0) & 0x1));
	    if(!passPFloose) continue;
	    double jetpt(jets[ijet].pt());

	    //top candidate jets
	    looseJets.push_back(jets[ijet]);
	    if(jetpt<30 || fabs(jets[ijet].eta())>1.5 ) continue;
	    selJets.push_back(jets[ijet]);
	    //if(jets[ijet].getVal("csv") <= 0.405) continue; // CSVV1L
	    bool hasCSVV1L(jets[ijet].getVal("csv") > 0.405); // CSVV1L
	    bool hasBtagCorr(hasCSVV1L);
	    if(isMC){
	      //set a unique seed
	      double bseed_sin_phi = sin(jets[ijet].phi()*1000000);
	      double bseed = abs(static_cast<int>(bseed_sin_phi*100000));
	      
	      // get jet flavour
	      const data::PhysicsObject_t &bgenJet=jets[ijet].getObject("genJet");
	      int bflavid=bgenJet.info.find("id")->second;
	      
	      //Initialize class
	      //	      BTagSFUtil* btsfutil = new BTagSFUtil( bseed );
	      BTagSFUtil btsfutil( bseed );
	      //btsfutil.modifyBTagsWithSF(hasCSVV1L, bflavid, 1., 1., 1., 1.);//0.98, 0.841, 1.21, 0.137);
	      
	      TString flavKey("udsg");
	      if(abs(bflavid)==4) flavKey="c";
	      if(abs(bflavid)==5) flavKey="b";
	      std::pair<TString,TString> btagKey("csvL",flavKey);
	      if(btagEffCorr.find(btagKey)!=btagEffCorr.end())
		{
		  TGraphErrors* mceffGr=btagEffCorr[btagKey].first;
		  TGraphErrors* sfGr=btagEffCorr[btagKey].second;
		  if(mceffGr && sfGr){
		    float eff=mceffGr->Eval(jetpt);
		    float sf=sfGr->Eval(jetpt);	
		    if(var == "btagup" || var == "btagdown" || var == "unbtagup" || var == "unbtagdown"){
		      // Apply uncertainty shift
		      float sferr(0.), delta(1000000.);
		      for(size_t ind=0; ind<btagBins.size(); ++ind){
			float tempDelta( fabs(jetpt - btagBins[ind]) );
			if(tempDelta<delta){
			  delta=tempDelta;
			  if      (var == "btagup"   || var == "unbtagup")   sferr=fabs(sfGr->GetErrorYhigh(ind));  // Ensure positive number
			  else if (var == "btagdown" || var == "unbtagdown") sferr=0-fabs(sfGr->GetErrorYlow(ind)); // Ensure negative number
			}
		      }
 
//		      for(int ind=0; ind<sfGr->GetN(); ++ind){
//			double xv(0.), yv(0.);
//			sfGr->GetPoint(ind,xv,yv);
//			float tempDelta(jetpt - xv );
//			if(tempDelta<delta){
//			  delta=tempDelta; 
//			  if      (var == "btagup"   || var == "unbtagup")   sferr=fabs(sfGr->GetErrorYhigh(ind));  // Ensure positive number
//			  else if (var == "btagdown" || var == "unbtagdown") sferr=0-fabs(sfGr->GetErrorYlow(ind)); // Ensure negative number
//			}
//		      }
		      sf+=sferr;
		    }
		    btsfutil.modifyBTagsWithSF(hasBtagCorr, sf, eff);	    
		  }
		}
	    }
	    //	    if(!hasCSVV1L) continue;
	    if(!hasBtagCorr) continue;
	    selbJets.push_back(jets[ijet]);
	  }
	sort(looseJets.begin(),looseJets.end(),data::PhysicsObject_t::sortByPt);
	sort(selJets.begin(),  selJets.end(),  data::PhysicsObject_t::sortByCSV);
	sort(selbJets.begin(),  selbJets.end(),  data::PhysicsObject_t::sortByCSV);
	
	//select the event
	if(selLeptons.size()<1 || selLeptons.size()>2) continue;
	controlHistos.fillHisto("evtflow"+var, ch, 0, weight);
	if(var==""){
	  controlHistos.fillHisto("nvertices",  ch, ev.nvtx, weight);
	  controlHistos.fillHisto("nverticesUnweighted",  ch, ev.nvtx, (isMC ? xsec/cnorm : 1.0)*llScaleFactor*wgtTopPt);
	}
	
	LorentzVector ll=selLeptons[0]+selLeptons[1];
	LorentzVector lnu=selLeptons[0]+met;
	LorentzVector lnub;
	float mll=ll.mass();
	float thetall=utils::cmssw::getArcCos<LorentzVector>(selLeptons[0],selLeptons[1]);
	float mtsum=utils::cmssw::getMT<LorentzVector>(selLeptons[0],met)+utils::cmssw::getMT<LorentzVector>(selLeptons[1],met);
	bool isWcand( fabs(mll-75)<15);
	//bool isOS( selLeptons[0].get("id")*selLeptons[1].get("id") < 0 ); 
	bool passJetSelection(selJets.size()>=2);
	bool passMetSelection(met.pt()<70);//!isSameFlavor || met[0].pt()>40); (charged Higgs case expects high MET 
	bool passBtagSelection(selbJets.size()>=2);
	if(passBtagSelection){
	  passBtagSelection = passBtagSelection && (selbJets[0].pt()>50 || selbJets[1].pt()>50);
	  lnub=lnu+selbJets[0];
	}
	bool isTopCand(fabs(lnub.mass()-170.) <20.);
	
	//control distributions
	std::vector<TString> ctrlCategs;
  
	if(selLeptons.size()>0)                                                                                      ctrlCategs.push_back("1_2leptons");
	if(selLeptons.size()>0 && passJetSelection)                                                                  ctrlCategs.push_back("geq2jets");
	if(selLeptons.size()>0 && passJetSelection && passBtagSelection)                                             ctrlCategs.push_back("geq2btags");
	if(selLeptons.size()>0 && passJetSelection && passBtagSelection && isWcand)                                  ctrlCategs.push_back("wcand");
	if(selLeptons.size()>0 && passJetSelection && passBtagSelection && isWcand && isTopCand)                     ctrlCategs.push_back("topcand");
	if(selLeptons.size()>0 && passJetSelection && passBtagSelection && isWcand && isTopCand && passMetSelection) ctrlCategs.push_back("met");

	for(size_t icat=0; icat<ctrlCategs.size(); icat++)
	  {
	    double ptmin(999999999.);
	    double sumpt(0.);
	    double ht(0), htb(0), htnol(0), htbnol(0);
	    for(size_t ilep=0; ilep<2; ilep++)
	      {
		sumpt+= selLeptons[ilep].pt();
		ht+= selLeptons[ilep].pt();
		htb+= selLeptons[ilep].pt();
		if(selLeptons[ilep].pt() < ptmin)
		  ptmin = selLeptons[ilep].pt();
		if(abs(selLeptons[ilep].get("id"))==11){
		  controlHistos.fillHisto(ctrlCategs[icat]+"emva"+var, ch, selLeptons[ilep].getVal("mvatrig"), weight);
		  controlHistos.fillHisto(ctrlCategs[icat]+"pte"+var,  ch, selLeptons[ilep].pt(),        weight);
		  controlHistos.fillHisto(ctrlCategs[icat]+"ptlep"+var,ch, selLeptons[ilep].pt(),        weight);
		}
		else if(abs(selLeptons[ilep].get("id"))==13){
		  controlHistos.fillHisto(ctrlCategs[icat]+"ptmu"+var,  ch, selLeptons[ilep].pt(),        weight);
		  controlHistos.fillHisto(ctrlCategs[icat]+"ptlep"+var, ch, selLeptons[ilep].pt(),        weight);
		}
	      }
	    controlHistos.fillHisto(ctrlCategs[icat]+"sumpt"+var, ch, sumpt, weight);
	    
	    // index 0 and 1
	    double dphi( deltaPhi(selLeptons[0].phi(), selLeptons[1].phi()) );
	    double dR( deltaR(selLeptons[0].eta(),selLeptons[0].phi(),selLeptons[1].eta(),selLeptons[1].phi()) );	    
	    controlHistos.fillHisto( ctrlCategs[icat]+"dphill"+var, ch, abs(dphi), weight);
	    controlHistos.fillHisto( ctrlCategs[icat]+"drll"+var  , ch, dR  , weight);
	    
	    
	    for(size_t ijet=0; ijet<selJets.size(); ++ijet) // FIXME: am I sure that for HT I want to use only jets with pt>30, eta<2.5?
	      {
		ht+=selJets[ijet].pt();
		htnol+=selJets[ijet].pt();
		if(ijet<selbJets.size())
		  {
		    htb+=selbJets[ijet].pt();
		    htbnol+=selbJets[ijet].pt();
		  }
	      }
	    ht+=met.pt();
	    htb+=met.pt();
	    htnol+=met.pt();
	    htbnol+=met.pt();
	    
	    controlHistos.fillHisto(ctrlCategs[icat]+"ptmin"+var,        ch, ptmin,           weight);
	    controlHistos.fillHisto(ctrlCategs[icat]+"mll"+var,          ch, mll,             weight);
	    controlHistos.fillHisto(ctrlCategs[icat]+"ptll"+var,         ch, ll.pt(),         weight);
	    controlHistos.fillHisto(ctrlCategs[icat]+"mtsum"+var,        ch, mtsum,           weight);
	    controlHistos.fillHisto(ctrlCategs[icat]+"dilarccosine"+var, ch, thetall,         weight);
	    controlHistos.fillHisto(ctrlCategs[icat]+"met"+var,          ch, met.pt(),        weight);
	    controlHistos.fillHisto(ctrlCategs[icat]+"metnotoppt"+var,   ch, met.pt(),        weight/wgtTopPt);
	    controlHistos.fillHisto(ctrlCategs[icat]+"njets"+var,        ch, selJets.size(),  weight);
	    controlHistos.fillHisto(ctrlCategs[icat]+"njetsnotoppt"+var, ch, selJets.size(),  weight/wgtTopPt);
	    controlHistos.fillHisto(ctrlCategs[icat]+"nbjets"+var,       ch, selbJets.size(), weight);
	    controlHistos.fillHisto(ctrlCategs[icat]+"ht"+var,           ch, ht,              weight);
	    controlHistos.fillHisto(ctrlCategs[icat]+"htb"+var,          ch, htb,             weight);
	    controlHistos.fillHisto(ctrlCategs[icat]+"htnol"+var,        ch, htnol,           weight);
	    controlHistos.fillHisto(ctrlCategs[icat]+"htbnol"+var,       ch, htbnol,          weight);
	    
	    for(size_t ijet=0; ijet<looseJets.size(); ijet++)
	      {
		if(looseJets[ijet].pt()<30 || fabs(looseJets[ijet].eta())>2.5) continue;
		TString label("jet"); label+=(ijet+1);
		//		if(ijet+1 >=3) continue; // unnecessary histos at the moment 
		const data::PhysicsObject_t &genJet=looseJets[ijet].getObject("genJet");
		int flavId=genJet.info.find("id")->second;
		if(abs(flavId)==5 || abs(flavId)==4 ) flavId=abs(flavId)-1;
		else if(abs(flavId)>6)                flavId=1;
		else if(abs(flavId)==0)               flavId=0;
		else                                  flavId=2;
		controlHistos.fillHisto(ctrlCategs[icat]+"pt"+label+"pt"+var,        ch, looseJets[ijet].pt(), weight);
		controlHistos.fillHisto(ctrlCategs[icat]+"pt"+label+"eta"+var,       ch, fabs(looseJets[ijet].eta()), weight);
		controlHistos.fillHisto(ctrlCategs[icat]+"pt"+label+"flav"+var,      ch, abs(flavId), weight);
		controlHistos.fillHisto(ctrlCategs[icat]+"pt"+label+"nobsmearpt"+var,ch, abs(flavId)==5 ? looseJets[ijet].pt() : looseJets[ijet].getVal("jer"), weight);
		controlHistos.fillHisto(ctrlCategs[icat]+"pt"+label+"smearpt"+var,   ch,                                         looseJets[ijet].getVal("jer"), weight);
	      }
	    
	    for(size_t ijet=0; ijet<selJets.size(); ijet++)
	      {
		TString label("jet"); label+=(ijet+1);
		//		if(ijet+1 >=3) continue; // unnecessary histos at the moment 
		const data::PhysicsObject_t &genJet=selJets[ijet].getObject("genJet");
		int flavId=genJet.info.find("id")->second;
		if(abs(flavId)==5 || abs(flavId)==4 ) flavId=abs(flavId)-1;
		else if(abs(flavId)>6)                flavId=1;
		else if(abs(flavId)==0)               flavId=0;
		else                                  flavId=2;
		controlHistos.fillHisto(ctrlCategs[icat]+"btag"+label+"pt"+var,        ch, selJets[ijet].pt(), weight,true);
		controlHistos.fillHisto(ctrlCategs[icat]+"btag"+label+"ptnotoppt"+var, ch, selJets[ijet].pt(), weight/wgtTopPt,true);
		controlHistos.fillHisto(ctrlCategs[icat]+"btag"+label+"eta"+var,       ch, fabs(selJets[ijet].eta()), weight);
		controlHistos.fillHisto(ctrlCategs[icat]+"btag"+label+"flav"+var,      ch, abs(flavId), weight);
		controlHistos.fillHisto(ctrlCategs[icat]+"btag"+label+"nobsmearpt"+var,ch, abs(flavId)==5 ? selJets[ijet].pt() : selJets[ijet].getVal("jer"), weight);
		controlHistos.fillHisto(ctrlCategs[icat]+"btag"+label+"smearpt"+var,   ch,                                       selJets[ijet].getVal("jer"), weight);
	      }
	  }
	
	//      if(passDilSelection &&                     passMetSelection && isOS) btvAn.analyze(selLeptons,looseJets,isMC,ev.nvtx,weight,weightUp,weightDown);
	//	if(passDilSelection && passJetSelection &&                     isOS) lxyAn.analyze(selLeptons,selJets,met[0],gen,weight);
	
	//select the event
	if(!passJetSelection) continue;
	controlHistos.fillHisto("evtflow"+var, ch, 1, weight);
	
	if(!passBtagSelection) continue;
	controlHistos.fillHisto("evtflow"+var, ch, 2, weight);
	
	if(!isWcand) continue;
	controlHistos.fillHisto("evtflow"+var, ch, 3, weight);
	
	if(!isTopCand) continue;
	controlHistos.fillHisto("evtflow"+var, ch, 4, weight);

	if(!passMetSelection) continue;
	controlHistos.fillHisto("evtflow"+var, ch, 5, weight);
	
//	//run the lxy analysis
//	//	lxyAn.analyze(selLeptons,selJets,met[0],gen,weight);
//	
//	//      float nbtags(0);
//	//	for(size_t ijet=0; ijet<selJets.size(); ijet++) nbtags += (selJets[ijet].getVal("csv")>0.405); // CSVV1L
//	float nbtags(selbJets.size());
//	//	if(nbtags>0){
//	if(nbtags>5)
//	  controlHistos.fillHisto("finalevtflow"+var, ch, 5, weight);
//	else
//	  controlHistos.fillHisto("finalevtflow"+var, ch, nbtags, weight);
//	  
//	  //}
//	
//	if(nbtags==0) controlHistos.fillHisto("finalevtflow0"+var, ch, 0, weight);
//	if(nbtags==1) controlHistos.fillHisto("finalevtflow1"+var, ch, 0, weight);
//	if(nbtags==2) controlHistos.fillHisto("finalevtflow2"+var, ch, 0, weight);
//	if(nbtags==3) controlHistos.fillHisto("finalevtflow3"+var, ch, 0, weight);
//	if(nbtags==4) controlHistos.fillHisto("finalevtflow4"+var, ch, 0, weight);
//	if(nbtags>=5) controlHistos.fillHisto("finalevtflow5"+var, ch, 0, weight);
//	
//	if(nbtags<2) continue;
//	controlHistos.fillHisto("evtflow"+var, ch, 5, weight);
//	
//	if(nbtags>5)
//	  controlHistos.fillHisto("finalevtflow2btags"+var, ch, 5, weight);	
//	else
//	  controlHistos.fillHisto("finalevtflow2btags"+var, ch, nbtags, weight);
//
	if(spyEvents){
	  spyEvents->getEvent().cat=ev.cat;
	  evSummaryWeight=xsecWeight*weight;
	  spyEvents->getTree()->Fill();
	}
	
	//PF candidates
	//data::PhysicsObjectCollection_t pf = evSummary.getPhysicsObject(DataEventSummaryHandler::PFCANDIDATES);
	//ueAn.analyze(selLeptons,selJets,met,pf,gen,weight);
      }
    }
  
  if(nDuplicates) cout << "[Warning] found " << nDuplicates << " duplicate events in this ntuple" << endl;
  
  //
  // close opened files
  // 
  inF->Close();
  if(spyFile){
    spyDir->cd(); spyEvents->getTree()->Write();
    spyFile->Close();
  }
  
  
  //
  // save histos to local file
  //
  TString outUrl(out);
  gSystem->ExpandPathName(outUrl);
  gSystem->Exec("mkdir -p " + outUrl);
  outUrl += "/";
  outUrl += proctag;
  if(mcTruthMode!=0) { outUrl += "_filt"; outUrl += mcTruthMode; }
  outUrl += ".root";
  TFile *file=TFile::Open(outUrl, "recreate");
  controlHistos.Write();
  file->Close();
  
  //that's all folks!
}  
