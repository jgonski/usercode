
//#Includetr "RUAnalysis/Ntupler/interface/Ntupler.h"
#include "RUAnalysis/Analysis/interface/NtpThreeJet.h"
#include "RUAnalysis/Analysis/interface/EventShapeVariables.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <cmath>
#include <algorithm>
#include "TLorentzVector.h"
#include "TFile.h"
#include "math.h"
#include "TMath.h"
#include "TROOT.h"
#include "TStyle.h"
#include "Math/VectorUtil.h"
#include "Math/Boost.h"

#include "Math/GenVector/Boost.h"
#include "TVector3.h"
//#include "../interface/BTagSFUtil.h"
#include "../interface/BTagSFUtil_CSVM_BtagMap.h"
//#include "../interface/BTagSFUtil_Loose.h"
//#include "../interface/BTagSFUtil_CSVT.h"
//#include "../interface/BTagSFUtil_JPTM.h"
#include "RUAnalysis/Analysis/interface/LumiReweightingStandAlone.h"

using namespace std;

NtpThreeJet::NtpThreeJet (std::vector<TString>& InFileNames, bool const IsData,
TString const OutFileName, TString const PUFile) : NtpReader(InFileNames, IsData)
{
  fOutFile = new TFile(OutFileName, "recreate");
  cout<<"OutPut file: "<<OutFileName<<endl;
  stringstream ss;
  ss<<OutFileName;
  string NameFile= ss.str();
  string::size_type found = NameFile.find("BtagMap");
  string::size_type found0 = NameFile.find("mc_");
  int length=abs(int(found0 + 3) - int(found - 1));
  //Should be TTjets for now its the RPV sample
    string BtagMap = "TTJets_MassiveB_AK5PF_CSVM_bTaggingEfficiencyMap.root";

   if (found!=string::npos && found0!=string::npos)
    {
      BtagMap = NameFile.substr(found0 + 3, length) + "_AK5PF_CSVM_bTaggingEfficiencyMap.root";
    }
  
  cout<<"BtagMap: data/"<<BtagMap<<" as map for Btagging efficiencies"<<endl;
  
  

  f_EffMap = new TFile(("data/"+BtagMap).c_str());
  if (!f_EffMap->IsOpen()) {
    std::cerr << "ERROR: cannot open Efficiency map file " << "data/"<<BtagMap << std::endl;
    f_EffMap = new TFile("data/TTJets_MassiveB_AK5PF_CSVM_bTaggingEfficiencyMap.root");
   std::cerr << "BtagMap: TTbar instead"<< std::endl;
  if (!f_EffMap->IsOpen()) {
    std::cerr << "ERROR: cannot open Second  Efficiency map file data/TTJets_MassiveB_AK5PF_CSVM_bTaggingEfficiencyMap.root" << std::endl;
    throw;
  }
    }

  if (!fOutFile->IsOpen()) {
    std::cerr << "ERROR: cannot open output root file " << OutFileName << std::endl;
    throw;
  }
  fPUFile = new TFile("data/"+PUFile);
  if (fPUFile->IsOpen()) {
    std::cout << "Found PU file " << PUFile << std::endl;
	} else {
    std::cerr << "Did not find PU file " << PUFile << " -- using default" << std::endl;
  }
}

NtpThreeJet::~NtpThreeJet ()
{
    std::cout << "Starting NtpThreeJet destructor " << std::endl;
    if (fOutFile) {
			// fOutFile->Write(); // Writes another copy of all objects
			fOutFile->Close();
			std::cout << "Closed file, now deleting pointer" << std::endl;
			delete fOutFile;
    }
    if (fPUFile) {
			fPUFile->Close();
			delete fPUFile;
    }
    /*    if (f_EffMap) {
			f_EffMap->Close();
			delete fPUFile;
			}*/
    
    std::cout << "Done with NtpThreeJet destructor " << std::endl;
    std::cout << "Done with NtpThreeJet destructor " << std::endl;
}


