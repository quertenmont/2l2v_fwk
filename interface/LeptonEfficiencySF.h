#ifndef LeptonEfficiencySF_h
#define LeptonEfficiencySF_h

// cf.
// https://twiki.cern.ch/twiki/bin/view/Main/EGammaScaleFactors2012#2012_8_TeV_data_53X
//
class LeptonEfficiencySF
{
 public:

  //
  LeptonEfficiencySF() { }

  //
  ~LeptonEfficiencySF() {}

  //
  std::pair<float,float> getLeptonEfficiency(float pt, float eta, int id, std::string wp)
    {
      eta=fabs(eta);
      id=abs(id);
      
      std::pair<float,float> eff(1.0,0.04);
      switch(id){
      case 11:
	{
	  if(wp=="loose")
	    {
		if( eta >= -2.5 && eta < -2.0 ){
		       if( pt < 30 ){ eff.first=0.98563; eff.second=0.0883625; }
		       else if( pt < 40 ){ eff.first=1.00842; eff.second=0.0648338; }
		       else if( pt < 50 ){ eff.first=0.990451; eff.second=0.0688289; }
		       else { eff.first=0.949225; eff.second=0.11184; }
		}
		else if( eta >= -2 && eta < -1.4 ){
		       if( pt < 30 ){ eff.first=0.937672; eff.second=0.0699228; }
		       else if( pt < 40 ){ eff.first=0.982285; eff.second=0.051957; }
		       else if( pt < 50 ){ eff.first=0.972619; eff.second=0.0551727; }
		       else { eff.first=0.960835; eff.second=0.0956784; }
		}
		else if( eta >= -1.4 && eta < -0.8 ){
		       if( pt < 30 ){ eff.first=0.970037; eff.second=0.0643857; }
		       else if( pt < 40 ){ eff.first=0.988901; eff.second=0.0434648; }
		       else if( pt < 50 ){ eff.first=0.98464; eff.second=0.0403634; }
		       else { eff.first=0.988509; eff.second=0.0693141; }              
		}
		else if( eta >= -0.8 && eta < 0 ){
		       if( pt < 30 ){ eff.first=0.972285; eff.second=0.051734; }     
		       else if( pt < 40 ){ eff.first=0.987778; eff.second=0.0333322; }
		       else if( pt < 50 ){ eff.first=0.989219; eff.second=0.0326959; }
		       else { eff.first=0.980778; eff.second=0.053409; }              
		}
		else if( eta >= 0 && eta < 0.8 ){
		       if( pt < 30 ){ eff.first=0.972702; eff.second=0.0514579; }     
		       else if( pt < 40 ){ eff.first=0.982472; eff.second=0.0328293; }
		       else if( pt < 50 ){ eff.first=0.979501; eff.second=0.0318654; }
		       else { eff.first=0.981492; eff.second=0.0528332; }              
		}
		else if( eta >= 0.8 && eta < 1.4 ){
		       if( pt < 30 ){ eff.first=0.977918; eff.second=0.0647224; }
		       else if( pt < 40 ){ eff.first=0.979425; eff.second=0.0413102; }
		       else if( pt < 50 ){ eff.first=0.994031; eff.second=0.0394454; }
		       else { eff.first=0.981925; eff.second=0.0655805; }
		}
		else if( eta >= 1.4 && eta < 2.0 ){
		       if( pt < 30 ){ eff.first=0.95262; eff.second=0.0708203; }
		       else if( pt < 40 ){ eff.first=0.979062; eff.second=0.0511177; }
		       else if( pt < 50 ){ eff.first=0.998381; eff.second=0.0543951; }
		       else { eff.first=1.00527; eff.second=0.088789; }
		}
		else {
 		      if( pt < 30 ){ eff.first=1.00213; eff.second=0.0791871; }
		       else if( pt < 40 ){ eff.first=0.987968; eff.second=0.0583879; }
		       else if( pt < 50 ){ eff.first=1.01177; eff.second=0.0645323; }
		       else { eff.first=0.983595; eff.second=0.105065; }
		}
	      }
	  if(wp=="medium")
	      {
		if( eta >= -2.5 && eta < -2.0 ){
		       if( pt < 30 ){ eff.first=0.874245; eff.second=0.0830587; }
		       else if( pt < 40 ){ eff.first=0.932571; eff.second=0.0622322; }
		       else if( pt < 50 ){ eff.first=0.950056; eff.second=0.0675432; }
		       else{ eff.first=0.859676; eff.second=0.105093; }
		}
		else if( eta >= -2.0 && eta < -1.4 ){
		       if( pt < 30 ){ eff.first=0.882252; eff.second=0.0691932; }
		       else if( pt < 40 ){ eff.first=0.947279; eff.second=0.0515772; }
		       else if( pt < 50 ){ eff.first=0.935612; eff.second=0.0542408; }
		       else { eff.first=0.923622; eff.second=0.093825; }
		}
		else if( eta >= -1.4 && eta < -0.8 ){
		       if( pt < 30 ){ eff.first=0.973758; eff.second=0.0667138; }
		       else if( pt < 40 ){ eff.first=0.953688; eff.second=0.0432537; }
		       else if( pt < 50 ){ eff.first=0.958874; eff.second=0.0402327; }
		       else { eff.first=0.953866; eff.second=0.0683705; }              
		}
		else if( eta >= -0.8 && eta < 0 ){
		       if( pt < 30 ){ eff.first=0.966344; eff.second=0.053096; }     
		       else if( pt < 40 ){ eff.first=0.976407; eff.second=0.0337626; }
		       else if( pt < 50 ){ eff.first=0.977874; eff.second=0.0329249; }
		       else { eff.first=0.988572; eff.second=0.054376; }              
		}
		else if( eta >= 0 && eta < 0.8 ){
		       if( pt < 30 ){ eff.first=0.963403; eff.second=0.0526731; }     
		       else if( pt < 40 ){ eff.first=0.965189; eff.second=0.0330997; }
		       else if( pt < 50 ){ eff.first=0.969854; eff.second=0.0321271; }
		       else { eff.first=0.974913; eff.second=0.0532595; }              
		}
		else if( eta >= 0 && eta < 0.8 ){
		       if( pt < 30 ){ eff.first=0.963403; eff.second=0.0526731; }     
		       else if( pt < 40 ){ eff.first=0.965189; eff.second=0.0330997; }
		       else if( pt < 50 ){ eff.first=0.969854; eff.second=0.0321271; }
		       else { eff.first=0.974913; eff.second=0.0532595; }              
		}
		else if( eta >= 0.8 && eta < 1.4 ){
		       if( pt < 30 ){ eff.first=0.971722; eff.second=0.0665727; }
		       else if( pt < 40 ){ eff.first=0.94177; eff.second=0.0410411; }
		       else if( pt < 50 ){ eff.first=0.966271; eff.second=0.0392817; }
		       else { eff.first=0.966591; eff.second=0.0656947; }
		}
		else if( eta >= 1.4 && eta < 2.0 ){
		      if( pt < 30 ){ eff.first=0.945345; eff.second=0.0726882; }
		       else if( pt < 40 ){ eff.first=0.963352; eff.second=0.0514941; }
		       else if( pt < 50 ){ eff.first=0.979092; eff.second=0.0542603; }
		       else { eff.first=0.952561; eff.second=0.0859961; }
		}
		else if( eta >= 2.0 && eta < 2.5 ){
		       if( pt < 30 ){ eff.first=0.994598; eff.second=0.0809519; }
		       else if( pt < 40 ){ eff.first=0.965872; eff.second=0.0583806; }
		       else if( pt < 50 ){ eff.first=0.987706; eff.second=0.0641467; }
  		       else { eff.first=0.955117; eff.second=0.10377; }
		}
	      }
	  if(wp=="tight")
	      {
		if( eta >= -2.5 && eta < -2.0 ){
		       if( pt < 30 ){ eff.first=0.84725; eff.second=0.0841521; }
		       else if( pt < 40 ){ eff.first=0.896803; eff.second=0.0617527; }
		       else if( pt < 50 ){ eff.first=0.934238; eff.second=0.0676831; }
		       else { eff.first=0.814249; eff.second=0.102239; }
		}
		else if( eta >= -2.0 && eta < -1.4 ){
		       if( pt < 30 ){ eff.first=0.81451; eff.second=0.0689909; }
		       else if( pt < 40 ){ eff.first=0.918068; eff.second=0.0520854; }
		       else if( pt < 50 ){ eff.first=0.895098; eff.second=0.0536899; }
		       else { eff.first=0.888402; eff.second=0.0924434; }
		}
		else if( eta >= -1.4 && eta < -0.8 ){
		       if( pt < 30 ){ eff.first=0.936833; eff.second=0.0676447; }
		       else if( pt < 40 ){ eff.first=0.913318; eff.second=0.0432068; }
		       else if( pt < 50 ){ eff.first=0.92881; eff.second=0.0401974; }
		       else { eff.first=0.91562; eff.second=0.0674593; }             
		}
		else if( eta >= -0.8 && eta < 0 ){
		       if( pt < 30 ){ eff.first=0.934999; eff.second=0.052994; }    
		       else if( pt < 40 ){ eff.first=0.959169; eff.second=0.0338947; }
		       else if( pt < 50 ){ eff.first=0.970003; eff.second=0.0331906; }
		       else { eff.first=0.96905; eff.second=0.0541575; }             
		}
		else if( eta >= 0 && eta < 0.8 ){
		       if( pt < 30 ){ eff.first=0.964619; eff.second=0.0538283; }    
		       else if( pt < 40 ){ eff.first=0.955241; eff.second=0.0334249; }
		       else if( pt < 50 ){ eff.first=0.970802; eff.second=0.0326056; }
		       else { eff.first=0.94757; eff.second=0.052687; }             
		}
		else if( eta >= 0.8 && eta < 1.4 ){
		       if( pt < 30 ){ eff.first=0.957247; eff.second=0.0686992; }
	               else if( pt < 40 ){ eff.first=0.894828; eff.second=0.0407361; }
		       else if( pt < 50 ){ eff.first=0.937758; eff.second=0.0393072; }
		       else { eff.first=0.905154; eff.second=0.0635497; }
		}
		else if( eta >= 1.4 && eta < 2.0 ){
		       if( pt < 30 ){ eff.first=0.923384; eff.second=0.0752278; }
		       else if( pt < 40 ){ eff.first=0.924472; eff.second=0.0515969; }
		       else if( pt < 50 ){ eff.first=0.948364; eff.second=0.0540815; }
		       else { eff.first=0.909257; eff.second=0.0843905; }
		}
		else if( eta >= 2.0 && eta < 2.5 ){
		       if( pt < 30 ){ eff.first=0.994083; eff.second=0.0838027; }
		       else if( pt < 40 ){ eff.first=0.967218; eff.second=0.0596714; }
		       else if( pt < 50 ){ eff.first=0.962953; eff.second=0.0638915; }
		       else { eff.first=0.946442; eff.second=0.104233; }
		}

	      }
	}
	break;
      case 13:
	{
	  if(wp=="loose")
	    {
		if( eta >= -2.4 && eta < -2.1 ){
		       if( pt < 25 ){ eff.first=1.07537; eff.second=0.149881; }
		       else if( pt < 30 ){ eff.first=1.10263; eff.second=0.125934; }
		       else if( pt < 35 ){ eff.first=1.05168; eff.second=0.105491; }
		       else if( pt < 40 ){ eff.first=0.999611; eff.second=0.0883756; }
		       else if( pt < 50 ){ eff.first=1.02331; eff.second=0.070486; }
		       else if( pt < 60 ){ eff.first=1.01414; eff.second=0.121958; }
		       else if( pt < 90 ){ eff.first=1.01671; eff.second=0.188573; }
		       else if( pt < 140 ){ eff.first=1.01783; eff.second=0.446804; }
		       else { eff.first=0; eff.second=0; }
		}
		else if( eta >= -2.1 && eta < -1.2 ){
		       if( pt < 25 ){ eff.first=1.01136; eff.second=0.0803326; }
		       else if( pt < 30 ){ eff.first=1.04675; eff.second=0.0645099; }
		       else if( pt < 35 ){ eff.first=1.01289; eff.second=0.0540217; }
		       else if( pt < 40 ){ eff.first=1.01163; eff.second=0.0461871; }
		       else if( pt < 50 ){ eff.first=1.01301; eff.second=0.0301994; }
		       else if( pt < 60 ){ eff.first=1.00738; eff.second=0.0621656; }
		       else if( pt < 90 ){ eff.first=0.999336; eff.second=0.0893752; }
		       else if( pt < 140 ){ eff.first=0.952042; eff.second=0.208969; }
		       else { eff.first=0.938528; eff.second=0.444387; }
		}
		else if( eta >= -1.2 && eta < -0.9 ){
		       if( pt < 25 ){ eff.first=0.923816; eff.second=0.115792; }
		       else if( pt < 30 ){ eff.first=0.98393; eff.second=0.110994; }
		       else if( pt < 35 ){ eff.first=1.03724; eff.second=0.0871372; }
 	               else if( pt < 40 ){ eff.first=1.00385; eff.second=0.0662519; }
		       else if( pt < 50 ){ eff.first=1.01086; eff.second=0.0472075; }
		       else if( pt < 60 ){ eff.first=1.00729; eff.second=0.0979038; }
		       else if( pt < 90 ){ eff.first=1.00966; eff.second=0.147356; }
		       else if( pt < 140 ){ eff.first=1.02896; eff.second=0.306172; }
		       else { eff.first=1.06023; eff.second=1.08046; }
		}
		else if( eta >= -0.9 && eta < 0 ){
		       if( pt < 25 ){ eff.first=1.0023; eff.second=0.072312; }
		       else if( pt < 30 ){ eff.first=1.04451; eff.second=0.0536374; }
		       else if( pt < 35 ){ eff.first=1.01985; eff.second=0.0419339; }
		       else if( pt < 40 ){ eff.first=1.00915; eff.second=0.0359674; }
		       else if( pt < 50 ){ eff.first=1.01467; eff.second=0.0264719; }
		       else if( pt < 60 ){ eff.first=1.00968; eff.second=0.0530667; }
		       else if( pt < 90 ){ eff.first=0.987115; eff.second=0.072688; }
		       else if( pt < 140 ){ eff.first=1.00135; eff.second=0.195789; }
		       else { eff.first=1.02367; eff.second=0.491135; }
		}
		else if( eta >= 0 && eta < 0.9 ){
		       if( pt < 25 ){ eff.first=0.984414; eff.second=0.0696114; }
		       else if( pt < 30 ){ eff.first=1.00743; eff.second=0.0525317; }
		       else if( pt < 35 ){ eff.first=1.02249; eff.second=0.0417056; }
		       else if( pt < 40 ){ eff.first=1.02562; eff.second=0.0360638; }
		       else if( pt < 50 ){ eff.first=1.01486; eff.second=0.0265924; }
		       else if( pt < 60 ){ eff.first=1.00673; eff.second=0.0557899; }
		       else if( pt < 90 ){ eff.first=1.00672; eff.second=0.0778862; }
		       else if( pt < 140 ){ eff.first=1.00581; eff.second=0.188603; }
		       else { eff.first=1.02003; eff.second=0.370385; }
		}
		else if( eta >= 0.9 && eta < 1.2 ){
		       if( pt < 25 ){ eff.first=0.983886; eff.second=0.129608; }
		       else if( pt < 30 ){ eff.first=0.957383; eff.second=0.0986641; }
		       else if( pt < 35 ){ eff.first=1.02346; eff.second=0.0813516; }
		       else if( pt < 40 ){ eff.first=1.0198; eff.second=0.0694007; }
		       else if( pt < 50 ){ eff.first=1.01047; eff.second=0.0476384; }
		       else if( pt < 60 ){ eff.first=0.988696; eff.second=0.093559; }
		       else if( pt < 90 ){ eff.first=0.990878; eff.second=0.131086; }
		       else if( pt < 140 ){ eff.first=1.02471; eff.second=0.553011; }
		       else { eff.first=0.902957; eff.second=0.461321; }
		}
		else if( eta >= 1.2 && eta < 2.1 ){
		       if( pt < 25 ){ eff.first=1.04612; eff.second=0.079847; }
		       else if( pt < 30 ){ eff.first=1.04092; eff.second=0.0629323; }
		       else if( pt < 35 ){ eff.first=1.03206; eff.second=0.0551064; }
		       else if( pt < 40 ){ eff.first=1.00742; eff.second=0.0462022; }
		       else if( pt < 50 ){ eff.first=1.00635; eff.second=0.0304797; }
		       else if( pt < 60 ){ eff.first=1.00363; eff.second=0.0606111; }
		       else if( pt < 90 ){ eff.first=0.996578; eff.second=0.0890957; }
		       else if( pt < 140 ){ eff.first=1.02065; eff.second=0.209138; }
		       else { eff.first=1.02527; eff.second=0.492839; }
		}
		else if( eta >= 2.1 && eta < 2.4 ){
		       if( pt < 25 ){ eff.first=1.10645; eff.second=0.157415; }
		       else if( pt < 30 ){ eff.first=1.05957; eff.second=0.123243; }
		       else if( pt < 35 ){ eff.first=1.06936; eff.second=0.114918; }
		       else if( pt < 40 ){ eff.first=1.02847; eff.second=0.0920642; }
		       else if( pt < 50 ){ eff.first=1.01087; eff.second=0.0691265; }
		       else if( pt < 60 ){ eff.first=1.00054; eff.second=0.138117; }
		       else if( pt < 90 ){ eff.first=1.00023; eff.second=0.206921; }
		       else if( pt < 140 ){ eff.first=1.01806; eff.second=0.471105; }
		       else { eff.first=1.08238; eff.second=1.60232; }
		}
	    }

          if(wp=="medium")
           {
		if( eta >= -2.4 && eta < -2.1 ){
		       if( pt < 25 ){ eff.first=1.08298; eff.second=0.15095; }
		       else if( pt < 30 ){ eff.first=1.10716; eff.second=0.126454; }
		       else if( pt < 35 ){ eff.first=1.0481; eff.second=0.105411; }
		       else if( pt < 40 ){ eff.first=0.999304; eff.second=0.0885352; }
		       else if( pt < 50 ){ eff.first=1.02713; eff.second=0.0708361; }
		       else if( pt < 60 ){ eff.first=1.00476; eff.second=0.12126; }
		       else if( pt < 90 ){ eff.first=0.987406; eff.second=0.184666; }
		       else if( pt < 140 ){ eff.first=1.02224; eff.second=0.448763; }
		       else { eff.first=0.941546; eff.second=0.445845; }
		}
		else if( eta >= -2.1 && eta < -1.2 ){
		       if( pt < 25 ){ eff.first=1.00904; eff.second=0.0802196; }
		       else if( pt < 30 ){ eff.first=1.04652; eff.second=0.0645287; }
		       else if( pt < 35 ){ eff.first=1.00841; eff.second=0.0538634; }
		       else if( pt < 40 ){ eff.first=1.0105; eff.second=0.0461732; }
		       else if( pt < 50 ){ eff.first=1.01014; eff.second=0.0301468; }
		       else if( pt < 60 ){ eff.first=1.00744; eff.second=0.062199; }
		       else if( pt < 90 ){ eff.first=1.00145; eff.second=0.0895662; }
		       else if( pt < 140 ){ eff.first=0.93112; eff.second=0.20561; }
		       else { eff.first=0.941546; eff.second=0.445845; }
		}
		else if( eta >= -1.2 && eta < -0.9 ){
		       if( pt < 25 ){ eff.first=0.920946; eff.second=0.115729; }
		       else if( pt < 30 ){ eff.first=0.982211; eff.second=0.111009; }
		       else if( pt < 35 ){ eff.first=1.03501; eff.second=0.0871139; }
		       else if( pt < 40 ){ eff.first=1.00236; eff.second=0.0662292; }
		       else if( pt < 50 ){ eff.first=1.01017; eff.second=0.0472268; }
		       else if( pt < 60 ){ eff.first=0.992983; eff.second=0.096966; }
		       else if( pt < 90 ){ eff.first=1.00259; eff.second=0.146704; }
		       else if( pt < 140 ){ eff.first=1.03683; eff.second=0.308563; }
		       else { eff.first=1.05108; eff.second=1.07087; }
		}
		else if( eta >= -0.9 && eta < 0 ){
		       if( pt < 25 ){ eff.first=1.00123; eff.second=0.0723533; }
		       else if( pt < 30 ){ eff.first=1.03759; eff.second=0.0534621; }
		       else if( pt < 35 ){ eff.first=1.01621; eff.second=0.0418813; }
		       else if( pt < 40 ){ eff.first=1.00603; eff.second=0.0359411; }
		       else if( pt < 50 ){ eff.first=1.01387; eff.second=0.0264969; }
		       else if( pt < 60 ){ eff.first=1.00986; eff.second=0.0531497; }
		       else if( pt < 90 ){ eff.first=0.990358; eff.second=0.0729794; }
		       else if( pt < 140 ){ eff.first=1.0099; eff.second=0.197484; }
		       else { eff.first=1.02945; eff.second=0.493926; }
		}
		else if( eta >= 0 && eta < 0.9 ){
		       if( pt < 25 ){ eff.first=0.987126; eff.second=0.069861; }
		       else if( pt < 30 ){ eff.first=1.00595; eff.second=0.0525605; }
		       else if( pt < 35 ){ eff.first=1.02178; eff.second=0.0417531; }
		       else if( pt < 40 ){ eff.first=1.02817; eff.second=0.0361833; }
		       else if( pt < 50 ){ eff.first=1.01248; eff.second=0.0265856; }
		       else if( pt < 60 ){ eff.first=1.00531; eff.second=0.0558186; }
		       else if( pt < 90 ){ eff.first=1.00326; eff.second=0.0777884; }
		       else if( pt < 140 ){ eff.first=0.975708; eff.second=0.184504; }
		       else { eff.first=1.02823; eff.second=0.37341; }
		}
		else if( eta >= 0.9 && eta < 1.2 ){
		       if( pt < 25 ){ eff.first=0.979248; eff.second=0.129341; }
		       else if( pt < 30 ){ eff.first=0.961153; eff.second=0.0990553; }
		       else if( pt < 35 ){ eff.first=1.02893; eff.second=0.0817905; }
		       else if( pt < 40 ){ eff.first=1.01734; eff.second=0.0693589; }
		       else if( pt < 50 ){ eff.first=1.01152; eff.second=0.0477283; }
		       else if( pt < 60 ){ eff.first=0.988422; eff.second=0.0936392; }
		       else if( pt < 90 ){ eff.first=0.996439; eff.second=0.13183; }
		       else if( pt < 140 ){ eff.first=1.02781; eff.second=0.554689; }
		       else { eff.first=0.916951; eff.second=0.468697; }
		}
		else if( eta >= 1.2 && eta < 2.1 ){
		       if( pt < 25 ){ eff.first=1.04801; eff.second=0.0799921; }
		       else if( pt < 30 ){ eff.first=1.04101; eff.second=0.0629687; }
		       else if( pt < 35 ){ eff.first=1.03076; eff.second=0.0550783; }
		       else if( pt < 40 ){ eff.first=1.00729; eff.second=0.0462214; }
		       else if( pt < 50 ){ eff.first=1.00392; eff.second=0.0304375; }
		       else if( pt < 60 ){ eff.first=1.00214; eff.second=0.0605757; }
		       else if( pt < 90 ){ eff.first=0.994351; eff.second=0.0889824; }
		       else if( pt < 140 ){ eff.first=1.02383; eff.second=0.209802; }
		       else { eff.first=1.02951; eff.second=0.494905; }
		}
		else if( eta >= 2.1 && eta < 2.4 ){
		       if( pt < 25 ){ eff.first=1.08578; eff.second=0.155354; }
		       else if( pt < 30 ){ eff.first=1.05746; eff.second=0.123229; }
		       else if( pt < 35 ){ eff.first=1.06354; eff.second=0.114639; }
		       else if( pt < 40 ){ eff.first=1.03235; eff.second=0.0925116; }
		       else if( pt < 50 ){ eff.first=1.01344; eff.second=0.0693856; }
		       else if( pt < 60 ){ eff.first=0.997983; eff.second=0.138095; }
		       else if( pt < 90 ){ eff.first=0.98747; eff.second=0.205356; }
		       else if( pt < 140 ){ eff.first=1.02459; eff.second=0.474202; }
		       else { eff.first=1.02951; eff.second=0.494905; }
		}
           }
	  if(wp=="tight")
           {
		if( eta >= -2.4 && eta < -2.1 ){
		       if( pt < 25 ){ eff.first=1.05896; eff.second=0.148858; }
		       else if( pt < 30 ){ eff.first=1.07145; eff.second=0.123662; }
		       else if( pt < 35 ){ eff.first=1.03724; eff.second=0.104884; }
		       else if( pt < 40 ){ eff.first=0.98658; eff.second=0.0879698; }
		       else if( pt < 50 ){ eff.first=1.01285; eff.second=0.0702757; }
		       else if( pt < 60 ){ eff.first=1.01878; eff.second=0.123193; }
		       else if( pt < 90 ){ eff.first=0.988831; eff.second=0.18576; }
		       else if( pt < 140 ){ eff.first=0.974866; eff.second=0.438375; }
		       else { eff.first=0.946542; eff.second=0.448165; }
		}
		else if( eta >= -2.1 && eta < -1.2 ){
		       if( pt < 25 ){ eff.first=0.993193; eff.second=0.0793924; }
		       else if( pt < 30 ){ eff.first=1.04212; eff.second=0.0643879; }
		       else if( pt < 35 ){ eff.first=1.00783; eff.second=0.0539157; }
		       else if( pt < 40 ){ eff.first=1.01329; eff.second=0.0463266; }
		       else if( pt < 50 ){ eff.first=1.00923; eff.second=0.0301668; }
		       else if( pt < 60 ){ eff.first=1.00037; eff.second=0.0619657; }
		       else if( pt < 90 ){ eff.first=0.993295; eff.second=0.0891842; }
		       else if( pt < 140 ){ eff.first=0.961159; eff.second=0.210982; }
		       else { eff.first=0.946542; eff.second=0.448165; }
		}
		else if( eta >= -1.2 && eta < -0.9 ){
		       if( pt < 25 ){ eff.first=0.919173; eff.second=0.115805; }
		       else if( pt < 30 ){ eff.first=0.978114; eff.second=0.110969; }
		       else if( pt < 35 ){ eff.first=1.02581; eff.second=0.0867546; }
		       else if( pt < 40 ){ eff.first=0.999135; eff.second=0.0662461; }
		       else if( pt < 50 ){ eff.first=1.00088; eff.second=0.0470278; }
		       else if( pt < 60 ){ eff.first=0.988822; eff.second=0.0969112; }
		       else if( pt < 90 ){ eff.first=1.00374; eff.second=0.147269; }
		       else if( pt < 140 ){ eff.first=1.05584; eff.second=0.314327; }
		       else { eff.first=0.553107; eff.second=0.686326; }
		}
		else if( eta >= -0.9 && eta < 0 ){
		       if( pt < 25 ){ eff.first=1.00486; eff.second=0.0727995; }
		       else if( pt < 30 ){ eff.first=1.03504; eff.second=0.0535578; }
		       else if( pt < 35 ){ eff.first=1.01851; eff.second=0.0421166; }
		       else if( pt < 40 ){ eff.first=1.01314; eff.second=0.0362845; }
		       else if( pt < 50 ){ eff.first=1.02215; eff.second=0.026776; }
		       else if( pt < 60 ){ eff.first=1.01459; eff.second=0.0535502; }
		       else if( pt < 90 ){ eff.first=0.991003; eff.second=0.0732794; }
		       else if( pt < 140 ){ eff.first=1.00506; eff.second=0.197458; }
		       else { eff.first=1.05133; eff.second=0.504524; }
		}
		else if( eta >= 0 && eta < 0.9 ){
		       if( pt < 25 ){ eff.first=0.992418; eff.second=0.0704088; }
		       else if( pt < 30 ){ eff.first=1.00792; eff.second=0.0528591; }
		       else if( pt < 35 ){ eff.first=1.02459; eff.second=0.042006; }
		       else if( pt < 40 ){ eff.first=1.03041; eff.second=0.0363941; }
		       else if( pt < 50 ){ eff.first=1.01776; eff.second=0.0268112; }
		       else if( pt < 60 ){ eff.first=1.0152; eff.second=0.0564591; }
		       else if( pt < 90 ){ eff.first=1.00011; eff.second=0.0778948; }
		       else if( pt < 140 ){ eff.first=0.967242; eff.second=0.183709; }
		       else { eff.first=1.04032; eff.second=0.377802; }
		}
		else if( eta >= 0.9 && eta < 1.2 ){
		       if( pt < 25 ){ eff.first=0.977766; eff.second=0.129498; }
		       else if( pt < 30 ){ eff.first=0.951332; eff.second=0.0985158; }
		       else if( pt < 35 ){ eff.first=1.02189; eff.second=0.0815794; }
		       else if( pt < 40 ){ eff.first=0.990319; eff.second=0.0681869; }
		       else if( pt < 50 ){ eff.first=0.994383; eff.second=0.0472548; }
		       else if( pt < 60 ){ eff.first=0.958992; eff.second=0.0917997; }
		       else if( pt < 90 ){ eff.first=0.99744; eff.second=0.132249; }
		       else if( pt < 140 ){ eff.first=1.04877; eff.second=0.566072; }
		       else { eff.first=0.805019; eff.second=0.425193; }
		}
		else if( eta >= 1.2 && eta < 2.1 ){
		       if( pt < 25 ){ eff.first=1.02934; eff.second=0.0790198; }
		       else if( pt < 30 ){ eff.first=1.04291; eff.second=0.0631162; }
		       else if( pt < 35 ){ eff.first=1.02908; eff.second=0.055071; }
		       else if( pt < 40 ){ eff.first=1.00246; eff.second=0.0461131; }
		       else if( pt < 50 ){ eff.first=1.00239; eff.second=0.0304474; }
		       else if( pt < 60 ){ eff.first=0.989368; eff.second=0.060103; }
		       else if( pt < 90 ){ eff.first=0.978865; eff.second=0.0881146; }
		       else if( pt < 140 ){ eff.first=0.953392; eff.second=0.199404; }
		       else { eff.first=0.926652; eff.second=0.458689; }
		}
		else if( eta >= 2.1 && eta < 2.4 ){
		       if( pt < 25 ){ eff.first=1.083; eff.second=0.155419; }
		       else if( pt < 30 ){ eff.first=1.05835; eff.second=0.123567; }
		       else if( pt < 35 ){ eff.first=1.08342; eff.second=0.116617; }
		       else if( pt < 40 ){ eff.first=1.01698; eff.second=0.0917138; }
		       else if( pt < 50 ){ eff.first=0.997086; eff.second=0.0687643; }
		       else if( pt < 60 ){ eff.first=0.974719; eff.second=0.136208; }
		       else if( pt < 90 ){ eff.first=0.879815; eff.second=0.189473; }
		       else if( pt < 140 ){ eff.first=0.955354; eff.second=0.453776; }
		       else { eff.first=1.10915; eff.second=1.64255; }
		}
	   }
	}
	break;
      }
      
      return eff;
    }

 private:

};


#endif