void NtpThreeJet::BookHistograms()
{

  h2_EffMapB    = (TH2D*)f_EffMap->Get("efficiency_b");
  h2_EffMapC    = (TH2D*)f_EffMap->Get("efficiency_c");
  h2_EffMapUDSG = (TH2D*)f_EffMap->Get("efficiency_udsg");
  double PtBins[] = {20,30, 40, 50, 60, 70, 80, 100, 120, 160, 210, 260, 320, 400, 500, 600, 800};

  CSVM_SFb_0to2p4 = new TF1("CSVM_SFb_0to2p4","0.726981*((1.+(0.253238*x))/(1.+(0.188389*x)))", 30.,670.);

  // Tagger: CSVM within 30 < pt < 670 GeV, abs(eta) < 2.4, x = pt
  CSVM_SFb_errors = new TH1D("CSVM_SFb_errors", "CSVM_SFb_errors", 16, PtBins);  

  CSVM_SFb_errors->SetBinContent( 0,0.0554504);
  CSVM_SFb_errors->SetBinContent( 1,0.0209663);
  CSVM_SFb_errors->SetBinContent( 2,0.0207019);
  CSVM_SFb_errors->SetBinContent( 3,0.0230073);
  CSVM_SFb_errors->SetBinContent( 4,0.0208719);
  CSVM_SFb_errors->SetBinContent( 5,0.0200453);
  CSVM_SFb_errors->SetBinContent( 6,0.0264232);
  CSVM_SFb_errors->SetBinContent( 7,0.0240102);
  CSVM_SFb_errors->SetBinContent( 8,0.0229375);
  CSVM_SFb_errors->SetBinContent( 9,0.0184615);
  CSVM_SFb_errors->SetBinContent(10,0.0216242);
  CSVM_SFb_errors->SetBinContent(11,0.0248119);
  CSVM_SFb_errors->SetBinContent(12,0.0465748);
  CSVM_SFb_errors->SetBinContent(13,0.0474666);
  CSVM_SFb_errors->SetBinContent(14,0.0718173);
  CSVM_SFb_errors->SetBinContent(15,0.0717567);
  CSVM_SFb_errors->SetBinContent(16,(2*0.0717567));

  float ptmax=800;


  CSVM_SFl_0to2p4 = new TF1("CSVM_SFl_0to2p4","((1.07585+(0.00119553*x))+(-3.00163e-06*(x*x)))+(2.10724e-09*(x*(x*x)))", 20.,ptmax);
  CSVM_SFl_0to2p4_min= new TF1("CSVM_SFl_0to2p4_min","((0.987005+(0.000726254*x))+(-1.73476e-06*(x*x)))+(1.20406e-09*(x*(x*x)))", 20.,ptmax);
  CSVM_SFl_0to2p4_max = new TF1("CSVM_SFl_0to2p4_max","((1.1647+(0.00166318*x))+(-4.26493e-06*(x*x)))+(3.01017e-09*(x*(x*x)))", 20.,ptmax);
  

     CSVM_SFl_0to0p8 = new TF1("CSVM_SFl_0to0p8","((1.06238+(0.00198635*x))+(-4.89082e-06*(x*x)))+(3.29312e-09*(x*(x*x)))", 20.,ptmax);
     CSVM_SFl_0to0p8_min = new TF1("CSVM_SFl_0to0p8_min","((0.972746+(0.00104424*x))+(-2.36081e-06*(x*x)))+(1.53438e-09*(x*(x*x)))", 20.,ptmax);
      CSVM_SFl_0to0p8_max= new TF1("CSVM_SFl_0to0p8_max","((1.15201+(0.00292575*x))+(-7.41497e-06*(x*x)))+(5.0512e-09*(x*(x*x)))", 20.,ptmax);


	 CSVM_SFl_0p8to1p6= new TF1("CSVM_SFl_0p8to1p6","((1.08048+(0.00110831*x))+(-2.96189e-06*(x*x)))+(2.16266e-09*(x*(x*x)))", 20.,ptmax);
	  CSVM_SFl_0p8to1p6_min = new TF1("CSVM_SFl_0p8to1p6_min","((0.9836+(0.000649761*x))+(-1.59773e-06*(x*x)))+(1.14324e-09*(x*(x*x)))", 20.,ptmax);
	 CSVM_SFl_0p8to1p6_max = new TF1("CSVM_SFl_0p8to1p6_max","((1.17735+(0.00156533*x))+(-4.32257e-06*(x*x)))+(3.18197e-09*(x*(x*x)))", 20.,ptmax);

  
  CSVM_SFl_Corr = new TF1("CSVM_SFl_Corr","(1.10422 + (-0.000523856*x) + (1.14251e-06*(x*x)))", 0.,670.);

  CSVM_SFl_0to2p4 =   new TF1("CSVM_SFl_0to2p4","((1.04318+(0.000848162*x))+(-2.5795e-06*(x*x)))+(1.64156e-09*(x*(x*x)))", 20.,800.);
  CSVM_SFl_0to0p8 =   new TF1("CSVM_SFl_0to0p8","((1.06182+(0.000617034*x))+(-1.5732e-06*(x*x)))+(3.02909e-10*(x*(x*x)))", 20.,800.);
  CSVM_SFl_0p8to1p6 = new TF1("CSVM_SFl_0p8to1p6","((1.111+(-9.64191e-06*x))+(1.80811e-07*(x*x)))+(-5.44868e-10*(x*(x*x)))", 20.,6.);
  CSVM_SFl_1p6to2p4 = new TF1("CSVM_SFl_1p6to2p4","((1.08498+(-0.000701422*x))+(3.43612e-06*(x*x)))+(-4.11794e-09*(x*(x*x)))", 20.,670.);


     CSVM_SFl_1p6to2p4 = new TF1("CSVM_SFl_1p6to2p4","((1.09145+(0.000687171*x))+(-2.45054e-06*(x*x)))+(1.7844e-09*(x*(x*x)))", 20.,ptmax);
     CSVM_SFl_1p6to2p4_min = new TF1("CSVM_SFl_1p6to2p4_min","((1.00616+(0.000358884*x))+(-1.23768e-06*(x*x)))+(6.86678e-10*(x*(x*x)))", 20.,ptmax);
     CSVM_SFl_1p6to2p4_max = new TF1("CSVM_SFl_1p6to2p4_max","((1.17671+(0.0010147*x))+(-3.66269e-06*(x*x)))+(2.88425e-09*(x*(x*x)))", 20.,ptmax);




  char hNAME[99];
 
  //before cuts, just the ones that where already in the ntuple
  h_PossibleTrigger  = new TH1F("PossibleTrigger", "PossibleTrigger",30,0,30);   h_PossibleTrigger->Sumw2();

  h_mindRMuonJet = new TH1F("mindRMuonJet", "mindRMuonJet",100,0,5);   h_mindRMuonJet->Sumw2();
  h_mindRMuonJet_mPFIso = new TH2F("mindRMuonJet_mPFIso", "mindRMuonJet_mPFIso",200,0,10,100,0,5);  h_mindRMuonJet_mPFIso->Sumw2();
  h_ept= new TH1F("ept", "ept",200,0,1000); h_ept->Sumw2();
  h_mpt= new TH1F("mpt", "mpt",200,0,1000); h_mpt->Sumw2();
  h_mPFIso= new TH1F("mPFIso", "mPFIso",200,0,10); h_mPFIso->Sumw2();
  //befor cuts
  h_MET = new TH1F("MET", "MET",200,0,800); h_MET->Sumw2();
  h_nTruePileUp = new TH1F("nTruePileUp", "nTruePileUp",100,0,100); h_nTruePileUp->Sumw2();
  h_nTruePileUp_NoWeight = new TH1F("nTruePileUp_NoWeight", "nTruePileUp_NoWeight",100,0,100); h_nTruePileUp_NoWeight->Sumw2();
  h_nVertex = new TH1F("nVertex", "nVertex",100,0,100); h_nVertex->Sumw2();
  h_HT = new TH1F("HT", "HT",300,0,3000); h_HT->Sumw2();
  h_HMT = new TH1F("HMT", "HMT", 300,0,6000); h_HMT->Sumw2();
  h_nBJet35  = new TH1F("nBJet35", "nBJet35",20,0,20); h_nBJet35->Sumw2();
  h_nJet35  = new TH1F("nJet35", "nJet35",20,0,20); h_nJet35->Sumw2();
  h_Jet0= new TH1F("Jet0", "Jet0",400,0,2000); h_Jet0->Sumw2();
  h_Jet1= new TH1F("Jet1", "Jet1",400,0,2000); h_Jet1->Sumw2();
  h_Jet2= new TH1F("Jet2", "Jet2",200,0,1000); h_Jet2->Sumw2();
  h_Jet3= new TH1F("Jet3", "Jet3",200,0,1000); h_Jet3->Sumw2();
  h_Jet4= new TH1F("Jet4", "Jet4",200,0,1000); h_Jet4->Sumw2();
  h_Jet5= new TH1F("Jet5", "Jet5",200,0,1000); h_Jet5->Sumw2();
  h_Jet6= new TH1F("Jet6", "Jet6",200,0,1000); h_Jet6->Sumw2();
  h_BJet1= new TH1F("BJet1", "BJet1",200,0,1000); h_BJet1->Sumw2();
  h_BJet2= new TH1F("BJet2", "BJet2",200,0,1000); h_BJet2->Sumw2();
  h_BJet3= new TH1F("BJet3", "BJet3",200,0,1000); h_BJet3->Sumw2();
  h_NeutralHad_JetPt= new TH2F("NeutralHad_JetPt", "NeutralHad_JetPt",200,0,1000,100,0,1);  h_NeutralHad_JetPt->Sumw2();

  //after cuts
  h_MET_EvtSel = new TH1F("MET_EvtSel", "MET_EvtSel",200,0,1000); h_MET_EvtSel->Sumw2();
  h_HT_EvtSel = new TH1F("HT_EvtSel", "HT_EvtSel",400,0,4000); h_HT_EvtSel->Sumw2();
  h_MJ_EvtSel = new TH1F("MJ_EvtSel", "MJ_EvtSel",400,0,4000); h_MJ_EvtSel->Sumw2();
  h_HMT_EvtSel = new TH1F("HMT_EvtSel", "HMT_EvtSel",400,0,4000); h_HMT_EvtSel->Sumw2();
  h_nBJet35_EvtSel  = new TH1F("nBJet35_EvtSel", "nBJet35_EvtSel",20,0,20); h_nBJet35_EvtSel->Sumw2();
  h_nJet35_EvtSel  = new TH1F("nJet35_EvtSel", "nJet35_EvtSel",20,0,20); h_nJet35_EvtSel->Sumw2();
  h_Jet0_EvtSel= new TH1F("Jet0_EvtSel", "Jet0_EvtSel",400,0,2000); h_Jet0_EvtSel->Sumw2();
  h_Jet1_EvtSel= new TH1F("Jet1_EvtSel", "Jet1_EvtSel",400,0,2000); h_Jet1_EvtSel->Sumw2();
  h_Jet2_EvtSel= new TH1F("Jet2_EvtSel", "Jet2_EvtSel",200,0,1000); h_Jet2_EvtSel->Sumw2();
  h_Jet3_EvtSel= new TH1F("Jet3_EvtSel", "Jet3_EvtSel",200,0,1000); h_Jet3_EvtSel->Sumw2();
  h_Jet4_EvtSel= new TH1F("Jet4_EvtSel", "Jet4_EvtSel",200,0,1000); h_Jet4_EvtSel->Sumw2();
  h_Jet5_EvtSel= new TH1F("Jet5_EvtSel", "Jet5_EvtSel",200,0,1000); h_Jet5_EvtSel->Sumw2();
  h_Jet6_EvtSel= new TH1F("Jet6_EvtSel", "Jet6_EvtSel",200,0,1000); h_Jet6_EvtSel->Sumw2();
  h_BJet1_EvtSel= new TH1F("BJet1_EvtSel", "BJet1_EvtSel",200,0,1000); h_BJet1_EvtSel->Sumw2();
  h_BJet2_EvtSel= new TH1F("BJet2_EvtSel", "BJet2_EvtSel",200,0,1000); h_BJet2_EvtSel->Sumw2();
  h_BJet3_EvtSel= new TH1F("BJet3_EvtSel", "BJet3_EvtSel",200,0,1000); h_BJet3_EvtSel->Sumw2();

  cout<<"before histos"<<endl;
  Double_t tbins[59]={0,10,20,30,40,50,60,70,80,90,
		      100,110,121,131,141,151,162,174,187,201,
		      216,232,249,267,286,307,329,353,379,406,
		      435,466,500,536,575,617,662,710,761,816,
		      875,938,1006,1079,1157,1241,1331,1427,1530,1641,
		      1760,1887,2024,2170,2327,2495,2677,3000,3400};

  // for(int b=0; b<5; b++){
  for(int b=0; b<2; b++){
    // for (int i=0; i<11; i++){
    for (int i=0; i<9; i++){
	  int iPt=30+i*10;
      //Triplet mass plots
      Mjjj_sumpt_bjet_pt_njet.push_back(std::vector<std::vector<TH2F*> >());
      Mjjj_vecpt_bjet_pt_njet.push_back(std::vector<std::vector<TH2F*> >());
      Mjjj_P_bjet_pt_njet.push_back(std::vector<std::vector<TH2F*> >());
      DeltaM_bjet_pt_njet.push_back(std::vector<std::vector<TH1F*> >());
      Mjjj_sumpt_btag_pt_njet.push_back(std::vector<std::vector<TH2F*> >());      
      Aplanarity_bjet_pt_njet.push_back(std::vector<std::vector<TH1F*> >());
      Sphericity_bjet_pt_njet.push_back(std::vector<std::vector<TH1F*> >());
      Circularity_bjet_pt_njet.push_back(std::vector<std::vector<TH1F*> >());
      Isotropy_bjet_pt_njet.push_back(std::vector<std::vector<TH1F*> >());
      C_bjet_pt_njet.push_back(std::vector<std::vector<TH1F*> >());
      D_bjet_pt_njet.push_back(std::vector<std::vector<TH1F*> >());
      
      Mjjj_bjet_pt_njet_diag.push_back(std::vector<std::vector<std::vector<TH1F*> > >());
      Mjjj_btag_bjet_pt_njet_diag.push_back(std::vector<std::vector<std::vector<TH1F*> > >());
      Mjjj_btag_bjet_pt_njet_diag_1trp.push_back(std::vector<std::vector<std::vector<TH1F*> > >());

      Mjjj_Sph4_bjet_pt_njet_diag.push_back(std::vector<std::vector<std::vector<TH1F*> > >());
      Mjjj_Sph4_bjet_pt_njet_diag_1trp.push_back(std::vector<std::vector<std::vector<TH1F*> > >());
      Mjjj_Sph4_btag_bjet_pt_njet_diag.push_back(std::vector<std::vector<std::vector<TH1F*> > >());
      Mjjj_Sph4_btag_bjet_pt_njet_diag_1trp.push_back(std::vector<std::vector<std::vector<TH1F*> > >());
      Mjjj_Sph4_btag_bjet_trpwt_pt_njet_diag.push_back(std::vector<std::vector<std::vector<TH1F*> > >());
      Mjjj_btag_bjet_trpwt_pt_njet_diag.push_back(std::vector<std::vector<std::vector<TH1F*> > >());
      MjjjSym_bjet_pt_njet_diag.push_back(std::vector<std::vector<std::vector<TH1F*> > >());      
      Mjjj_bjet_pt_njet_diag_MCmatch.push_back(std::vector<std::vector<std::vector<TH1F*> > >());
      Mjjj_bjet_pt_njet_diag_MCcomb.push_back(std::vector<std::vector<std::vector<TH1F*> > >());
      Mjjj_Sph4_bjet_pt_njet_diag_MCmatch.push_back(std::vector<std::vector<std::vector<TH1F*> > >());
      Mjjj_Sph4_bjet_pt_njet_diag_MCcomb.push_back(std::vector<std::vector<std::vector<TH1F*> > >());
      
      HT_bjet_pt_njet_diag.push_back(std::vector<std::vector<std::vector<TH1F*> > >());
      HT_btag_bjet_pt_njet_diag.push_back(std::vector<std::vector<std::vector<TH1F*> > >());
      HT_Sph4_btag_bjet_pt_njet_diag.push_back(std::vector<std::vector<std::vector<TH1F*> > >());
      Ntrip_bjet_pt_njet_diag_MCmatch.push_back(std::vector<std::vector<std::vector<TH1F*> > >());
      Ntrip_bjet_pt_njet_diag_MCcomb.push_back(std::vector<std::vector<std::vector<TH1F*> > >());

      Ntrip_btag_bjet_pt_njet_diag.push_back(std::vector<std::vector<std::vector<TH1F*> > >());
      Ntrip_Sph4_btag_bjet_pt_njet_diag.push_back(std::vector<std::vector<std::vector<TH1F*> > >());
      
      MjjjSym_bjet_pt_njet_diag_MCmatch.push_back(std::vector<std::vector<std::vector<TH1F*> > >());
      MjjjSym_bjet_pt_njet_diag_MCcomb.push_back(std::vector<std::vector<std::vector<TH1F*> > >());
      
      P_bjet_pt_njet_diag_MCmatch.push_back(std::vector<std::vector<std::vector<TH1F*> > >());
      P_bjet_pt_njet_diag_MCcomb.push_back(std::vector<std::vector<std::vector<TH1F*> > >());
      
      h31_bjet_pt_njet_diag_MCmatch.push_back(std::vector<std::vector<std::vector<TH1F*> > >());
      h31_bjet_pt_njet_diag_MCcomb.push_back(std::vector<std::vector<std::vector<TH1F*> > >());
      
      Maxtr_bjet_pt_njet_diag_MCmatch.push_back(std::vector<std::vector<std::vector<TH1F*> > >());
      Maxtr_bjet_pt_njet_diag_MCcomb.push_back(std::vector<std::vector<std::vector<TH1F*> > >());
      
      MET_bjet_pt_njet_diag.push_back(std::vector<std::vector<std::vector<TH1F*> > >());
      
      
      P_bjet_pt_njet_diag.push_back(std::vector<std::vector<std::vector<TH1F*> > >());
      MET_over_HT_bjet_pt_njet_diag.push_back(std::vector<std::vector<std::vector<TH1F*> > >());
      HMT_bjet_pt_njet_diag.push_back(std::vector<std::vector<std::vector<TH1F*> > >());
      nJet35_bjet_pt_njet_diag.push_back(std::vector<std::vector<std::vector<TH1F*> > >());
      nBJet35_bjet_pt_njet_diag.push_back(std::vector<std::vector<std::vector<TH1F*> > >());
      nVertex_bjet_pt_njet_diag.push_back(std::vector<std::vector<std::vector<TH1F*> > >());
      Jet0_bjet_pt_njet_diag.push_back(std::vector<std::vector<std::vector<TH1F*> > >());
      Jet1_bjet_pt_njet_diag.push_back(std::vector<std::vector<std::vector<TH1F*> > >());
      Jet2_bjet_pt_njet_diag.push_back(std::vector<std::vector<std::vector<TH1F*> > >());
      Jet3_bjet_pt_njet_diag.push_back(std::vector<std::vector<std::vector<TH1F*> > >());
      Jet4_bjet_pt_njet_diag.push_back(std::vector<std::vector<std::vector<TH1F*> > >());
      Jet5_bjet_pt_njet_diag.push_back(std::vector<std::vector<std::vector<TH1F*> > >());
      Jet6_bjet_pt_njet_diag.push_back(std::vector<std::vector<std::vector<TH1F*> > >());
      BJet1_bjet_pt_njet_diag.push_back(std::vector<std::vector<std::vector<TH1F*> > >());
      BJet2_bjet_pt_njet_diag.push_back(std::vector<std::vector<std::vector<TH1F*> > >());
      BJet3_bjet_pt_njet_diag.push_back(std::vector<std::vector<std::vector<TH1F*> > >());
      
      //Dalitz Plots
      MjjHigh_MjjMid_bjet_pt_njet_diag.push_back(std::vector<std::vector<std::vector<TH2F*> > >());
      MjjHigh_MjjLow_bjet_pt_njet_diag.push_back(std::vector<std::vector<std::vector<TH2F*> > >());
      Mjjj_btag_bjet_pt_njet_diag.push_back(std::vector<std::vector<std::vector<TH1F*> > >());
       Mjjj_btag_bjet_pt_njet_diag_1trp.push_back(std::vector<std::vector<std::vector<TH1F*> > >());
     
      MjjMid_MjjLow_bjet_pt_njet_diag.push_back(std::vector<std::vector<std::vector<TH2F*> > >());
      //Dijet mass plots
      M12_bjet_pt_njet_diag.push_back(std::vector<std::vector<std::vector<TH1F*> > >());
      M13_bjet_pt_njet_diag.push_back(std::vector<std::vector<std::vector<TH1F*> > >());
      M23_bjet_pt_njet_diag.push_back(std::vector<std::vector<std::vector<TH1F*> > >());
      // for(int k=0; k<3; k++){
      for(int k=0; k<1; k++){
	int iNjet=k+6;
	
      Mjjj_sumpt_bjet_pt_njet[b].push_back(std::vector<TH2F*> ());
      Mjjj_vecpt_bjet_pt_njet[b].push_back(std::vector<TH2F*> ());
      Mjjj_P_bjet_pt_njet[b].push_back(std::vector<TH2F*> ());
      DeltaM_bjet_pt_njet[b].push_back(std::vector<TH1F*> ());
      Mjjj_sumpt_btag_pt_njet[b].push_back(std::vector<TH2F*> ());

      Aplanarity_bjet_pt_njet[b].push_back(std::vector<TH1F*> ());
      Sphericity_bjet_pt_njet[b].push_back(std::vector<TH1F*> ());
      Circularity_bjet_pt_njet[b].push_back(std::vector<TH1F*> ());
      Isotropy_bjet_pt_njet[b].push_back(std::vector<TH1F*> ());
      C_bjet_pt_njet[b].push_back(std::vector<TH1F*> ());
      D_bjet_pt_njet[b].push_back(std::vector<TH1F*> ());


      sprintf(hNAME, "Mjjj_sumpt_bjet%i_pt%i_GE%ijet", b, iPt,iNjet);
      Mjjj_sumpt_bjet_pt_njet[b][i].push_back(new TH2F(hNAME,hNAME,200,0,2000,200,0,2000));
      Mjjj_sumpt_bjet_pt_njet[b][i][k]->Sumw2();

      sprintf(hNAME, "Mjjj_vecpt_bjet%i_pt%i_GE%ijet", b, iPt,iNjet);
      Mjjj_vecpt_bjet_pt_njet[b][i].push_back(new TH2F(hNAME,hNAME,200,0,2000,200,0,2000));
      Mjjj_vecpt_bjet_pt_njet[b][i][k]->Sumw2();

      sprintf(hNAME, "Aplanarity_bjet%i_pt%i_GE%ijet", b, iPt,iNjet);
      Aplanarity_bjet_pt_njet[b][i].push_back(new TH1F(hNAME,hNAME,100,0,1));
      Aplanarity_bjet_pt_njet[b][i][k]->Sumw2();
      sprintf(hNAME, "Sphericity_bjet%i_pt%i_GE%ijet", b, iPt,iNjet);
      Sphericity_bjet_pt_njet[b][i].push_back(new TH1F(hNAME,hNAME,100,0,1));
      Sphericity_bjet_pt_njet[b][i][k]->Sumw2();

      sprintf(hNAME, "Circularity_bjet%i_pt%i_GE%ijet", b, iPt,iNjet);
      Circularity_bjet_pt_njet[b][i].push_back(new TH1F(hNAME,hNAME,100,0,1));
      Circularity_bjet_pt_njet[b][i][k]->Sumw2();
      
      sprintf(hNAME, "Isotropy_bjet%i_pt%i_GE%ijet", b, iPt,iNjet);
      Isotropy_bjet_pt_njet[b][i].push_back(new TH1F(hNAME,hNAME,100,0,1));
      Isotropy_bjet_pt_njet[b][i][k]->Sumw2();
      sprintf(hNAME, "C_bjet%i_pt%i_GE%ijet", b, iPt,iNjet);
      C_bjet_pt_njet[b][i].push_back(new TH1F(hNAME,hNAME,100,0,1));
      C_bjet_pt_njet[b][i][k]->Sumw2();
      sprintf(hNAME, "D_bjet%i_pt%i_GE%ijet", b, iPt,iNjet);
      D_bjet_pt_njet[b][i].push_back(new TH1F(hNAME,hNAME,100,0,1));
      D_bjet_pt_njet[b][i][k]->Sumw2();

      sprintf(hNAME, "Mjjj_P_bjet%i_pt%i_GE%ijet", b, iPt,iNjet);
      Mjjj_P_bjet_pt_njet[b][i].push_back(new TH2F(hNAME,hNAME,200,0,20,200,0,2000));
      Mjjj_P_bjet_pt_njet[b][i][k]->Sumw2();

      sprintf(hNAME, "DeltaM_bjet%i_pt%i_GE%ijet", b, iPt,iNjet);
      DeltaM_bjet_pt_njet[b][i].push_back(new TH1F(hNAME,hNAME,200,-2000,2000));
      DeltaM_bjet_pt_njet[b][i][k]->Sumw2();

      sprintf(hNAME, "Mjjjbtag_sumpt_bjet%i_pt%i_GE%ijet", b, iPt,iNjet);
      Mjjj_sumpt_btag_pt_njet[b][i].push_back(new TH2F(hNAME,hNAME,200,0,2000,200,0,2000));
      Mjjj_sumpt_btag_pt_njet[b][i][k]->Sumw2();

      Mjjj_bjet_pt_njet_diag[b].push_back(std::vector<std::vector<TH1F*> > ());
      Mjjj_btag_bjet_pt_njet_diag[b].push_back(std::vector<std::vector<TH1F*> > ());
      Mjjj_btag_bjet_pt_njet_diag_1trp[b].push_back(std::vector<std::vector<TH1F*> > ());

      Mjjj_Sph4_bjet_pt_njet_diag[b].push_back(std::vector<std::vector<TH1F*> > ());
      Mjjj_Sph4_bjet_pt_njet_diag_1trp[b].push_back(std::vector<std::vector<TH1F*> > ());
      Mjjj_Sph4_btag_bjet_pt_njet_diag[b].push_back(std::vector<std::vector<TH1F*> > ());
      Mjjj_Sph4_btag_bjet_pt_njet_diag_1trp[b].push_back(std::vector<std::vector<TH1F*> > ());
      Mjjj_Sph4_btag_bjet_trpwt_pt_njet_diag[b].push_back(std::vector<std::vector<TH1F*> > ());
      Mjjj_btag_bjet_trpwt_pt_njet_diag[b].push_back(std::vector<std::vector<TH1F*> > ());

      MjjjSym_bjet_pt_njet_diag[b].push_back(std::vector<std::vector<TH1F*> > ());      
      Mjjj_bjet_pt_njet_diag_MCmatch[b].push_back(std::vector<std::vector<TH1F*> > ());
      Mjjj_bjet_pt_njet_diag_MCcomb[b].push_back(std::vector<std::vector<TH1F*> > ());

      Mjjj_Sph4_bjet_pt_njet_diag_MCmatch[b].push_back(std::vector<std::vector<TH1F*> > ());
      Mjjj_Sph4_bjet_pt_njet_diag_MCcomb[b].push_back(std::vector<std::vector<TH1F*> > ());

      Ntrip_bjet_pt_njet_diag_MCmatch[b].push_back(std::vector<std::vector<TH1F*> > ());
      Ntrip_bjet_pt_njet_diag_MCcomb[b].push_back(std::vector<std::vector<TH1F*> > ());

      Ntrip_btag_bjet_pt_njet_diag[b].push_back(std::vector<std::vector<TH1F*> > ());
      Ntrip_Sph4_btag_bjet_pt_njet_diag[b].push_back(std::vector<std::vector<TH1F*> > ());

      MjjjSym_bjet_pt_njet_diag_MCmatch[b].push_back(std::vector<std::vector<TH1F*> > ());
      MjjjSym_bjet_pt_njet_diag_MCcomb[b].push_back(std::vector<std::vector<TH1F*> > ());

      P_bjet_pt_njet_diag_MCmatch[b].push_back(std::vector<std::vector<TH1F*> > ());
      P_bjet_pt_njet_diag_MCcomb[b].push_back(std::vector<std::vector<TH1F*> > ());

      h31_bjet_pt_njet_diag_MCmatch[b].push_back(std::vector<std::vector<TH1F*> > ());
      h31_bjet_pt_njet_diag_MCcomb[b].push_back(std::vector<std::vector<TH1F*> > ());

      Maxtr_bjet_pt_njet_diag_MCmatch[b].push_back(std::vector<std::vector<TH1F*> > ());
      Maxtr_bjet_pt_njet_diag_MCcomb[b].push_back(std::vector<std::vector<TH1F*> > ());

      MET_bjet_pt_njet_diag[b].push_back(std::vector<std::vector<TH1F*> > ());
      HT_bjet_pt_njet_diag[b].push_back(std::vector<std::vector<TH1F*> > ());
HT_btag_bjet_pt_njet_diag[b].push_back(std::vector<std::vector<TH1F*> > ());
HT_Sph4_btag_bjet_pt_njet_diag[b].push_back(std::vector<std::vector<TH1F*> > ());
      P_bjet_pt_njet_diag[b].push_back(std::vector<std::vector<TH1F*> > ());
      MET_over_HT_bjet_pt_njet_diag[b].push_back(std::vector<std::vector<TH1F*> > ());
      HMT_bjet_pt_njet_diag[b].push_back(std::vector<std::vector<TH1F*> > ());
      nBJet35_bjet_pt_njet_diag[b].push_back(std::vector<std::vector<TH1F*> > ());
      nJet35_bjet_pt_njet_diag[b].push_back(std::vector<std::vector<TH1F*> > ());
      nVertex_bjet_pt_njet_diag[b].push_back(std::vector<std::vector<TH1F*> > ());
      Jet0_bjet_pt_njet_diag[b].push_back(std::vector<std::vector<TH1F*> > ());
      Jet1_bjet_pt_njet_diag[b].push_back(std::vector<std::vector<TH1F*> > ());
      Jet2_bjet_pt_njet_diag[b].push_back(std::vector<std::vector<TH1F*> > ());
      Jet3_bjet_pt_njet_diag[b].push_back(std::vector<std::vector<TH1F*> > ());
      Jet4_bjet_pt_njet_diag[b].push_back(std::vector<std::vector<TH1F*> > ());
      Jet5_bjet_pt_njet_diag[b].push_back(std::vector<std::vector<TH1F*> > ());
      Jet6_bjet_pt_njet_diag[b].push_back(std::vector<std::vector<TH1F*> > ());
      BJet1_bjet_pt_njet_diag[b].push_back(std::vector<std::vector<TH1F*> > ());
      BJet2_bjet_pt_njet_diag[b].push_back(std::vector<std::vector<TH1F*> > ());
      BJet3_bjet_pt_njet_diag[b].push_back(std::vector<std::vector<TH1F*> > ());

      MjjHigh_MjjMid_bjet_pt_njet_diag[b].push_back(std::vector<std::vector<TH2F*> > ());
      MjjHigh_MjjLow_bjet_pt_njet_diag[b].push_back(std::vector<std::vector<TH2F*> > ());
      MjjMid_MjjLow_bjet_pt_njet_diag[b].push_back(std::vector<std::vector<TH2F*> > ());

      M12_bjet_pt_njet_diag[b].push_back(std::vector<std::vector<TH1F*> > ());
      M13_bjet_pt_njet_diag[b].push_back(std::vector<std::vector<TH1F*> > ());
      M23_bjet_pt_njet_diag[b].push_back(std::vector<std::vector<TH1F*> > ());
      // for(int j=0; j<25; j++){
	 for(int j=0; j<8; j++){
	
	int iDiag=j*10+40;
	bool ResBin=true;
	Mjjj_bjet_pt_njet_diag[b][i].push_back(std::vector<TH1F*> ());
	sprintf(hNAME, "Mjjj_bjet%i_pt%i_diag%i_GE%ijet", b,iPt,iDiag,iNjet);
	if(!ResBin)Mjjj_bjet_pt_njet_diag[b][i][k].push_back(new TH1F(hNAME,hNAME,250,0,2500));
	else if(ResBin)Mjjj_bjet_pt_njet_diag[b][i][k].push_back(new TH1F(hNAME,hNAME,58,tbins));
	Mjjj_bjet_pt_njet_diag[b][i][k][j]->Sumw2();

	Mjjj_btag_bjet_pt_njet_diag[b][i].push_back(std::vector<TH1F*> ());
	if(b<1) sprintf(hNAME, "Mjjj_btag%i_bjet%i_pt%i_diag%i_GE%ijet", b,b,iPt,iDiag,iNjet);
	if(b>=1) sprintf(hNAME, "Mjjj_btag%i_bjet%i_pt%i_diag%i_GE%ijet", 1,b,iPt,iDiag,iNjet);
	if(!ResBin)Mjjj_btag_bjet_pt_njet_diag[b][i][k].push_back(new TH1F(hNAME,hNAME,250,0,2500));
	else if(ResBin)Mjjj_btag_bjet_pt_njet_diag[b][i][k].push_back(new TH1F(hNAME,hNAME,58,tbins));
	Mjjj_btag_bjet_pt_njet_diag[b][i][k][j]->Sumw2();

	Mjjj_btag_bjet_pt_njet_diag_1trp[b][i].push_back(std::vector<TH1F*> ());
	if(b<1) sprintf(hNAME, "Mjjj_btag%i_bjet%i_pt%i_diag%i_GE%ijet_1trp", b,b,iPt,iDiag,iNjet);
	if(b>=1) sprintf(hNAME, "Mjjj_btag%i_bjet%i_pt%i_diag%i_GE%ijet_1trp", 1,b,iPt,iDiag,iNjet);
	if(!ResBin)Mjjj_btag_bjet_pt_njet_diag_1trp[b][i][k].push_back(new TH1F(hNAME,hNAME,250,0,2500));
	else if(ResBin)Mjjj_btag_bjet_pt_njet_diag_1trp[b][i][k].push_back(new TH1F(hNAME,hNAME,58,tbins));
	Mjjj_btag_bjet_pt_njet_diag_1trp[b][i][k][j]->Sumw2();


	Mjjj_Sph4_bjet_pt_njet_diag[b][i].push_back(std::vector<TH1F*> ());
	sprintf(hNAME, "Mjjj_Sph4_bjet%i_pt%i_diag%i_GE%ijet", b,iPt,iDiag,iNjet);
	if(!ResBin)Mjjj_Sph4_bjet_pt_njet_diag[b][i][k].push_back(new TH1F(hNAME,hNAME,250,0,2500));
	else if(ResBin)Mjjj_Sph4_bjet_pt_njet_diag[b][i][k].push_back(new TH1F(hNAME,hNAME,58,tbins));
	Mjjj_Sph4_bjet_pt_njet_diag[b][i][k][j]->Sumw2();

	Mjjj_Sph4_bjet_pt_njet_diag_1trp[b][i].push_back(std::vector<TH1F*> ());
	sprintf(hNAME, "Mjjj_Sph4_bjet%i_pt%i_diag%i_GE%ijet_1trp", b,iPt,iDiag,iNjet);
	if(!ResBin)Mjjj_Sph4_bjet_pt_njet_diag_1trp[b][i][k].push_back(new TH1F(hNAME,hNAME,250,0,2500));
	else if(ResBin)Mjjj_Sph4_bjet_pt_njet_diag_1trp[b][i][k].push_back(new TH1F(hNAME,hNAME,58,tbins));
	Mjjj_Sph4_bjet_pt_njet_diag_1trp[b][i][k][j]->Sumw2();

	Mjjj_Sph4_btag_bjet_pt_njet_diag[b][i].push_back(std::vector<TH1F*> ());
	if(b<1) sprintf(hNAME, "Mjjj_Sph4_btag%i_bjet%i_pt%i_diag%i_GE%ijet", b,b,iPt,iDiag,iNjet);
	if(b>=1) sprintf(hNAME, "Mjjj_Sph4_btag%i_bjet%i_pt%i_diag%i_GE%ijet", 1,b,iPt,iDiag,iNjet);
	if(!ResBin)Mjjj_Sph4_btag_bjet_pt_njet_diag[b][i][k].push_back(new TH1F(hNAME,hNAME,250,0,2500));
	else if(ResBin)Mjjj_Sph4_btag_bjet_pt_njet_diag[b][i][k].push_back(new TH1F(hNAME,hNAME,58,tbins));
	Mjjj_Sph4_btag_bjet_pt_njet_diag[b][i][k][j]->Sumw2();
	
	Mjjj_Sph4_btag_bjet_pt_njet_diag_1trp[b][i].push_back(std::vector<TH1F*> ());
	if(b<1) sprintf(hNAME, "Mjjj_Sph4_btag%i_bjet%i_pt%i_diag%i_GE%ijet_1trp", b,b,iPt,iDiag,iNjet);
	if(b>=1) sprintf(hNAME, "Mjjj_Sph4_btag%i_bjet%i_pt%i_diag%i_GE%ijet_1trp", 1,b,iPt,iDiag,iNjet);
	if(!ResBin)Mjjj_Sph4_btag_bjet_pt_njet_diag_1trp[b][i][k].push_back(new TH1F(hNAME,hNAME,250,0,2500));
	else if(ResBin)Mjjj_Sph4_btag_bjet_pt_njet_diag_1trp[b][i][k].push_back(new TH1F(hNAME,hNAME,58,tbins));
	Mjjj_Sph4_btag_bjet_pt_njet_diag_1trp[b][i][k][j]->Sumw2();
	
	Mjjj_Sph4_btag_bjet_trpwt_pt_njet_diag[b][i].push_back(std::vector<TH1F*> ());
	if(b<1) sprintf(hNAME, "Mjjj_Sph4_btag%i_bjet%i_trpwt_pt%i_diag%i_GE%ijet", b,b,iPt,iDiag,iNjet);
	if(b>=1) sprintf(hNAME, "Mjjj_Sph4_btag%i_bjet%i_trpwt_pt%i_diag%i_GE%ijet", 1,b,iPt,iDiag,iNjet);
	Mjjj_Sph4_btag_bjet_trpwt_pt_njet_diag[b][i][k].push_back(new TH1F(hNAME, hNAME,250, 0, 2500));
	Mjjj_Sph4_btag_bjet_trpwt_pt_njet_diag[b][i][k][j]->Sumw2();


	Mjjj_btag_bjet_trpwt_pt_njet_diag[b][i].push_back(std::vector<TH1F*> ());
	if(b<1) sprintf(hNAME, "Mjjj_btag%i_bjet%i_trpwt_pt%i_diag%i_GE%ijet", b,b,iPt,iDiag,iNjet);
	if(b>=1) sprintf(hNAME, "Mjjj_btag%i_bjet%i_trpwt_pt%i_diag%i_GE%ijet", 1,b,iPt,iDiag,iNjet);
	Mjjj_btag_bjet_trpwt_pt_njet_diag[b][i][k].push_back(new TH1F(hNAME, hNAME,250, 0, 2500));
	Mjjj_btag_bjet_trpwt_pt_njet_diag[b][i][k][j]->Sumw2();


	MjjjSym_bjet_pt_njet_diag[b][i].push_back(std::vector<TH1F*> ());
	sprintf(hNAME, "MjjjSym_bjet%i_pt%i_diag%i_GE%ijet", b,iPt,iDiag,iNjet);
	MjjjSym_bjet_pt_njet_diag[b][i][k].push_back(new TH1F(hNAME,hNAME,100,0,1));
	MjjjSym_bjet_pt_njet_diag[b][i][k][j]->Sumw2();

	Mjjj_bjet_pt_njet_diag_MCmatch[b][i].push_back(std::vector<TH1F*> ());
        sprintf(hNAME, "Mjjj_bjet%i_pt%i_diag%i_GE%ijet_MCmatch", b,iPt,iDiag,iNjet);
	if(!ResBin)Mjjj_bjet_pt_njet_diag_MCmatch[b][i][k].push_back(new TH1F(hNAME,hNAME,250,0,2500));
	else if(ResBin)Mjjj_bjet_pt_njet_diag_MCmatch[b][i][k].push_back(new TH1F(hNAME,hNAME,58,tbins));
        Mjjj_bjet_pt_njet_diag_MCmatch[b][i][k][j]->Sumw2();

	Mjjj_bjet_pt_njet_diag_MCcomb[b][i].push_back(std::vector<TH1F*> ());
        sprintf(hNAME, "Mjjj_bjet%i_pt%i_diag%i_GE%ijet_MCcomb", b,iPt,iDiag,iNjet);
	if(!ResBin)Mjjj_bjet_pt_njet_diag_MCcomb[b][i][k].push_back(new TH1F(hNAME,hNAME,250,0,2500));
	else if(ResBin)Mjjj_bjet_pt_njet_diag_MCcomb[b][i][k].push_back(new TH1F(hNAME,hNAME,58,tbins));
        Mjjj_bjet_pt_njet_diag_MCcomb[b][i][k][j]->Sumw2();

	Mjjj_Sph4_bjet_pt_njet_diag_MCcomb[b][i].push_back(std::vector<TH1F*> ());
	sprintf(hNAME, "Mjjj_Sph4_bjet%i_pt%i_diag%i_GE%ijet_MCcomb", b,iPt,iDiag,iNjet);
	if(!ResBin)Mjjj_Sph4_bjet_pt_njet_diag_MCcomb[b][i][k].push_back(new TH1F(hNAME,hNAME,250,0,2500));
	else if(ResBin)Mjjj_Sph4_bjet_pt_njet_diag_MCcomb[b][i][k].push_back(new TH1F(hNAME,hNAME,58,tbins));
	Mjjj_Sph4_bjet_pt_njet_diag_MCcomb[b][i][k][j]->Sumw2();


	Mjjj_Sph4_bjet_pt_njet_diag_MCmatch[b][i].push_back(std::vector<TH1F*> ());
	sprintf(hNAME, "Mjjj_Sph4_bjet%i_pt%i_diag%i_GE%ijet_MCmatch", b,iPt,iDiag,iNjet);
	if(!ResBin)Mjjj_Sph4_bjet_pt_njet_diag_MCmatch[b][i][k].push_back(new TH1F(hNAME,hNAME,250,0,2500));
	else if(ResBin)Mjjj_Sph4_bjet_pt_njet_diag_MCmatch[b][i][k].push_back(new TH1F(hNAME,hNAME,58,tbins));
	Mjjj_Sph4_bjet_pt_njet_diag_MCmatch[b][i][k][j]->Sumw2();



	Ntrip_bjet_pt_njet_diag_MCmatch[b][i].push_back(std::vector<TH1F*> ());
        sprintf(hNAME, "Ntrip_bjet%i_pt%i_diag%i_GE%ijet_MCmatch", b,iPt,iDiag,iNjet);
	Ntrip_bjet_pt_njet_diag_MCmatch[b][i][k].push_back(new TH1F(hNAME,hNAME,20,0,20));
        Ntrip_bjet_pt_njet_diag_MCmatch[b][i][k][j]->Sumw2();

	Ntrip_bjet_pt_njet_diag_MCcomb[b][i].push_back(std::vector<TH1F*> ());
        sprintf(hNAME, "Ntrip_bjet%i_pt%i_diag%i_GE%ijet_MCcomb", b,iPt,iDiag,iNjet);
	Ntrip_bjet_pt_njet_diag_MCcomb[b][i][k].push_back(new TH1F(hNAME,hNAME,20,0,20));
        Ntrip_bjet_pt_njet_diag_MCcomb[b][i][k][j]->Sumw2();



	Ntrip_btag_bjet_pt_njet_diag[b][i].push_back(std::vector<TH1F*> ());
        sprintf(hNAME, "Ntrip_btag%i_bjet%i_pt%i_diag%i_GE%ijet", b,b,iPt,iDiag,iNjet);
	Ntrip_btag_bjet_pt_njet_diag[b][i][k].push_back(new TH1F(hNAME,hNAME,20,0,20));
        Ntrip_btag_bjet_pt_njet_diag[b][i][k][j]->Sumw2();

	Ntrip_Sph4_btag_bjet_pt_njet_diag[b][i].push_back(std::vector<TH1F*> ());
        sprintf(hNAME, "Ntrip_Sph4_btag%i_bjet%i_pt%i_diag%i_GE%ijet", b,b,iPt,iDiag,iNjet);
	Ntrip_Sph4_btag_bjet_pt_njet_diag[b][i][k].push_back(new TH1F(hNAME,hNAME,20,0,20));
        Ntrip_Sph4_btag_bjet_pt_njet_diag[b][i][k][j]->Sumw2();

	MjjjSym_bjet_pt_njet_diag_MCmatch[b][i].push_back(std::vector<TH1F*> ());
        sprintf(hNAME, "MjjjSym_bjet%i_pt%i_diag%i_GE%ijet_MCmatch", b,iPt,iDiag,iNjet);
	MjjjSym_bjet_pt_njet_diag_MCmatch[b][i][k].push_back(new TH1F(hNAME,hNAME,100,0,1));
        MjjjSym_bjet_pt_njet_diag_MCmatch[b][i][k][j]->Sumw2();

	MjjjSym_bjet_pt_njet_diag_MCcomb[b][i].push_back(std::vector<TH1F*> ());
        sprintf(hNAME, "MjjjSym_bjet%i_pt%i_diag%i_GE%ijet_MCcomb", b,iPt,iDiag,iNjet);
	MjjjSym_bjet_pt_njet_diag_MCcomb[b][i][k].push_back(new TH1F(hNAME,hNAME,100,0,1));
        MjjjSym_bjet_pt_njet_diag_MCcomb[b][i][k][j]->Sumw2();


	P_bjet_pt_njet_diag_MCmatch[b][i].push_back(std::vector<TH1F*> ());
        sprintf(hNAME, "P_bjet%i_pt%i_diag%i_GE%ijet_MCmatch", b,iPt,iDiag,iNjet);
	P_bjet_pt_njet_diag_MCmatch[b][i][k].push_back(new TH1F(hNAME,hNAME,200,0,20));
        P_bjet_pt_njet_diag_MCmatch[b][i][k][j]->Sumw2();

	P_bjet_pt_njet_diag_MCcomb[b][i].push_back(std::vector<TH1F*> ());
        sprintf(hNAME, "P_bjet%i_pt%i_diag%i_GE%ijet_MCcomb", b,iPt,iDiag,iNjet);
	P_bjet_pt_njet_diag_MCcomb[b][i][k].push_back(new TH1F(hNAME,hNAME,200,0,20));
        P_bjet_pt_njet_diag_MCcomb[b][i][k][j]->Sumw2();

       h31_bjet_pt_njet_diag_MCmatch[b][i].push_back(std::vector<TH1F*> ());
        sprintf(hNAME, "h31_bjet%i_pt%i_diag%i_GE%ijet_MCmatch", b,iPt,iDiag,iNjet);
	h31_bjet_pt_njet_diag_MCmatch[b][i][k].push_back(new TH1F(hNAME,hNAME,100,0,1));
       h31_bjet_pt_njet_diag_MCmatch[b][i][k][j]->Sumw2();

	h31_bjet_pt_njet_diag_MCcomb[b][i].push_back(std::vector<TH1F*> ());
        sprintf(hNAME, "h31_bjet%i_pt%i_diag%i_GE%ijet_MCcomb", b,iPt,iDiag,iNjet);
	h31_bjet_pt_njet_diag_MCcomb[b][i][k].push_back(new TH1F(hNAME,hNAME,100,0,1));
        h31_bjet_pt_njet_diag_MCcomb[b][i][k][j]->Sumw2();


	Maxtr_bjet_pt_njet_diag_MCmatch[b][i].push_back(std::vector<TH1F*> ());
        sprintf(hNAME, "Maxtr_bjet%i_pt%i_diag%i_GE%ijet_MCmatch", b,iPt,iDiag,iNjet);
	Maxtr_bjet_pt_njet_diag_MCmatch[b][i][k].push_back(new TH1F(hNAME,hNAME,200,-2,2));
        Maxtr_bjet_pt_njet_diag_MCmatch[b][i][k][j]->Sumw2();

	Maxtr_bjet_pt_njet_diag_MCcomb[b][i].push_back(std::vector<TH1F*> ());
        sprintf(hNAME, "Maxtr_bjet%i_pt%i_diag%i_GE%ijet_MCcomb", b,iPt,iDiag,iNjet);
	Maxtr_bjet_pt_njet_diag_MCcomb[b][i][k].push_back(new TH1F(hNAME,hNAME,200,-2,2));
        Maxtr_bjet_pt_njet_diag_MCcomb[b][i][k][j]->Sumw2();

	MET_bjet_pt_njet_diag[b][i].push_back(std::vector<TH1F*> ());
        sprintf(hNAME, "MET_bjet%i_pt%i_diag%i_GE%ijet", b,iPt,iDiag, iNjet);
	        MET_bjet_pt_njet_diag[b][i][k].push_back(new TH1F(hNAME,hNAME,500,0,1000));
        MET_bjet_pt_njet_diag[b][i][k][j]->Sumw2();

	HT_bjet_pt_njet_diag[b][i].push_back(std::vector<TH1F*> ());
        sprintf(hNAME, "HT_bjet%i_pt%i_diag%i_GE%ijet", b,iPt,iDiag, iNjet);
	HT_bjet_pt_njet_diag[b][i][k].push_back(new TH1F(hNAME,hNAME,500,0,4000));
        HT_bjet_pt_njet_diag[b][i][k][j]->Sumw2();

	HT_btag_bjet_pt_njet_diag[b][i].push_back(std::vector<TH1F*> ());
        sprintf(hNAME, "HT_btag%i_bjet%i_pt%i_diag%i_GE%ijet", b,b,iPt,iDiag, iNjet);
	HT_btag_bjet_pt_njet_diag[b][i][k].push_back(new TH1F(hNAME,hNAME,500,0,4000));
        HT_btag_bjet_pt_njet_diag[b][i][k][j]->Sumw2();

	HT_Sph4_btag_bjet_pt_njet_diag[b][i].push_back(std::vector<TH1F*> ());
        sprintf(hNAME, "HT_Sph4_btag%i_bjet%i_pt%i_diag%i_GE%ijet", b,b,iPt,iDiag, iNjet);
	HT_Sph4_btag_bjet_pt_njet_diag[b][i][k].push_back(new TH1F(hNAME,hNAME,500,0,4000));
        HT_Sph4_btag_bjet_pt_njet_diag[b][i][k][j]->Sumw2();




	P_bjet_pt_njet_diag[b][i].push_back(std::vector<TH1F*> ());
        sprintf(hNAME, "P_bjet%i_pt%i_diag%i_GE%ijet", b,iPt,iDiag, iNjet);
	P_bjet_pt_njet_diag[b][i][k].push_back(new TH1F(hNAME,hNAME,200,0,20));
        P_bjet_pt_njet_diag[b][i][k][j]->Sumw2();

	MET_over_HT_bjet_pt_njet_diag[b][i].push_back(std::vector<TH1F*> ());
        sprintf(hNAME, "MET_over_HT_bjet%i_pt%i_diag%i_GE%ijet", b,iPt,iDiag, iNjet);
	        MET_over_HT_bjet_pt_njet_diag[b][i][k].push_back(new TH1F(hNAME,hNAME,500,0,1));
        MET_over_HT_bjet_pt_njet_diag[b][i][k][j]->Sumw2();

	HMT_bjet_pt_njet_diag[b][i].push_back(std::vector<TH1F*> ());
        sprintf(hNAME, "HMT_bjet%i_pt%i_diag%i_GE%ijet", b,iPt,iDiag, iNjet);
	        HMT_bjet_pt_njet_diag[b][i][k].push_back(new TH1F(hNAME,hNAME,500,0,4000));
        HMT_bjet_pt_njet_diag[b][i][k][j]->Sumw2();

	nJet35_bjet_pt_njet_diag[b][i].push_back(std::vector<TH1F*> ());
        sprintf(hNAME, "nJet35_bjet%i_pt%i_diag%i_GE%ijet", b,iPt,iDiag, iNjet);
	        nJet35_bjet_pt_njet_diag[b][i][k].push_back(new TH1F(hNAME,hNAME,30,0,30));
        nJet35_bjet_pt_njet_diag[b][i][k][j]->Sumw2();
	
	nBJet35_bjet_pt_njet_diag[b][i].push_back(std::vector<TH1F*> ());
        sprintf(hNAME, "nBJet35_bjet%i_pt%i_diag%i_GE%ijet", b,iPt,iDiag, iNjet);
	        nBJet35_bjet_pt_njet_diag[b][i][k].push_back(new TH1F(hNAME,hNAME,30,0,30));
        nBJet35_bjet_pt_njet_diag[b][i][k][j]->Sumw2();

	nVertex_bjet_pt_njet_diag[b][i].push_back(std::vector<TH1F*> ());
        sprintf(hNAME, "nVertex_bjet%i_pt%i_diag%i_GE%ijet", b,iPt,iDiag, iNjet);
	        nVertex_bjet_pt_njet_diag[b][i][k].push_back(new TH1F(hNAME,hNAME,50,0,50));
        nVertex_bjet_pt_njet_diag[b][i][k][j]->Sumw2();

	Jet0_bjet_pt_njet_diag[b][i].push_back(std::vector<TH1F*> ());
        sprintf(hNAME, "Jet0_bjet%i_pt%i_diag%i_GE%ijet", b,iPt,iDiag, iNjet);
	        Jet0_bjet_pt_njet_diag[b][i][k].push_back(new TH1F(hNAME,hNAME,1000,0,2000));
        Jet0_bjet_pt_njet_diag[b][i][k][j]->Sumw2();
	
	Jet1_bjet_pt_njet_diag[b][i].push_back(std::vector<TH1F*> ());
        sprintf(hNAME, "Jet1_bjet%i_pt%i_diag%i_GE%ijet", b,iPt,iDiag, iNjet);
	        Jet1_bjet_pt_njet_diag[b][i][k].push_back(new TH1F(hNAME,hNAME,1000,0,2000));
        Jet1_bjet_pt_njet_diag[b][i][k][j]->Sumw2();
	
	Jet2_bjet_pt_njet_diag[b][i].push_back(std::vector<TH1F*> ());
        sprintf(hNAME, "Jet2_bjet%i_pt%i_diag%i_GE%ijet", b,iPt,iDiag, iNjet);
	        Jet2_bjet_pt_njet_diag[b][i][k].push_back(new TH1F(hNAME,hNAME,1000,0,2000));
        Jet2_bjet_pt_njet_diag[b][i][k][j]->Sumw2();
	
	Jet3_bjet_pt_njet_diag[b][i].push_back(std::vector<TH1F*> ());
        sprintf(hNAME, "Jet3_bjet%i_pt%i_diag%i_GE%ijet", b,iPt,iDiag, iNjet);
	        Jet3_bjet_pt_njet_diag[b][i][k].push_back(new TH1F(hNAME,hNAME,1000,0,2000));
        Jet3_bjet_pt_njet_diag[b][i][k][j]->Sumw2();
	
	Jet4_bjet_pt_njet_diag[b][i].push_back(std::vector<TH1F*> ());
        sprintf(hNAME, "Jet4_bjet%i_pt%i_diag%i_GE%ijet", b,iPt,iDiag, iNjet);
	        Jet4_bjet_pt_njet_diag[b][i][k].push_back(new TH1F(hNAME,hNAME,1000,0,2000));
        Jet4_bjet_pt_njet_diag[b][i][k][j]->Sumw2();

	Jet5_bjet_pt_njet_diag[b][i].push_back(std::vector<TH1F*> ());
        sprintf(hNAME, "Jet5_bjet%i_pt%i_diag%i_GE%ijet", b,iPt,iDiag, iNjet);
	        Jet5_bjet_pt_njet_diag[b][i][k].push_back(new TH1F(hNAME,hNAME,1000,0,2000));
        Jet5_bjet_pt_njet_diag[b][i][k][j]->Sumw2();

	Jet6_bjet_pt_njet_diag[b][i].push_back(std::vector<TH1F*> ());
        sprintf(hNAME, "Jet6_bjet%i_pt%i_diag%i_GE%ijet", b,iPt,iDiag, iNjet);
	        Jet6_bjet_pt_njet_diag[b][i][k].push_back(new TH1F(hNAME,hNAME,1000,0,2000));
        Jet6_bjet_pt_njet_diag[b][i][k][j]->Sumw2();

	BJet1_bjet_pt_njet_diag[b][i].push_back(std::vector<TH1F*> ());
        sprintf(hNAME, "BJet1_bjet%i_pt%i_diag%i_GE%ijet", b,iPt,iDiag, iNjet);
	        BJet1_bjet_pt_njet_diag[b][i][k].push_back(new TH1F(hNAME,hNAME,1000,0,2000));
        BJet1_bjet_pt_njet_diag[b][i][k][j]->Sumw2();

	BJet2_bjet_pt_njet_diag[b][i].push_back(std::vector<TH1F*> ());
        sprintf(hNAME, "BJet2_bjet%i_pt%i_diag%i_GE%ijet", b,iPt,iDiag, iNjet);
	        BJet2_bjet_pt_njet_diag[b][i][k].push_back(new TH1F(hNAME,hNAME,1000,0,2000));
        BJet2_bjet_pt_njet_diag[b][i][k][j]->Sumw2();

	BJet3_bjet_pt_njet_diag[b][i].push_back(std::vector<TH1F*> ());
        sprintf(hNAME, "BJet3_bjet%i_pt%i_diag%i_GE%ijet", b,iPt,iDiag, iNjet);
	        BJet3_bjet_pt_njet_diag[b][i][k].push_back(new TH1F(hNAME,hNAME,1000,0,2000));
        BJet3_bjet_pt_njet_diag[b][i][k][j]->Sumw2();


      }
    }
  }
  }
  cout<<"after histos"<<endl;
 return;
}
 
void NtpThreeJet::WriteHistograms()
{
	cout<<"made it here"<<endl;
  char FOLDER[100];
  fOutFile->cd();

   fOutFile->mkdir("Events");
   fOutFile->cd("Events");
   h_PossibleTrigger->Write();

   h_ept->Write();
   h_mpt->Write();
   h_mPFIso->Write();
   h_mindRMuonJet->Write();
   h_mindRMuonJet_mPFIso->Write();
   h_MET->Write();
   h_nTruePileUp->Write();
   h_nTruePileUp_NoWeight->Write();
   h_nVertex->Write();
   h_nBJet35->Write();
   h_nJet35->Write();
   h_HT->Write();
   h_HMT->Write();
   h_Jet0->Write();
   h_Jet1->Write();
   h_Jet2->Write();
   h_Jet3->Write();
   h_Jet4->Write();
   h_Jet5->Write();
   h_Jet6->Write();
   h_BJet1->Write();
   h_BJet2->Write();
   h_BJet3->Write();
   h_NeutralHad_JetPt->Write();
   fOutFile->mkdir("Trigger");
   fOutFile->cd("Trigger");

   TDirectory* triplets=fOutFile->mkdir("Triplets");
   fOutFile->cd("Triplets");
   h_MET_EvtSel->Write();
   h_nBJet35_EvtSel->Write();
   h_nJet35_EvtSel->Write();
   h_HT_EvtSel->Write();
   h_MJ_EvtSel->Write();
   h_HMT_EvtSel->Write();
   h_Jet0_EvtSel->Write();
   h_Jet1_EvtSel->Write();
   h_Jet2_EvtSel->Write();
   h_Jet3_EvtSel->Write();
   h_Jet4_EvtSel->Write();
   h_Jet5_EvtSel->Write();
   h_Jet6_EvtSel->Write();
   h_BJet1_EvtSel->Write();
   h_BJet2_EvtSel->Write();
   h_BJet3_EvtSel->Write();
   // for (int b=0; b<5; b++){
   for(int b=0; b<2; b++){
     sprintf(FOLDER, "bjet_%i", b);
     TDirectory* now=triplets->mkdir(FOLDER);
     triplets->cd(FOLDER);
       // for (int i=0; i<11; i++){
	   for (int i=3; i<9; i++) {
	 sprintf(FOLDER, "jetpt_%i", i*10+30);
	 TDirectory* now2=now->mkdir(FOLDER);
	 now->cd(FOLDER);
      // for(int k=0; k<3; k++){
	 for (int k=0; k<1; k++){
	   Mjjj_sumpt_bjet_pt_njet[b][i][k]->Write();
	   Mjjj_vecpt_bjet_pt_njet[b][i][k]->Write();
	   Mjjj_P_bjet_pt_njet[b][i][k]->Write();
	   DeltaM_bjet_pt_njet[b][i][k]->Write();
	
	   Aplanarity_bjet_pt_njet[b][i][k]->Write();
	   Sphericity_bjet_pt_njet[b][i][k]->Write();
	   Circularity_bjet_pt_njet[b][i][k]->Write();
	   Isotropy_bjet_pt_njet[b][i][k]->Write();
	   C_bjet_pt_njet[b][i][k]->Write();
	   D_bjet_pt_njet[b][i][k]->Write();
	   
	   Mjjj_sumpt_btag_pt_njet[b][i][k]->Write();
	   // for(int j=0; j<25; j++){ 
	  for(int j=7; j<8; j++){
	     /*	     sprintf(FOLDER, "diagcut_%i_%i",k,j*10+40);
	     now2->mkdir(FOLDER);
	     now2->cd(FOLDER);*/
	     Mjjj_bjet_pt_njet_diag[b][i][k][j]->Write();  
	     Mjjj_btag_bjet_pt_njet_diag[b][i][k][j]->Write();  
	     Mjjj_btag_bjet_pt_njet_diag_1trp[b][i][k][j]->Write();  

	     Mjjj_Sph4_bjet_pt_njet_diag[b][i][k][j]->Write();  
	     Mjjj_Sph4_bjet_pt_njet_diag_1trp[b][i][k][j]->Write();  
	     Mjjj_Sph4_btag_bjet_pt_njet_diag[b][i][k][j]->Write();  
	     Mjjj_Sph4_btag_bjet_pt_njet_diag_1trp[b][i][k][j]->Write();  
	     Mjjj_btag_bjet_trpwt_pt_njet_diag[b][i][k][j]->Write();  
	     Mjjj_Sph4_btag_bjet_trpwt_pt_njet_diag[b][i][k][j]->Write();  

	     Mjjj_bjet_pt_njet_diag_MCmatch[b][i][k][j]->Write();  
	     Mjjj_bjet_pt_njet_diag_MCcomb[b][i][k][j]->Write();  

	     // Mjjj_Sph4_bjet_pt_njet_diag_MCmatch[b][i][k][j]->Write();  
	     //Mjjj_Sph4_bjet_pt_njet_diag_MCcomb[b][i][k][j]->Write();  

	     Ntrip_bjet_pt_njet_diag_MCmatch[b][i][k][j]->Write();  
	     Ntrip_bjet_pt_njet_diag_MCcomb[b][i][k][j]->Write();  

	     Ntrip_btag_bjet_pt_njet_diag[b][i][k][j]->Write();  
	     Ntrip_Sph4_btag_bjet_pt_njet_diag[b][i][k][j]->Write();  

	     
	     bool WriteFull=true;
	     if(WriteFull){
	     MjjjSym_bjet_pt_njet_diag[b][i][k][j]->Write();  
	     MjjjSym_bjet_pt_njet_diag_MCmatch[b][i][k][j]->Write();  
	     MjjjSym_bjet_pt_njet_diag_MCcomb[b][i][k][j]->Write();  

	     P_bjet_pt_njet_diag_MCmatch[b][i][k][j]->Write();  
	     P_bjet_pt_njet_diag_MCcomb[b][i][k][j]->Write();  

	     h31_bjet_pt_njet_diag_MCmatch[b][i][k][j]->Write();  
	     h31_bjet_pt_njet_diag_MCcomb[b][i][k][j]->Write();  

	     Maxtr_bjet_pt_njet_diag_MCmatch[b][i][k][j]->Write();  
	     Maxtr_bjet_pt_njet_diag_MCcomb[b][i][k][j]->Write();  
	     MET_bjet_pt_njet_diag[b][i][k][j]->Write();
	     HT_bjet_pt_njet_diag[b][i][k][j]->Write();
	     HT_btag_bjet_pt_njet_diag[b][i][k][j]->Write();
	     HT_Sph4_btag_bjet_pt_njet_diag[b][i][k][j]->Write();
	     P_bjet_pt_njet_diag[b][i][k][j]->Write();
	     MET_over_HT_bjet_pt_njet_diag[b][i][k][j]->Write();
	     HMT_bjet_pt_njet_diag[b][i][k][j]->Write();
	     nJet35_bjet_pt_njet_diag[b][i][k][j]->Write();
	     nBJet35_bjet_pt_njet_diag[b][i][k][j]->Write();
	     nVertex_bjet_pt_njet_diag[b][i][k][j]->Write();
	     Jet0_bjet_pt_njet_diag[b][i][k][j]->Write();
	     Jet1_bjet_pt_njet_diag[b][i][k][j]->Write();
	     Jet2_bjet_pt_njet_diag[b][i][k][j]->Write();
	     Jet3_bjet_pt_njet_diag[b][i][k][j]->Write();
	     Jet4_bjet_pt_njet_diag[b][i][k][j]->Write();
	     Jet5_bjet_pt_njet_diag[b][i][k][j]->Write();
	     Jet6_bjet_pt_njet_diag[b][i][k][j]->Write();
	     BJet1_bjet_pt_njet_diag[b][i][k][j]->Write();
	     BJet2_bjet_pt_njet_diag[b][i][k][j]->Write();
	     BJet3_bjet_pt_njet_diag[b][i][k][j]->Write();
	     }
	   }
	 }
       }
   }
   
  cout << "end of writing\n";
}

std::vector<math::XYZVector> makeVecForEventShape(std::vector<JetLV* > jets) {
  std::vector<math::XYZVector> p;
  for(unsigned int i=0; i< jets.size(); i++){
    math::XYZVector Vjet;
    Vjet = math::XYZVector(jets[i]->Px(), jets[i]->Py(), jets[i]->Pz());
   
    p.push_back(Vjet);
  }
  
  return p;
}


static void fill1trpHist(TH1F *const stdHist, float mjjj, double evtWt,
	TH1F *const hist1trp, std::map<int, int> &binList)
{
	Int_t binNum = stdHist->FindBin(mjjj);
	if (binNum >= 0 && binList.count(binNum) == 0) {
		hist1trp->Fill(mjjj, evtWt);
		binList[binNum] = 1;
	}
}


void NtpThreeJet::Loop ()
{
  
  /////////////////IMPORTANT DEFINE SOME CUTS//////////////////
  /////////////////////////////////////////////////////////////
  int njetsMin=6;
  bool DoPileUpReweight=true;
  bool DoBtagSF=true;  
  ////////////////////////////////////////////////////////////
  
  // Run over all entries and do what you like!
  int countallhad=0;
  int countlep=0;
  int countsemilep=0;
  
  //Initialize the lumi reweighting
  reweight::LumiReWeighting LumiWeights_;  
  std::vector< float > DataJun01 ;
  std::vector< float > Summer2012;
  
  // Distribution used for S10 Summer2012 MC.
  float Summer2012_f [60] = {2.560E-06, 5.239E-06, 1.420E-05, 5.005E-05, 1.001E-04, 2.705E-04, 1.999E-03, 
			     6.097E-03, 1.046E-02, 1.383E-02, 1.685E-02, 2.055E-02, 2.572E-02, 3.262E-02, 4.121E-02, 
			     4.977E-02, 5.539E-02, 5.725E-02, 5.607E-02, 5.312E-02, 5.008E-02, 4.763E-02, 4.558E-02, 
			     4.363E-02, 4.159E-02, 3.933E-02, 3.681E-02, 3.406E-02, 3.116E-02, 2.818E-02, 2.519E-02,
			     2.226E-02, 1.946E-02, 1.682E-02, 1.437E-02, 1.215E-02, 1.016E-02, 8.400E-03, 6.873E-03, 
			     5.564E-03, 4.457E-03, 3.533E-03, 2.772E-03, 2.154E-03, 1.656E-03, 1.261E-03, 9.513E-04, 
			     7.107E-04, 5.259E-04, 3.856E-04, 2.801E-04, 2.017E-04, 1.439E-04, 1.017E-04, 7.126E-05, 
			     4.948E-05, 3.405E-05, 2.322E-05, 1.570E-05, 5.005E-06};

  float data_f[60]={0,8.66979e-07,1.99567e-06,7.0944e-06,0.00115866,0.00304481,9.37811e-05,0.000438788,0.00369376,
		    0.0132685,0.0250747,0.035147,0.0466437,0.0612746,0.0752654,0.0879549,0.0977272,0.0971447,
		    0.0854517,0.0699568,0.0561969,0.0454351,0.0372194,0.0307875,0.0256272,0.0213495,0.0176728,
		    0.014462,0.0116675,0.00926419,0.00722899,0.00553611,0.00415629,0.0030565,0.00220048,0.00155038,
		    0.00106882,0.00072095,0.000475842,0.000307359,0.000194338,0.000120318,7.29651e-05,4.33587e-05,
		    2.52569e-05,1.44273e-05,8.08416e-06,4.44472e-06,2.39826e-06,1.27011e-06,0.00018054};
  
  TH1D *puhist = (TH1D *) gDirectory->Get("pileup");
  if (puhist) {
    double numEvts = puhist->Integral();
    if (numEvts > 0)
      puhist->Scale(1.0/numEvts);
  }
  for( int z=0; z<50; ++z) {
    Summer2012.push_back(Summer2012_f[z]);
    double dataVal = data_f[z];
    if (puhist)
      dataVal =  puhist->GetBinContent(z + 1);
    DataJun01.push_back(dataVal);
  }
  cout << endl;
  //actually calculate the weights
  LumiWeights_ = reweight::LumiReWeighting(Summer2012,DataJun01);
  
  //initialize some quantities needed
  std::vector<TLorentzVector* >      fBJets;
  std::vector<TLorentzVector* >      fNoBJets;
  std::vector<JetLV* >      	       fCleanJets;
  std::vector<JetLV* >      	       fClean6Jets;
  std::vector<TLorentzVector* >      fTestJets;
  std::vector<TLorentzVector* >      fCleanJets20;  
  
  vector<TLorentzVector* > fdummyCleanJets;  
  
  std::vector<int >   JetMoms;
  std::vector<int >   TripletMoms;
  std::vector<float >   sumScalarPtTriplet;
  std::vector<float >   massTriplet;
  std::vector<float >   Maxtr;
  
  std::vector<float >   sumScalarPtTripletComp;
  std::vector<float >   massTripletComp;
  std::map<unsigned int, std::vector<float> > massTripletPassBtag;
  std::vector<float >   PTriplet;
  std::vector<float >   h31Triplet;
  
  std::vector<float >   massDoublet12;
  std::vector<float >   massDoublet13;
  std::vector<float >   massDoublet23;
  
  std::vector<float >   massDoubletHigh;
  std::vector<float >   massDoubletMid;
  std::vector<float >   massDoubletLow;
  
  std::vector<float >   sumVectorPtTriplet;
  std::vector <std::vector<JetLV* > > Triplet;
  
  TLorentzVector* dummyJet= new TLorentzVector (0,0,0,0);
  int notrig = 0;
  ////////////////////////////////////////////////////////////////
  //////////////////////////EVENT LOOP////////////////////////////
  ////////////////////////////////////////////////////////////////
  for (int ientry = 0; GetEntry(ientry) > 0; ++ientry) {
    //  cout<<"-----------------------------------------"<<endl;
    //do pile up reweighting if we use MonteCarlo
    double MyWeight = LumiWeights_.weight(nTruePileUp);
    double weight=1;
    if(!DataIs) weight = MyWeight;

    ///////////////////Clear out variables/////////////////////
    while (fCleanJets20.size() > 0) { // Reclaim some used memory
    	TLorentzVector *oldjet = fCleanJets20.back();
    	if (oldjet != NULL)
    		delete oldjet;
    	fCleanJets20.pop_back();
    }
    Triplet.clear();    sumScalarPtTriplet.clear();  sumVectorPtTriplet.clear(); massTriplet.clear();
    PTriplet.clear();    h31Triplet.clear();    Maxtr.clear();
    fBJets.clear(); fNoBJets.clear();fCleanJets.clear(); fClean6Jets.clear();    
    fCleanJets20.clear(); fTestJets.clear();     fdummyCleanJets.clear();
    massDoublet12.clear();  massDoublet13.clear(); massDoublet23.clear();
    massDoubletHigh.clear();  massDoubletMid.clear();  massDoubletLow.clear();
    JetMoms.clear(); TripletMoms.clear();
    ////////////////////////////////////////////////////////////
    
    //count something for every 100 events
    if (ientry % 100 == 0) {
      printf("\n=================Processing entry: %i\n", ientry);
    }
    //can be used to run SUSY scans, then change the If statement
    if(1==1){//MSquark == 375 && MLSP ==75)
      
      ////////JETS///////
      //Count all the jets above 35 Gev, also calculated HT=SumptAllJet, count number of b-jets
      int nJet20=0; int nJet35=0; int nBJet35=0; int nNoBJet35=0;       
      float SumptAllJet=0;
      float MassAllJet=0;
      float SumptAllJet20=0;
      float SumMetPtAllJet=0;
      float SumMetPtAllJet20=0;
      
      int dummycounter=0;     
      //cout<<"Shouldn't be anyting "<<fCleanJets.size()<<" "<<nPFJets<<" "<<sizeof(jet_PF_pt)<<endl;
      for (int i=0; i<nPFJets; i++){
	//cout<<i<<". th jet: "<<jet_PF_pt[i]<<" eta: "<< fabs(jet_PF_eta[i])<<endl;
	JetLV* Jet=new JetLV(jet_PF_px[i],jet_PF_py[i],jet_PF_pz[i],jet_PF_e[i]);
	if (jet_PF_pt[i]>20.0 && fabs(jet_PF_eta[i])<2.5){
	  bool isTagged = false;
	  //CSVL > 0.244, CSVM > 0.679, CSVT > 0.898, JPM > 
	  if (bdiscCSV_PF[i] >  0.679)
	  isTagged = true;
	  //if (bdiscJP_PF[i] >  0.545)
	  //isTagged = true;

 //implementing b-tagging scale factors
	  int jet_flavor =jet_PF_PartonFlav[i];
	  float jet_pt = jet_PF_pt[i];
	  float jet_phi = jet_PF_phi[i];
	  float jet_eta = jet_PF_eta[i];
	  //apply upgrading/downgrading for MC
	  
	  if(!DataIs){
	  if(DoBtagSF){
	    
	    double phi = jet_phi;
	    double sin_phi = sin(phi*1000000);
	    double seed = abs(static_cast<int>(sin_phi*100000));
	    //Initialize class   
	    BTagSFUtil* btsfutil = new BTagSFUtil( seed );
	    bool temp=isTagged;
	    //For uncertainties use BtagSys=-1,1 
	    int BtagSys =0;
	    
	    float Btag_SF =  GetBTagSF(jet_pt,jet_eta,BtagSys*1);
	    float Btag_eff =  h2_EffMapB->GetBinContent( h2_EffMapB->GetXaxis()->FindBin(jet_pt), h2_EffMapB->GetYaxis()->FindBin(fabs(jet_eta)) );
	    
	    //For uncertainties use BtagSys=-2,2 double uncertainties for c jets                                         
	    float Ctag_SF =  GetBTagSF(jet_pt,jet_eta,BtagSys*2);
	    float Ctag_eff = h2_EffMapC->GetBinContent( h2_EffMapC->GetXaxis()->FindBin(jet_pt), h2_EffMapC->GetYaxis()->FindBin(fabs(jet_eta)) );
	    
	    float UDSGtag_SF = GetLightJetSF(jet_pt,jet_eta,BtagSys*1);
	    float UDSGtag_eff = h2_EffMapUDSG->GetBinContent( h2_EffMapUDSG->GetXaxis()->FindBin(jet_pt), h2_EffMapUDSG->GetYaxis()->FindBin(fabs(jet_eta)) );
	    
	    btsfutil->modifyBTagsWithSF(isTagged, jet_flavor, Btag_SF,Btag_eff,Ctag_SF,Ctag_eff,UDSGtag_SF,UDSGtag_eff);
	    //if (temp != isTagged)
	    // cout<< endl << "GOT ONE!!!!!!!!!!!: "<<temp<<"  "<<isTagged<<" "<<jet_flavor<<"  "<<bdiscCSV_PF[i]<<endl;
	    delete btsfutil;
	  }
	  }
	  Jet->setBtag(isTagged);
	  nJet20++;
	  fCleanJets20.push_back(Jet);
	  SumptAllJet20=SumptAllJet20+jet_PF_pt[i];
	  if(jet_PF_pt[i]>35.0){
	    SumptAllJet=SumptAllJet+jet_PF_pt[i];
	    MassAllJet=MassAllJet+jet_PF_mass[i];
	    fCleanJets.push_back(Jet);
	    h_NeutralHad_JetPt->Fill(jet_PF_pt[i],jet_PF_NeutralHad[i],weight);
	    JetMoms.push_back(jet_PF_JetMom[i]);
	    //	cout<<"JetMomFromTree: "<<jet_PF_JetMom[i]<<endl;
	    //cout<<"JetMomFromTree_InArray: "<<JetMoms[nJet35]<<endl;
	    
	    nJet35++;
	    dummycounter++;
	    if (isTagged)
	      {
		nBJet35++;
		fBJets.push_back(Jet);
		fdummyCleanJets.push_back(dummyJet);
	      }
	    if (!isTagged)
	      {
		nNoBJet35++;
		fNoBJets.push_back(Jet);
		fdummyCleanJets.push_back(Jet);
		
	      }
	    }
	  }
	
	}
	
	
        SumMetPtAllJet = pfMET + SumptAllJet;
	SumMetPtAllJet20 = pfMET + SumptAllJet20;
      

	//MUON/////
	//make some plots for the muons
	
	for (int i=0; i<nMuons; i++){
	  TLorentzVector Muon(mpx[i],mpy[i],mpz[i],me[i]);
	  //lets look at the leading muon for now
	  if(i==0){
	    h_mPFIso->Fill(mPFIso[i],weight);
	    h_mpt->Fill(Muon.Pt(),weight);
	    //calcualte the minimum dR to one of the jets
	    float mindRMuonJet=9999;
	    for (int j=0; j<nJet35; j++){
	      TLorentzVector Jet1(fCleanJets[j]->Px(),fCleanJets[j]->Py(),fCleanJets[j]->Pz(),fCleanJets[j]->E());
	      float dRMuonJet=Jet1.DeltaR(Muon);
	      if (dRMuonJet < mindRMuonJet) mindRMuonJet=dRMuonJet;
	      //	 cout<<"All: "<<dRMuonJet<<endl;
	      
	    }
	    //cout<<mindRMuonJet<<endl;
	    h_mindRMuonJet->Fill(mindRMuonJet,weight);
	    
	    h_mindRMuonJet_mPFIso->Fill(mPFIso[i],mindRMuonJet,weight);
	  }
	}
	
	//Possible Triggers selections
	
   if (HasSelTrigger){
     h_nBJet35->Fill(nBJet35,weight);
     h_nJet35->Fill(nJet35,weight);
     h_MET->Fill(pfMET,weight);
     h_nTruePileUp->Fill(nTruePileUp,weight);
     h_nTruePileUp_NoWeight->Fill(nTruePileUp);
     h_nVertex->Fill(nGoodVtx,weight);
     h_HT->Fill(SumptAllJet,weight);
     h_HMT->Fill(SumMetPtAllJet,weight);
     if(nBJet35>2){           
       h_BJet1->Fill(fBJets[0]->Pt(),weight);
       h_BJet2->Fill(fBJets[1]->Pt(),weight);
       h_BJet3->Fill(fBJets[2]->Pt(),weight);
     }
     if(nJet35>=1)h_Jet0->Fill(fCleanJets[0]->Pt(),weight);
     if(nJet35>=2)h_Jet1->Fill(fCleanJets[1]->Pt(),weight);
     if(nJet35>=3)h_Jet2->Fill(fCleanJets[2]->Pt(),weight);
     if(nJet35>=4)h_Jet3->Fill(fCleanJets[3]->Pt(),weight);
     if(nJet35>=5)h_Jet4->Fill(fCleanJets[4]->Pt(),weight);
     if(nJet35>=6)h_Jet5->Fill(fCleanJets[5]->Pt(),weight);
     
     
     if ( nJet35>=6){
       //cout<<nJet35<<" "<<fCleanJets.size()<<" s"<<fCleanJets[0]->Pt()<<" "<<fCleanJets[1]->Pt()<<" "<<fCleanJets[2]->Pt()<<" "<<fCleanJets[3]->Pt()<<" "<<fCleanJets[4]->Pt()<<" "<<fCleanJets[5]->Pt()<<endl;
       //       if(fCleanJets[0]->Pt() > 80.0 && fCleanJets[1]->Pt() > 80. && fCleanJets[2]->Pt() > 80.0 && fCleanJets[3]->Pt() > 80.0 && 
       //fCleanJets[4]->Pt() > 60.0 && fCleanJets[5]->Pt() > 60.0)
       
       if(fCleanJets[3]->Pt() > 80 ){
	 //      if(SumptAllJet>900)
	 if(1==1){//nBJet35 >= 3)
	   //GetEvent Shape
	   //EventShape = EventShapeVariables(fCleanJets3Vec);
	   
	   //	   cout<<aplanarity_<<" "<<sphericity_<<" "<<circularity_<<" "<<isotropy_<<" "<<C_<<" "<<D_<<endl;
	   h_nBJet35_EvtSel->Fill(nBJet35,weight);
	   h_nJet35_EvtSel->Fill(nJet35,weight);
	   h_MET_EvtSel->Fill(pfMET,weight);
	
	   /*	   if(pfMET > 50.0)h_HT_EvtSel->Fill(SumptAllJet+pfMET,weight);
	   else if (pfMET <=50.0) h_HT_EvtSel->Fill(SumptAllJet,weight);
	   */
	   h_HT_EvtSel->Fill(SumptAllJet,weight);
	   h_MJ_EvtSel->Fill(MassAllJet,weight);
	   h_HMT_EvtSel->Fill(SumMetPtAllJet,weight);
	   h_Jet0_EvtSel->Fill(fCleanJets[0]->Pt(),weight);
	   h_Jet1_EvtSel->Fill(fCleanJets[1]->Pt(),weight);
	   h_Jet2_EvtSel->Fill(fCleanJets[2]->Pt(),weight);
	   if(nBJet35>2){           
	     h_BJet1_EvtSel->Fill(fBJets[0]->Pt(),weight);
	     h_BJet2_EvtSel->Fill(fBJets[1]->Pt(),weight);
	     h_BJet3_EvtSel->Fill(fBJets[2]->Pt(),weight);
	   }
	   if(nJet35>=4) h_Jet3_EvtSel->Fill(fCleanJets[3]->Pt(),weight);
	   if(nJet35>=5) h_Jet4_EvtSel->Fill(fCleanJets[4]->Pt(),weight);
	   if(nJet35>=6) h_Jet5_EvtSel->Fill(fCleanJets[5]->Pt(),weight);
	   if(nJet35>=7) h_Jet6_EvtSel->Fill(fCleanJets[6]->Pt(),weight);
	   //all the jets make triplets
	   //unsigned int numJetForTriplet=fCleanJets.size();
	   //	   cout<<fCleanJets.size()<<endl;
	   //only the six leading jets make triplets
	    unsigned int numJetForTriplet=6;
	   int nTriplets=0;
	   for (unsigned int i=0+0; i<numJetForTriplet-2; ++i) {
	     for (unsigned int j=i+1; j<numJetForTriplet-1; ++j) {
	       for (unsigned int k=j+1; k<numJetForTriplet-0; ++k) {
		 // cout<<nTriplets<<endl;
		 Triplet.push_back(std::vector<JetLV* > ());
		 
		 JetLV* Jet1; JetLV* Jet2; JetLV* Jet3;
		 Jet1=fCleanJets[i]; Jet2=fCleanJets[j]; Jet3=fCleanJets[k];
		 
		 JetLV* Jet4; JetLV* Jet5; JetLV* Jet6;
		 Jet4=fCleanJets[numJetForTriplet-1-k]; Jet5=fCleanJets[numJetForTriplet-1-j]; Jet6=fCleanJets[numJetForTriplet-1-i];
		 
		 TLorentzVector Triplet123; Triplet123=(*Jet1+*Jet2+*Jet3);
		 TLorentzVector Triplet456; Triplet456=(*Jet4+*Jet5+*Jet6);
		 TVector2 TripletPhiRapidity; TripletPhiRapidity.Set(Triplet123.Rapidity(),Triplet123.Phi());
		 
		 TripletPhiRapidity=TripletPhiRapidity/sqrt(TripletPhiRapidity.X()*TripletPhiRapidity.X()+TripletPhiRapidity.Y()*TripletPhiRapidity.Y());
		 TVector2 Jet1PhiRapidity; Jet1PhiRapidity.Set(Jet1->Rapidity(),Jet1->Phi());
		 Jet1PhiRapidity=Jet1PhiRapidity/sqrt(Jet1PhiRapidity.X()*Jet1PhiRapidity.X()+Jet1PhiRapidity.Y()*Jet1PhiRapidity.Y());
		 
		 TVector2 Jet2PhiRapidity; Jet2PhiRapidity.Set(Jet2->Rapidity(),Jet2->Phi());
		 Jet2PhiRapidity=Jet2PhiRapidity/sqrt(Jet2PhiRapidity.X()*Jet2PhiRapidity.X()+Jet2PhiRapidity.Y()*Jet2PhiRapidity.Y());
		 
		 TVector2 Jet3PhiRapidity; Jet3PhiRapidity.Set(Jet3->Rapidity(),Jet3->Phi());
		 Jet3PhiRapidity=Jet3PhiRapidity/sqrt(Jet3PhiRapidity.X()*Jet3PhiRapidity.X()+Jet3PhiRapidity.Y()*Jet3PhiRapidity.Y());
		 
		 Maxtr.push_back(1/3.0*((TripletPhiRapidity.X()*Jet1PhiRapidity.X()+TripletPhiRapidity.Y()*Jet1PhiRapidity.Y())+(TripletPhiRapidity.X()*Jet2PhiRapidity.X()+TripletPhiRapidity.Y()*Jet2PhiRapidity.Y())+(TripletPhiRapidity.X()*Jet3PhiRapidity.X()+TripletPhiRapidity.Y()*Jet3PhiRapidity.Y())));
		 
		 TLorentzVector Doublet12; Doublet12=*Jet1+*Jet2;
		 TLorentzVector Doublet13; Doublet13=*Jet1+*Jet3;
		 TLorentzVector Doublet23; Doublet23=*Jet2+*Jet3;
		 
		 float fP=Jet1->DeltaR(*Jet2)+Jet2->DeltaR(*Jet3)+Jet1->DeltaR(*Jet3);
		 PTriplet.push_back(fP);
		 
		 sumScalarPtTriplet.push_back(Jet1->Pt()+Jet2->Pt()+Jet3->Pt());
		 massTriplet.push_back(Triplet123.M());
		 h31Triplet.push_back(Jet3->Pt()/Jet1->Pt());
		 
		 sumScalarPtTripletComp.push_back(Jet4->Pt()+Jet5->Pt()+Jet6->Pt());
		 massTripletComp.push_back(Triplet456.M());
		 
		 sumVectorPtTriplet.push_back(Triplet123.Pt());
		 int TripletMomGood=-1;
		 if(JetMoms[i]==-1 || JetMoms[j]==-1 || JetMoms[k]==-1) TripletMomGood=-1;
		 if((JetMoms[i]!=-1 && JetMoms[j]!=-1 && JetMoms[k]!=-1) && (JetMoms[i]==JetMoms[j] && JetMoms[i]==JetMoms[k] && JetMoms[j]==JetMoms[k])) TripletMomGood=JetMoms[i];
		 
		 
		 TripletMoms.push_back(TripletMomGood);
		 //	     cout<<"Combo: "<<i<<" "<<j<<" "<<k<<" JetMoms: "<<JetMoms[i]<<" "<<JetMoms[j]<<" "<<JetMoms[k]<<" TripleMoms: "<<TripletMoms[nTriplets]<<endl;
		 massDoublet12.push_back(Doublet12.M());  
		 massDoublet13.push_back(Doublet13.M()); 
		 massDoublet23.push_back(Doublet23.M());
		 vector<float > dijetmass_3m;
		 
		 dijetmass_3m.push_back(Doublet12.M()*Doublet12.M()/(Triplet123.M()*Triplet123.M()+Jet1->M()*Jet1->M()+Jet2->M()*Jet2->M()+Jet3->M()*Jet3->M()));
		 dijetmass_3m.push_back(Doublet13.M()*Doublet13.M()/(Triplet123.M()*Triplet123.M()+Jet1->M()*Jet1->M()+Jet2->M()*Jet2->M()+Jet3->M()*Jet3->M()));
		 dijetmass_3m.push_back(Doublet23.M()*Doublet23.M()/(Triplet123.M()*Triplet123.M()+Jet1->M()*Jet1->M()+Jet2->M()*Jet2->M()+Jet3->M()*Jet3->M()));
		 sort(dijetmass_3m.begin(),dijetmass_3m.end());
		 
		 
		 
		 massDoubletHigh.push_back(dijetmass_3m[2]);  
		 massDoubletMid.push_back(dijetmass_3m[1]);  
		 massDoubletLow.push_back(dijetmass_3m[0]);
		 
		 Triplet[nTriplets].push_back(Jet1);
		 Triplet[nTriplets].push_back(Jet2);
		 Triplet[nTriplets].push_back(Jet3);
		 
		 nTriplets++;
		 
		 
	       }
	     }
	   }
	   
	   EventShapeVariables eventshape(makeVecForEventShape(fCleanJets));
	   //actually (r) with r=2 for normal and r=1 infrared safe defintion
	   float aplanarity_  = eventshape.aplanarity();
	   float sphericity_  = eventshape.sphericity();
	   float circularity_ = eventshape.circularity();
	   float isotropy_    = eventshape.isotropy();
	   float C_           = eventshape.C();
	   float D_           = eventshape.D();
	   
	   // for(int b=0; b<5; b++){
	   for(int b=0; b<2; b++){
	     
	     // for (int i=3; i<11; i++) {
	     for (int i=3; i<9; i++) {
		 float iPt=30.0+(float)i*10.0;
		 for(int k=0; k<1; k++)
		   { 
		     unsigned int iNjet=k+6;
		     //count njets with the pt cut -> gonna be slow
		     
		     
		     if(iNjet<=fCleanJets.size()){
		       //pt cut is applied to the N (loop) highes jets, all others are 35 GeV
		       if(fCleanJets[iNjet-1]->Pt()>iPt && nBJet35>=b)
			 {
			   Aplanarity_bjet_pt_njet[b][i][k]->Fill(aplanarity_,weight);
			   Sphericity_bjet_pt_njet[b][i][k]->Fill(sphericity_,weight);
			   Circularity_bjet_pt_njet[b][i][k]->Fill(circularity_,weight);
			   Isotropy_bjet_pt_njet[b][i][k]->Fill(isotropy_,weight);
			   C_bjet_pt_njet[b][i][k]->Fill(C_,weight);
			   D_bjet_pt_njet[b][i][k]->Fill(D_,weight);
			   
			   
			   
			   // for(int j=0; j<25; j++){
			   for(int j=7; j<8; j++){
			     int countMatchTrip=0;
			     int countCombTrip=0;
			     int countT=0;
			     massTripletPassBtag.clear();
			     std::map<int, int> noBhiMbins, bjetLoMbins, bjetHiMbins;
			     for(unsigned int q=0; q<massTriplet.size(); q++){  
			       //only fill the following plots once
			       
			       if(Triplet[q][2]->Pt()>iPt){
				 if(j==0){

				     Mjjj_sumpt_bjet_pt_njet[b][i][k]->Fill(sumScalarPtTriplet[q],massTriplet[q],weight);
				     Mjjj_vecpt_bjet_pt_njet[b][i][k]->Fill(sumVectorPtTriplet[q],massTriplet[q],weight);
				     Mjjj_P_bjet_pt_njet[b][i][k]->Fill(PTriplet[q],massTriplet[q],weight);
				     DeltaM_bjet_pt_njet[b][i][k]->Fill(massTriplet[q]-massTripletComp[q],weight);
				     Mjjj_sumpt_btag_pt_njet[b][i][k]->Fill(sumScalarPtTriplet[q],massTriplet[q],weight);
				   
				   if(b >= 1 && (Triplet[q][0]->btagged ||
								  Triplet[q][1]->btagged || Triplet[q][2]->btagged))
				     {	
				       Mjjj_sumpt_btag_pt_njet[b][i][k]->Fill(sumScalarPtTriplet[q],massTriplet[q],weight);
				     }		  
				 }
			       float iDiag=(float)j*10.0+40.0;
			       //to implement the pt cut we only keep triplets where the lowest jet passes pt cut
			       //also we check if the njetsMin cut satisfies this pt cut
			       
			       //cout<<"after selection"<<endl;
			       //cout<<Triplet[q][2].pt()<<" "<<fCleanJets.size()<<" "<<fCleanJets[iNjet-1].pt()<<endl;
			       if(massTriplet[q]<(sumScalarPtTriplet[q]-iDiag))
				 {
				   countT++;
				   float JetMassSym=fabs(massTriplet[q]-massTripletComp[q])/(massTriplet[q]+massTripletComp[q])/2;
				   Mjjj_bjet_pt_njet_diag[b][i][k][j]->Fill(massTriplet[q],weight);
				   if(sphericity_ >= 0.4) {
				     Mjjj_Sph4_bjet_pt_njet_diag[b][i][k][j]->Fill(massTriplet[q],weight);
				     fill1trpHist(Mjjj_Sph4_bjet_pt_njet_diag[b][i][k][j], massTriplet[q], weight, 
				     	 Mjjj_Sph4_bjet_pt_njet_diag_1trp[b][i][k][j], noBhiMbins);
				     /*if (iDiag==110 && Triplet[q][2]->Pt()>60.0){
				       cout<<"Diag: "<<iDiag<<" iPt: "<<Triplet[q][2]->Pt()<<" run: "<<run<<" evt: "<<event<<"lumi :"<<lumis<<endl; 

				       }*/
				   }
				   MjjjSym_bjet_pt_njet_diag[b][i][k][j]->Fill(JetMassSym,weight);
				   P_bjet_pt_njet_diag[b][i][k][j]->Fill(PTriplet[q], weight);	
				   if(b==0){
				     Mjjj_btag_bjet_pt_njet_diag[b][i][k][j]->Fill(massTriplet[q],weight);
				     massTripletPassBtag[b].push_back(massTriplet[q]);
				     if(sphericity_ >= 0.4) {
				     	 Mjjj_Sph4_btag_bjet_pt_njet_diag[b][i][k][j]->Fill(massTriplet[q],weight);

				     }
				   }
				   if (b>=1){
				     if (Triplet[q][0]->btagged || Triplet[q][1]->btagged || Triplet[q][2]->btagged){
				       Mjjj_btag_bjet_pt_njet_diag[b][i][k][j]->Fill(massTriplet[q],weight);
					   fill1trpHist(Mjjj_btag_bjet_pt_njet_diag[b][i][k][j], massTriplet[q], weight, 
				     	 Mjjj_btag_bjet_pt_njet_diag_1trp[b][i][k][j], bjetLoMbins);
				       massTripletPassBtag[b].push_back(massTriplet[q]);
				       if(sphericity_ >= 0.4){
				     	 Mjjj_Sph4_btag_bjet_pt_njet_diag[b][i][k][j]->Fill(massTriplet[q],weight);
				     	 fill1trpHist(Mjjj_Sph4_btag_bjet_pt_njet_diag[b][i][k][j], massTriplet[q], weight, 
				     	 	 Mjjj_Sph4_btag_bjet_pt_njet_diag_1trp[b][i][k][j], bjetHiMbins);

				     }
				     }
				   }
				   
				   if(TripletMoms[q]==0 || TripletMoms[q]==1){
				     Mjjj_bjet_pt_njet_diag_MCmatch[b][i][k][j]->Fill(massTriplet[q],weight);
				     //				     if(sphericity_ >= 0.4)				     Mjjj_Sph4_bjet_pt_njet_diag_MCmatch[b][i][k][j]->Fill(massTriplet[q],weight);
				     MjjjSym_bjet_pt_njet_diag_MCmatch[b][i][k][j]->Fill(JetMassSym,weight);
				     P_bjet_pt_njet_diag_MCmatch[b][i][k][j]->Fill(PTriplet[q],weight);
				     h31_bjet_pt_njet_diag_MCmatch[b][i][k][j]->Fill(h31Triplet[q],weight);
				     Maxtr_bjet_pt_njet_diag_MCmatch[b][i][k][j]->Fill(Maxtr[q],weight);
				     countMatchTrip++;
				   }
				   if(TripletMoms[q]!=0 && TripletMoms[q]!=1){
				     Mjjj_bjet_pt_njet_diag_MCcomb[b][i][k][j]->Fill(massTriplet[q],weight);
				     //if(sphericity_ >= 0.4) Mjjj_Sph4_bjet_pt_njet_diag_MCcomb[b][i][k][j]->Fill(massTriplet[q],weight);
				     MjjjSym_bjet_pt_njet_diag_MCcomb[b][i][k][j]->Fill(JetMassSym,weight);
				     P_bjet_pt_njet_diag_MCcomb[b][i][k][j]->Fill(PTriplet[q],weight);
				     h31_bjet_pt_njet_diag_MCcomb[b][i][k][j]->Fill(h31Triplet[q],weight);
				     Maxtr_bjet_pt_njet_diag_MCcomb[b][i][k][j]->Fill(Maxtr[q],weight);
				     countCombTrip++;
				   }
				 }          
			       }
			} // end triplet loop
			int mtpbsiz = massTripletPassBtag[b].size();
			double new_wt = weight;
			if (mtpbsiz > 1)
			  new_wt *= 1.0/mtpbsiz;
			
			for(unsigned int q = 0; q < mtpbsiz; q++){
			  Mjjj_btag_bjet_trpwt_pt_njet_diag[b][i][k][j]->Fill(massTripletPassBtag[b][q],new_wt);
			  if(sphericity_ >= 0.4) Mjjj_Sph4_btag_bjet_trpwt_pt_njet_diag[b][i][k][j]->Fill(massTripletPassBtag[b][q],new_wt);
			 if (q==0)
			   {
			     //for f1, count all events where at least one triplet passed the diagonal cut
			     HT_btag_bjet_pt_njet_diag[b][i][k][j]->Fill(SumptAllJet, weight);
			      Ntrip_btag_bjet_pt_njet_diag[b][i][k][j]->Fill(mtpbsiz);
			      if(sphericity_ >= 0.4) {
				HT_Sph4_btag_bjet_pt_njet_diag[b][i][k][j]->Fill(SumptAllJet, weight);
				Ntrip_Sph4_btag_bjet_pt_njet_diag[b][i][k][j]->Fill(mtpbsiz);
				
			      }

			      MET_bjet_pt_njet_diag[b][i][k][j]->Fill(pfMET, weight);
			      MET_over_HT_bjet_pt_njet_diag[b][i][k][j]->Fill(pfMET/SumptAllJet, weight);
			      HMT_bjet_pt_njet_diag[b][i][k][j]->Fill(SumMetPtAllJet20, weight);
			      nJet35_bjet_pt_njet_diag[b][i][k][j]->Fill(nJet35, weight);
			      nBJet35_bjet_pt_njet_diag[b][i][k][j]->Fill(nBJet35, weight);
			      nVertex_bjet_pt_njet_diag[b][i][k][j]->Fill(nGoodVtx,weight);
			      Jet0_bjet_pt_njet_diag[b][i][k][j]->Fill(fCleanJets[0]->Pt(), weight);
			      Jet1_bjet_pt_njet_diag[b][i][k][j]->Fill(fCleanJets[1]->Pt(), weight);
			      Jet2_bjet_pt_njet_diag[b][i][k][j]->Fill(fCleanJets[2]->Pt(), weight);
			      if(nJet35>=4)Jet3_bjet_pt_njet_diag[b][i][k][j]->Fill(fCleanJets[3]->Pt(), weight);
			      if(nJet35>=5)Jet4_bjet_pt_njet_diag[b][i][k][j]->Fill(fCleanJets[4]->Pt(), weight);
			      if(nJet35>=6)Jet5_bjet_pt_njet_diag[b][i][k][j]->Fill(fCleanJets[5]->Pt(), weight);
			      if(nJet35>=7)Jet6_bjet_pt_njet_diag[b][i][k][j]->Fill(fCleanJets[6]->Pt(), weight);
			      if(nBJet35>2){
				BJet1_bjet_pt_njet_diag[b][i][k][j]->Fill(fBJets[0]->Pt(), weight);
				BJet2_bjet_pt_njet_diag[b][i][k][j]->Fill(fBJets[1]->Pt(), weight);
				BJet3_bjet_pt_njet_diag[b][i][k][j]->Fill(fBJets[2]->Pt(), weight);
			      }
			      }
			    
			      }
			//cout<<"bjet: "<<b<<" "<<mtpbsiz<<" "<<countT<<endl;

			   Ntrip_bjet_pt_njet_diag_MCmatch[b][i][k][j]->Fill(countMatchTrip);
			   Ntrip_bjet_pt_njet_diag_MCcomb[b][i][k][j]->Fill(countCombTrip);
			 }//njet pt cut
		     }//diag cut loop
		   }//njetloop
	       }//pt loop
	   }//bjet loop
	 }//triplet loop
       }//3 or more b jets
     }//jetp
   } //minjet
     //lets see if the top branching ratios work
   
    } else notrig++;
      }//Msquqark
      
      
    }//get entrye
    cout << "\n events w/o trigger " << notrig << endl;
    
  }
  
  
  
  

float NtpThreeJet::GetBTagSF (float pt, float eta, int meanminmax){
  float jetPt=pt;

  float SFb_Unc_MultFactor =1.0;

  if (jetPt>800.0) jetPt= 800.0;
  if (jetPt<20.0) jetPt = 20.0;




  float  ScaleFactor     = CSVM_SFb_0to2p4->Eval(jetPt);
  float  ScaleFactor_unc = CSVM_SFb_errors->GetBinContent(CSVM_SFb_errors->GetXaxis()->FindBin(jetPt));
  
  float ScaleFactor_up   = ScaleFactor + abs(meanminmax)*SFb_Unc_MultFactor*ScaleFactor_unc;
  float ScaleFactor_down = ScaleFactor - abs(meanminmax)*SFb_Unc_MultFactor*ScaleFactor_unc;

  
  if(meanminmax==0) return ScaleFactor;
  if(meanminmax==-1 || meanminmax == -2) return ScaleFactor_down;
  if(meanminmax==1 || meanminmax == 2) return ScaleFactor_up;
  return ScaleFactor;
}



float NtpThreeJet::GetLightJetSF (float pt, float eta, int meanminmax){
  float jetPt=pt; 
  float jetAbsEta = fabs(eta);
  if (jetPt>800.0) jetPt= 800.0;
  if (jetPt<20.0) jetPt = 20.0;
  float  ScaleFactor     = 1;
  float ScaleFactor_up   = 1;
  float ScaleFactor_down = 1;

  if( pt>800 )
    {
      ScaleFactor      = CSVM_SFl_0to2p4->Eval(800);
      ScaleFactor_up   = ScaleFactor + 2*( (CSVM_SFl_0to2p4_max->Eval(800) - CSVM_SFl_0to2p4->Eval(800))/CSVM_SFl_0to2p4->Eval(800) )*ScaleFactor;
      ScaleFactor_down = ScaleFactor + 2*( (CSVM_SFl_0to2p4_min->Eval(800) - CSVM_SFl_0to2p4->Eval(800))/CSVM_SFl_0to2p4->Eval(800) )*ScaleFactor;
    }
  else
    {
      if(jetAbsEta<0.8)
	{
	  ScaleFactor      = CSVM_SFl_0to0p8->Eval(jetPt);
	  ScaleFactor_up   = ScaleFactor + ( (CSVM_SFl_0to0p8_max->Eval(jetPt) - CSVM_SFl_0to0p8->Eval(jetPt))/CSVM_SFl_0to0p8->Eval(jetPt) )*ScaleFactor;
	  ScaleFactor_down = ScaleFactor + ( (CSVM_SFl_0to0p8_min->Eval(jetPt) - CSVM_SFl_0to0p8->Eval(jetPt))/CSVM_SFl_0to0p8->Eval(jetPt) )*ScaleFactor;
	}
      else if(jetAbsEta>=0.8 && jetAbsEta<1.6)
	{
	  ScaleFactor      = CSVM_SFl_0p8to1p6->Eval(jetPt);
	  ScaleFactor_up   = ScaleFactor + ( (CSVM_SFl_0p8to1p6_max->Eval(jetPt) - CSVM_SFl_0p8to1p6->Eval(jetPt))/CSVM_SFl_0p8to1p6->Eval(jetPt) )*ScaleFactor;
	  ScaleFactor_down = ScaleFactor + ( (CSVM_SFl_0p8to1p6_min->Eval(jetPt) - CSVM_SFl_0p8to1p6->Eval(jetPt))/CSVM_SFl_0p8to1p6->Eval(jetPt) )*ScaleFactor;
	}
      else
	{
	  ScaleFactor      = CSVM_SFl_1p6to2p4->Eval(jetPt);
	  ScaleFactor_up   = ScaleFactor + ( (CSVM_SFl_1p6to2p4_max->Eval(jetPt) - CSVM_SFl_1p6to2p4->Eval(jetPt))/CSVM_SFl_1p6to2p4->Eval(jetPt) )*ScaleFactor;
	  ScaleFactor_down = ScaleFactor + ( (CSVM_SFl_1p6to2p4_min->Eval(jetPt) - CSVM_SFl_1p6to2p4->Eval(jetPt))/CSVM_SFl_1p6to2p4->Eval(jetPt) )*ScaleFactor;
	}
    }
  if(meanminmax==0) return ScaleFactor;
  if(meanminmax==-1) return ScaleFactor_down;
  if(meanminmax==1) return ScaleFactor_up;
  return ScaleFactor;
}
