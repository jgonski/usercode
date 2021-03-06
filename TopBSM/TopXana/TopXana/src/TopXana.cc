// -*- C++ -*-
//
// Package:    TopXana
// Class:      TopXana
// 
/**\class TopXana TopXana.cc TopBSM/TopXana/src/TopXana.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Claudia Seitz
//         Created:  Fri Jun 17 12:26:54 EDT 2011
// $Id$
//
//


// system include files
#include <memory>


// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "DataFormats/HLTReco/interface/TriggerEvent.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/Math/interface/LorentzVector.h"
#include "DataFormats/PatCandidates/interface/Jet.h" // based on DataFormats/Candidate/interface/Particle.h
#include "DataFormats/PatCandidates/interface/JetCorrFactors.h"
#include "TTree.h"
#include "TopBSM/TopXana/interface/TopXana.h"

#include "DataFormats/PatCandidates/interface/Electron.h" 
#include "DataFormats/PatCandidates/interface/Muon.h" 
#include "DataFormats/PatCandidates/interface/MET.h" 
#include "DataFormats/PatCandidates/interface/Photon.h"

#include "FWCore/Common/interface/TriggerNames.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include <iostream>
#include <algorithm>
#include <vector>
#include <fstream>
//
// class declaration


TopXana::TopXana(const edm::ParameterSet& iConfig)

{
  _sumPtMin        = iConfig.getUntrackedParameter<double>("sumPtMin",         300.0);
  _debug           = iConfig.getUntrackedParameter<bool>  ("debug",            false);
  _outputFilename  = iConfig.getUntrackedParameter<string>("outputFilename",  "PatJets_test.root");
  _outputFilename2 = iConfig.getUntrackedParameter<string>("outputFilename2", "PatJets_testTree.root");
  _patJetType      = iConfig.getUntrackedParameter<string>("PatJetType",      "selectedPatJets");
  _njetsMin        = iConfig.getUntrackedParameter<int>   ("NjetsMin",         4);
  _njetsMax        = iConfig.getUntrackedParameter<int>   ("NjetsMax",         4);
  _etacut          = iConfig.getUntrackedParameter<double>("etacut",           3.0); 
  _jetptcut        = iConfig.getUntrackedParameter<double>("jetptcut",         20.0);
  
  _eeta          = iConfig.getUntrackedParameter<double>("eeta",           2.1); 
  _ept       = iConfig.getUntrackedParameter<double>("ept",         20.0);
  
  _meta          = iConfig.getUntrackedParameter<double>("meta",           2.1); 
  _mpt       = iConfig.getUntrackedParameter<double>("mpt",         20.0);
  
  _pheta          = iConfig.getUntrackedParameter<double>("pheta",           1.45); 
  _phpt       = iConfig.getUntrackedParameter<double>("phpt",         30.0);
  
  _nbTagsMin       = iConfig.getUntrackedParameter<int>   ("nbTagsMin",        0);
  _nbTagsMax       = iConfig.getUntrackedParameter<int>   ("nbTagsMax",        1000);
  _isData          = iConfig.getUntrackedParameter<bool>  ("isData",           true);
  _noTripletBtag   = iConfig.getUntrackedParameter<bool>  ("noTripletBtag",    false);
  _nTripletBtagsMin= iConfig.getUntrackedParameter<int>   ("nTripletBtagsMin", 0);
  _nTripletBtagsMax= iConfig.getUntrackedParameter<int>   ("nTripletBtagsMax", 1000);
  _doBtagEff       = iConfig.getUntrackedParameter<bool>   ("doBtagEff", true);
  
  JSONFilename  = iConfig.getUntrackedParameter<string>("JSONFilename","Cert_160404-166502_7TeV_PromptReco_Collisions11_JSON.txt");
   //now do what ever initialization is needed

}


TopXana::~TopXana()
{
 
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called for each event  ------------
void
TopXana::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
 /////////////////
   //GET EVT INFO
   ////////////////
   run   = iEvent.id().run();
   event = iEvent.id().event();
   lumis = iEvent.id().luminosityBlock();
  ////////////////////////
  ////CHECK JSON
  //////////////////////

  GoodRun=kFALSE;
  UseJson(GoodRuns,GoodLumiStart,GoodLumiEnd,nGoodRuns,run,lumis);
  //onlye keep events when they are in the good run list
  //std::cout<<"--------------run: "<<run<<"--event: "<<event<<"--lumi: "<<lumis<<"--entry: "<<entry<<"-----------"<<endl;
  
 
 if(GoodRun){
   ///////////////////
   ////CLEAN UP VARIABLES
   //////////////////

  fGoodJets.clear(); fCleanJets.clear(); 
  nGoodJets=0; nCleanJets=0;
  fGoodElectrons.clear(); fCleanElectrons.clear();
  nGoodElectrons=0; nCleanElectrons=0; 
  fGoodMuons.clear(); fCleanMuons.clear(); 
  nGoodMuons=0; nCleanMuons=0;  
  fGoodPhotons.clear(); fCleanPhotons.clear();
  nGoodPhotons=0; nCleanPhotons=0;
  for (int i=0; i<200; ++i)
    {
      pdgID[i] = -99;
      MCpx[i] = -99;
      MCpy[i] = -99;
      MCpz[i] = -99;
      MCe[i] = -99;
    }
 
   Triplet.clear();   
   sumScalarPtTriplet.clear();
   sumVectorPtTriplet.clear();
   massTriplet.clear();
   nTriplets=0; q=0; //basically just triplet counting
   IsVtxGood = 0; nGoodVtx = 0;

  
  
   ///////////////////
   /////DO OBJECT ID
   //////////////////
   //Select all the objects int the event (vertex function makes some plots)
   DoJetID(iEvent);
   DoVertexID(iEvent);
   DoElectronID(iEvent);
   DoMuonID(iEvent);
   DoPhotonID(iEvent);

   //make some plots before cleanup 
   h_nGoodJets->Fill(nGoodJets);
   h_nGoodElectrons->Fill(nGoodElectrons);  
   h_nGoodMuons->Fill(nGoodMuons);
   h_nGoodPhotons->Fill(nGoodPhotons);

   //////////////////
   ///////REMOVE OVERLAP IN OCJECT COLLECTION
   ////////////////
   //lets clean our object collections
   DoCleanUp(fGoodMuons,fGoodElectrons,fGoodPhotons,fGoodJets);

   //make plots after the clean up
   h_nCleanElectrons->Fill(nCleanElectrons);
   h_nCleanMuons->Fill(nCleanMuons);
   h_nCleanPhotons->Fill(nCleanPhotons);
   h_nCleanJets->Fill(nCleanJets);

   /////////////////
   //////KINEMATIC PLOTS OF OBJECTS + STUFF FOR THE TREE
   ////////////////
   //make some kinematic plots and write out variables for the tree
   nJets=nCleanJets;
   for(int i=0; i<nCleanJets; i++){
     if(i<6){
       v_jet_pt[i]->Fill(fCleanJets[i].pt()); 
       v_jet_eta[i]->Fill(fCleanJets[i].eta()); 
       v_jet_phi[i]->Fill(fCleanJets[i].phi()); 
     }
     jetpt[i]=fCleanJets[i].pt();
     jetpx[i]=fCleanJets[i].px();
     jetpy[i]=fCleanJets[i].py();
     jetpz[i]=fCleanJets[i].pz();
     jete[i]=fCleanJets[i].energy();	 
     
     
   }
   nElectrons=nCleanElectrons;
   for(int i=0; i<nCleanElectrons; i++){
     if(i<6){
       v_e_pt[i]->Fill(fCleanElectrons[i].pt()); 
       v_e_eta[i]->Fill(fCleanElectrons[i].eta()); 
       v_e_phi[i]->Fill(fCleanElectrons[i].phi()); 
     }
     ept[i]=fCleanElectrons[i].pt();
     epx[i]=fCleanElectrons[i].px();
     epy[i]=fCleanElectrons[i].py();
     epz[i]=fCleanElectrons[i].pz();
     ee[i]=fCleanElectrons[i].energy();	 
     
     
   }
   nMuons=nCleanMuons;
   for(int i=0; i<nCleanMuons; i++){
     if(i<6){
       v_m_pt[i]->Fill(fCleanMuons[i].pt()); 
       v_m_eta[i]->Fill(fCleanMuons[i].eta()); 
       v_m_phi[i]->Fill(fCleanMuons[i].phi()); 
     }
     mpt[i]=fCleanMuons[i].pt();
     mpx[i]=fCleanMuons[i].px();
     mpy[i]=fCleanMuons[i].py();
     mpz[i]=fCleanMuons[i].pz();
     me[i]=fCleanMuons[i].energy();	 
     
     
   }
   nPhotons=nCleanPhotons;
   for(int i=0; i<nCleanPhotons; i++){
     if(i<6){
       v_ph_pt[i]->Fill(fCleanPhotons[i].pt()); 
       v_ph_eta[i]->Fill(fCleanPhotons[i].eta()); 
       v_ph_phi[i]->Fill(fCleanPhotons[i].phi()); 
     }
     phpt[i]=fCleanPhotons[i].pt();
     phpx[i]=fCleanPhotons[i].px();
     phpy[i]=fCleanPhotons[i].py();
     phpz[i]=fCleanPhotons[i].pz();
     phe[i]=fCleanPhotons[i].energy();	 
     
   }
   
   ///////////////////////////
   ////////EVENT SELECTION FOR PLOTS -> TREE HAS EVERYTHING
   /////////////////////////
   //DI-LEPTON PLOTS
   if (nCleanElectrons==2 && fCleanElectrons[1].pt()>45) h_DiElectronMass->Fill((fCleanElectrons[0].p4()+fCleanElectrons[1].p4()).mass());
   if (nCleanMuons==2 && fCleanMuons[1].pt()>30) h_DiMuonMass->Fill((fCleanMuons[0].p4()+fCleanMuons[1].p4()).mass());
   if ((nCleanElectrons == 1 && fCleanElectrons[0].pt()>45) && (nCleanMuons==1 && fCleanMuons[0].pt()>30)) h_ElectronMuonMass->Fill((fCleanElectrons[0].p4()+fCleanMuons[0].p4()).mass());
   //JET cuts
   bool enoughCleanJets=true;  
   if (_njetsMin  < 3) enoughCleanJets=false;
   //Need at leat N Jets                                                                       
   if (nCleanJets < _njetsMin) enoughCleanJets=false;
   //Need no more than N Jets ;)                                                          
   if (nCleanJets > _njetsMax) enoughCleanJets=false;

   //SINGLE LEPTON SELECTION 
   bool minCleanLeptons=false;
   bool enoughCleanLeptons=false;
   if ((nCleanElectrons+ nCleanMuons) == 1){
     minCleanLeptons=true;
     if (nCleanElectrons==1 && fCleanElectrons[0].pt()>45) enoughCleanLeptons=true;
     if (nCleanMuons==1 && fCleanMuons[0].pt()>30) enoughCleanLeptons=true;
   }
   /////////////////////
   //////TRIPLETS
   ///////////////////
  
    
   //call function that makes the triplets for all events that pass the minimum lepton ID but enough good jets
if(enoughCleanJets && minCleanLeptons){

    nTriplets=0;
   MakeTriplets(fCleanJets);  
   if(enoughCleanLeptons){
   //make some plots only for events with cuts applied
   
     
     //ok make now some plots
     for(unsigned int q=0; q<massTriplet.size(); q++)
       {
	 for (int i=0; i<7; i++)
	   {
	     float iPt=20+i*10;
	     for(int k=0; k<4; k++)
	       { 
		 unsigned int iNjet=k+3;
		 //count njets with the pt cut -> gonna be slow
		 // if(iNjet<=fCleanJets.size()){
		   if(Triplet[q][2].pt()>iPt && fCleanJets[3].pt()>iPt/*&& fCleanJets[iNjet-1].pt()>iPt*/)
		     {
		       Mjjj_sumpt_pt_njet[i][k]->Fill(sumScalarPtTriplet[q],massTriplet[q]);
		     }
		   for(int j=0; j<20; j++){
		     
		     float iDiag=j*10+40;
		     //to implement the pt cut we only keep triplets where the lowest jet passes pt cut
		     //also we check if the njetsMin cut satisfies this pt cut
		     //std::cout<<"before selection"<<endl;
		     // std::cout<<Triplet[q][2].pt()<<" "<<fCleanJets.size()<<" "<<fCleanJets[iNjet-1].pt()<<endl;
		     //if(iNjet>fCleanJets.size()){
		     // cout<<Triplet[q][2].pt()<<endl;
		     // cout<<fCleanJets.size()<<endl;
		     // cout<<iNjet-1<<endl;
			 //}
		     if(Triplet[q][2].pt()>iPt && fCleanJets[3].pt()>iPt/*&& fCleanJets[_njetsMin-1].pt()>iPt && fCleanJets[iNjet-1].pt()>iPt*/)
		       {
			 //cout<<"after selection"<<endl;
			 //cout<<Triplet[q][2].pt()<<" "<<fCleanJets.size()<<" "<<fCleanJets[iNjet-1].pt()<<endl;
			 if(massTriplet[q]<(sumScalarPtTriplet[q]-iDiag))
			   {
			     Mjjj_pt_njet_diag[i][k][j]->Fill(massTriplet[q]);
			   }
		       }
		     //  }
		 }//diag cut loop
	       }//njetloop
	   }//pt loop
       }//triplet loop
     
   }
 }
   
   //fill the tree use golden JSON file
   GetMCTruth(iEvent);
   MyTree->Fill();
   entry++;
   
 }
}


// ------------ method called once each job just before starting event loop  ------------
void 
TopXana::beginJob() 
{ if (_isData) {
    char c;
    int n;
    
    ifstream JSONFile(JSONFilename.data());
    nGoodRuns=0;
    int run, lb, le;
    bool startlb = false;
    while(!JSONFile.eof()){
      c=JSONFile.peek();
      while(!JSONFile.eof() && !( (c >= '0') && (c <= '9'))) {
	c = JSONFile.get();
	c = JSONFile.peek();

      }
      JSONFile>>n;
      if(n>100000){
	run = n;
	//cout<<run<<endl;
      }
      else{
	if(!startlb){
	  lb = n;
	  startlb = true;
	}
	else{
	  le = n;
	  GoodRuns.push_back(run);
	  GoodLumiStart.push_back(lb);
	  GoodLumiEnd.push_back(le);
	  ++nGoodRuns;
	  startlb = false;
	}
      }
    }
    cout << "Got: " << nGoodRuns << " specified as good." << endl;
    
    }

  countE=0;
  //define the tree we want to write out
  outputFile2= new TFile(_outputFilename2.data(),"recreate"); 
  outputFile2->cd();
  MyTree = new TTree("Events","");
  MyTree->Branch("run", &run);
  MyTree->Branch("event", &event);
  MyTree->Branch("lumis", &lumis);
 MyTree->Branch("entry", &entry);
  if(!_isData){
  MyTree->Branch("pdgID",pdgID,"pdgID[200]/I");
  MyTree->Branch("MCpx", MCpx, "MCpx[200]/F");
  MyTree->Branch("MCpy", MCpy, "MCpy[200]/F");
  MyTree->Branch("MCpz", MCpz, "MCpz[200]/F");
  MyTree->Branch("MCe", MCe, "MCe[200]/F");
  }
  MyTree->Branch("nJets", &nJets);
  MyTree->Branch("nElectrons", &nElectrons);
  MyTree->Branch("nMuons", &nMuons);
  MyTree->Branch("nPhotons", &nPhotons);
  MyTree->Branch("nTriplets", &nTriplets);
 MyTree->Branch("nGoodVtx", &nGoodVtx);

  MyTree->Branch("jetpt[nJets]", jetpt);
  MyTree->Branch("jetpx[nJets]", jetpx);
  MyTree->Branch("jetpy[nJets]", jetpy);
  MyTree->Branch("jetpz[nJets]", jetpz);
  MyTree->Branch("jete[nJets]", jete);

 
  MyTree->Branch("ept[nElectrons]", ept);
  MyTree->Branch("epx[nElectrons]", epx);
  MyTree->Branch("epy[nElectrons]", epy);
  MyTree->Branch("epz[nElectrons]", epz);
  MyTree->Branch("ee[nElectrons]", ee);
  
 
  MyTree->Branch("mpt[nMuons]", mpt);
  MyTree->Branch("mpx[nMuons]", mpx);
  MyTree->Branch("mpy[nMuons]", mpy);
  MyTree->Branch("mpz[nMuons]", mpz);
  MyTree->Branch("me[nMuons]", me);
  
 
  MyTree->Branch("phpt[nPhotons]", phpt);
  MyTree->Branch("phpx[nPhotons]", phpx);
  MyTree->Branch("phpy[nPhotons]", phpy);
  MyTree->Branch("phpz[nPhotons]", phpz);
  MyTree->Branch("phe[nPhotons]", phe);
  
  MyTree->Branch("triplet_jet1pt[nTriplets]", triplet_jet1pt);
  MyTree->Branch("triplet_jet2pt[nTriplets]", triplet_jet2pt);
  MyTree->Branch("triplet_jet3pt[nTriplets]", triplet_jet3pt);
  MyTree->Branch("triplet_sumScalarPt[nTriplets]",triplet_sumScalarPt);
  MyTree->Branch("triplet_mass[nTriplets]",triplet_mass);
  MyTree->Branch("triplet_sumVectorPt[nTriplets]",triplet_sumVectorPt);
  
  MyTree->Branch("triplet_jet1px[nTriplets]", triplet_jet1px);
  MyTree->Branch("triplet_jet1py[nTriplets]", triplet_jet1py);
  MyTree->Branch("triplet_jet1pz[nTriplets]", triplet_jet1pz);
  MyTree->Branch("triplet_jet1e[nTriplets]", triplet_jet1e);

 
  MyTree->Branch("triplet_jet2px[nTriplets]", triplet_jet2px);
  MyTree->Branch("triplet_jet2py[nTriplets]", triplet_jet2py);
  MyTree->Branch("triplet_jet2pz[nTriplets]", triplet_jet2pz);
  MyTree->Branch("triplet_jet2e[nTriplets]", triplet_jet2e);

 
  MyTree->Branch("triplet_jet3px[nTriplets]", triplet_jet3px);
  MyTree->Branch("triplet_jet3py[nTriplets]", triplet_jet3py);
  MyTree->Branch("triplet_jet3pz[nTriplets]", triplet_jet3pz);
  MyTree->Branch("triplet_jet3e[nTriplets]", triplet_jet3e);
  
 

  char hTITLE[99];
  char hNAME[99];
  //create output file
  outputFile = new TFile(_outputFilename.data(),"recreate");
  //initialize event counter
  entry = 0;

  h_DiElectronMass = new TH1F("DiElectronMass","DiElectronMass",200,0,400);
  h_DiMuonMass = new TH1F("DiMuonMass","DiMuonMass",200,0,400);
  h_ElectronMuonMass = new TH1F("ElectronMuonMass","ElectronMuonMass",200,0,400);


  sprintf(hTITLE, "Number of good Jet with Pt>%i and Eta<%i", (int) _jetptcut,(int) _etacut);
  h_nGoodJets = new TH1F("nJetsGood", hTITLE,20,0,20);
  
  sprintf(hTITLE, "Number of good Electrons with Pt>%i and Eta<%i", (int) _ept,(int) _eeta);
  h_nGoodElectrons = new TH1F("nElectronsGood", hTITLE,10,0,10);
  
  sprintf(hTITLE, "Number of good Muons with Pt>%i and Eta<%i", (int) _mpt,(int) _meta);
  h_nGoodMuons = new TH1F("nMuonsGood", hTITLE,10,0,10);
  
  sprintf(hTITLE, "Number of good Photons with Pt>%i and Eta<%i", (int) _phpt,(int) _pheta);
  h_nGoodPhotons = new TH1F("nPhotonsGood", hTITLE,10,0,10);
  
  sprintf(hTITLE, "Number of clean Jet with Pt>%i and Eta<%i", (int) _jetptcut,(int) _etacut);
  h_nCleanJets = new TH1F("nJetsClean", hTITLE,20,0,20);
  
  sprintf(hTITLE, "Number of clean Electrons with Pt>%i and Eta<%i", (int) _ept,(int) _eeta);
  h_nCleanElectrons = new TH1F("nElectronsClean", hTITLE,10,0,10);
  
  sprintf(hTITLE, "Number of clean Muons with Pt>%i and Eta<%i", (int) _mpt,(int) _meta);
  h_nCleanMuons = new TH1F("nMuonsClean", hTITLE,10,0,10);
  
  sprintf(hTITLE, "Number of clean Photons with Pt>%i and Eta<%i", (int) _phpt,(int) _pheta);
  h_nCleanPhotons = new TH1F("nPhotonsClean", hTITLE,10,0,10);
  
  h_nGoodVtx = new TH1F("nVtx","Number of Vertices",30,0,30);
  h_zPosGoodVtx = new TH1F("zPosCleanVtx","Z position of the vertices",600,-30,30);
  
  for(int i=0; i<6; i++)
    {
      sprintf(hNAME, "jet_%i_pt", i);
      sprintf(hTITLE, "JetPt of the %i st Jet with Pt>%i and Eta<%i", i,(int) _jetptcut,(int) _etacut);
      v_jet_pt.push_back(new TH1F(hNAME,hTITLE,200,0,1000));
      
      sprintf(hNAME, "jet_%i_eta", i);
      sprintf(hTITLE, "JetEta of the %i st Jet with Pt>%i and Eta<%i", i,(int) _jetptcut,(int) _etacut);
      v_jet_eta.push_back(new TH1F(hNAME,hTITLE,200,-5,5));
      
      sprintf(hNAME, "jet_%i_phi", i);
      sprintf(hTITLE, "JetPhi of the %i st Jet with Pt>%i and Eta<%i", i,(int) _jetptcut,(int) _etacut);
      v_jet_phi.push_back(new TH1F(hNAME,hTITLE,200,-5,5));
      
      sprintf(hNAME, "electron_%i_pt", i);
      sprintf(hTITLE, "ElectronPt of the %i st Electron with Pt>%i and Eta<%i", i,(int) _ept,(int) _eeta);
      v_e_pt.push_back(new TH1F(hNAME,hTITLE,200,0,1000));
      
      sprintf(hNAME, "electron_%i_eta", i);
      sprintf(hTITLE, "ElectronEta of the %i st Electron with Pt>%i and Eta<%i", i,(int) _ept,(int) _eeta);
      v_e_eta.push_back(new TH1F(hNAME,hTITLE,200,-5,5));
      
      sprintf(hNAME, "electron_%i_phi", i);
      sprintf(hTITLE, "ElectronPhi of the %i st Electron with Pt>%i and Eta<%i", i,(int) _ept,(int) _eeta);
      v_e_phi.push_back(new TH1F(hNAME,hTITLE,200,-5,5));
      
      sprintf(hNAME, "muon_%i_pt", i);
      sprintf(hTITLE, "MuonPt of the %i st Muon with Pt>%i and Eta<%i", i,(int) _mpt,(int) _meta);
      v_m_pt.push_back(new TH1F(hNAME,hTITLE,200,0,1000));

      sprintf(hNAME, "muon_%i_eta", i);
      sprintf(hTITLE, "MuonEta of the %i st Muon with Pt>%i and Eta<%i", i,(int) _mpt,(int) _meta);
      v_m_eta.push_back(new TH1F(hNAME,hTITLE,200,-5,5));
      
      sprintf(hNAME, "muon_%i_phi", i);
      sprintf(hTITLE, "MuonPhi of the %i st Muon with Pt>%i and Eta<%i", i,(int) _mpt,(int) _meta);
      v_m_phi.push_back(new TH1F(hNAME,hTITLE,200,-5,5));
      
      sprintf(hNAME, "photon_%i_pt", i);
      sprintf(hTITLE, "PhotonPt of the %i st Photon with Pt>%i and Eta<%i", i,(int) _phpt,(int) _pheta);
      v_ph_pt.push_back(new TH1F(hNAME,hTITLE,200,0,1000));
      
      sprintf(hNAME, "photon_%i_eta", i);
      sprintf(hTITLE, "PhotonEta of the %i st Photon with Pt>%i and Eta<%i", i,(int) _phpt,(int) _pheta);
      v_ph_eta.push_back(new TH1F(hNAME,hTITLE,200,-5,5));
      
      sprintf(hNAME, "photon_%i_phi", i);
      sprintf(hTITLE, "PhotonPhi of the %i st Photon with Pt>%i and Eta<%i", i,(int) _phpt,(int) _pheta);
      v_ph_phi.push_back(new TH1F(hNAME,hTITLE,200,-5,5));
     
    }
  
  for (int i=0; i<7; i++){
    int iPt=20+i*10;

    for(int k=0; k<4; k++){
      int iNjet=k+3;

      Mjjj_sumpt_pt_njet.push_back(std::vector<TH2F*> ());
      sprintf(hNAME, "Mjjj_sumpt_pt%i_GE%ijet", iPt,iNjet);
      Mjjj_sumpt_pt_njet[i].push_back(new TH2F(hNAME,hNAME,100,0,1000,100,0,1000));
      Mjjj_pt_njet_diag.push_back(std::vector<std::vector<TH1F*> > ());

      for(int j=0; j<20; j++){
	
	int iDiag=j*10+40;
	
	Mjjj_pt_njet_diag[i].push_back(std::vector<TH1F*> ());
	sprintf(hNAME, "Mjjj_pt%i_diag%i_GE%ijet", iPt,iDiag,iNjet);
	Mjjj_pt_njet_diag[i][k].push_back(new TH1F(hNAME,hNAME,100,0,1000));
      }
    }
  }


}

// ------------ method called once each job just after ending the event loop  ------------
void 
TopXana::endJob() 
{cout<<"Number of one lepton events "<<countE<<endl;
  //first write the tree
 outputFile2->Write(); outputFile2->Close();
 //now write out some plots
  char FOLDER[100];
  outputFile->cd();
  outputFile->mkdir("Jets");
  outputFile->cd("Jets");
  h_nGoodJets->Write();
  h_nCleanJets->Write();
  for(int i=0; i<6; i++)
    { 
      v_jet_pt[i]->Write();
      v_jet_eta[i]->Write();
      v_jet_phi[i]->Write();
    }
  
  outputFile->cd();
  outputFile->mkdir("Electrons");
  outputFile->cd("Electrons");
  h_nGoodElectrons->Write();
  h_nCleanElectrons->Write();
  for(int i=0; i<6; i++)
    { 
      v_e_pt[i]->Write();
      v_e_eta[i]->Write();
      v_e_phi[i]->Write();
    }  
  outputFile->mkdir("Muons");
  outputFile->cd("Muons");
  h_nGoodMuons->Write();
  h_nCleanMuons->Write();
  for(int i=0; i<6; i++)
    { 
      v_m_pt[i]->Write();
      v_m_eta[i]->Write();
      v_m_phi[i]->Write();
    }  
  
  outputFile->mkdir("Photons");
  outputFile->cd("Photons");
  h_nGoodPhotons->Write();
  h_nCleanPhotons->Write();
  for(int i=0; i<6; i++)
    { 
      v_ph_pt[i]->Write();
      v_ph_eta[i]->Write();
      v_ph_phi[i]->Write();
    }  
  outputFile->cd();
  outputFile->mkdir("Event");
  outputFile->cd("Event");
  h_DiMuonMass->Write();
h_DiElectronMass->Write();
h_ElectronMuonMass->Write();
  h_nGoodVtx->Write();
  h_zPosGoodVtx->Write();
  outputFile->cd();
  TDirectory* now=outputFile->mkdir("Triplets");
  
  for (int i=0; i<7; i++){
    sprintf(FOLDER, "jetpt_%i", i*10+20);
    now->mkdir(FOLDER);
    now->cd(FOLDER);
    for (int k=0; k<4; k++){
      Mjjj_sumpt_pt_njet[i][k]->Write();
      for(int j=0; j<20; j++){ 
	cout<<"Mjjj_pt"<<i*10+20<<"_njet"<<k+3<<"_diag"<<j*10+40<<"  "<<Mjjj_pt_njet_diag[i][k][j]->GetEntries()<<endl;
      	Mjjj_pt_njet_diag[i][k][j]->Write();  
      }
    }
  }
}

// ------------ method called when starting to processes a run  ------------
void 
TopXana::beginRun(edm::Run const&, edm::EventSetup const&)
{
}

// ------------ method called when ending the processing of a run  ------------
void 
TopXana::endRun(edm::Run const&, edm::EventSetup const&)
{
}

// ------------ method called when starting to processes a luminosity block  ------------
void 
TopXana::beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}

// ------------ method called when ending the processing of a luminosity block  ------------
void 
TopXana::endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void 
TopXana::UseJson( vector<int > GoodRuns, vector<int > GoodLumiStart,  vector<int > GoodLumiEnd,  Int_t nGoodRuns , int run, int lumis){
 
  if (_isData) {
    GoodRun=kFALSE;
    for (int ii=0;ii<nGoodRuns;++ii){
      if (run == GoodRuns[ii]){
	if (lumis >= GoodLumiStart[ii]
	    && lumis <=GoodLumiEnd[ii])
	  GoodRun = kTRUE;
      }
    }
  } else {  
    GoodRun = kTRUE;
  }
  return;
}

void 
TopXana::DoJetID(const edm::Event& iEvent){
   // Jet Handle  
  Handle< vector<Jet> > PatJets; 
  iEvent.getByLabel(_patJetType, PatJets); 
   //lets do some JetID
  
  for (unsigned int j=0; j<PatJets->size(); j++) {
     if (_debug && j==0){
       std::cout<<"--------------run: "<<run<<"--event: "<<event<<"--lumi: "<<lumis<<"--entry: "<<entry<<"-----------"<<endl;
       std::cout << "Number of PatJets: " << PatJets->size() << std::endl;
     }
     
     // Choose jets based on pt and abs(eta)
     if ((*PatJets)[j].pt()        > float(_jetptcut)  &&
	 fabs((*PatJets)[j].eta()) < float(_etacut))
       {
	 if (_debug) std::cout << "Jet #" << j << " Passes pT cut of " << _jetptcut 
			       << " and eTa cut of " << _etacut << " with pT: " 
			       <<  (*PatJets)[j].pt() << ", and eTa: " 
			       << (*PatJets)[j].eta() << std::endl;
	 bool jetID = false;
	 if (_patJetType == "selectedPatJets"){
	   //CaloJet ID
	   jetID = 
	     (((*PatJets)[j].correctedJet("Uncorrected").emEnergyFraction() > 0.01 ||
	       fabs((*PatJets)[j].eta())                            > 2.4) &&
	      (*PatJets)[j].correctedJet("Uncorrected").jetID().n90Hits    > 1     &&
	      (*PatJets)[j].correctedJet("Uncorrected").jetID().fHPD       < 0.98);
	 }
	 if (_patJetType == "selectedPatJetsAK5PF" || _patJetType == "goodPatJetsPFlow"){
	   //ParticleFlow ID
	   jetID = 
	     ((*PatJets)[j].correctedJet("Uncorrected").neutralHadronEnergyFraction()   < 0.99 && 
	      (*PatJets)[j].correctedJet("Uncorrected").neutralEmEnergyFraction()       < 0.99 &&
	      (*PatJets)[j].correctedJet("Uncorrected").numberOfDaughters()             > 1    &&
	      (fabs((*PatJets)[j].eta())                    > 2.4  ||
	       ((*PatJets)[j].correctedJet("Uncorrected").chargedHadronEnergyFraction() > 0.   &&
		(*PatJets)[j].correctedJet("Uncorrected").chargedEmEnergyFraction()     < 0.99 &&
		(*PatJets)[j].correctedJet("Uncorrected").chargedMultiplicity()         > 0.))); 
	 }
	 
	 if(jetID){
	   if (_debug) std::cout<<"After JetID"
				<< "Jet #" << j << " Passes pT cut of " << _jetptcut 
				<< " and eTa cut of " << _etacut << " with pT: " 
				<<  (*PatJets)[j].pt() << ", and eTa: " 
				<< (*PatJets)[j].eta() << std::endl;
	   
	   
	 
	   fGoodJets.push_back((*PatJets)[j]);
	 
	   nGoodJets++; 
	  
	   
	 }//JetID
       }//JetKinematics
   }//JetLoop
 if (_debug) std::cout << "Found "<< nGoodJets << " Jets" << std::endl;
return;
}

void 
TopXana::DoVertexID(const edm::Event& iEvent){

  edm::Handle<reco::VertexCollection>  recVtxs;
  iEvent.getByLabel("offlinePrimaryVertices", recVtxs);
  
 
  
  for (size_t i=0; i<recVtxs->size(); ++i)
    if (!((*recVtxs)[i].isFake())) {
      if ( ((*recVtxs)[i].ndof() > 4) &&
           (fabs( (*recVtxs)[i].z()) <= 24) &&
           ((*recVtxs)[i].position().rho() <= 2) ){
        nGoodVtx++;
	h_zPosGoodVtx->Fill((*recVtxs)[i].z());
      }
    }
  
  if (nGoodVtx > 0) IsVtxGood = 1;
  h_nGoodVtx->Fill(nGoodVtx);
  
  return;
}

void 
TopXana::DoElectronID(const edm::Event& iEvent){
  
  Handle< vector<Electron> > PatElectrons;
  iEvent.getByLabel("selectedPatElectrons", PatElectrons);

// begin electron loop                                                                                                                                               
  for (unsigned int j=0; j<PatElectrons->size(); j++) {
    //Fill all electrons                                                                                                                                               
  
    bool passconv=false;

    int eleid = (*PatElectrons)[j].electronID("simpleEleId80relIso");
    if (eleid ==7) passconv=true; // passes conversion rejection and everything else                                                                                   
    //        0: fails                                                                                                                                                 
    //        1: passes electron ID only                                                                                                                               
    //        2: passes electron Isolation only                                                                                                                        
    //        3: passes electron ID and Isolation only                                                                                                                 
    //        4: passes conversion rejection                                                                                                                           
    //        5: passes conversion rejection and ID                                                                                                                    
    //        6: passes conversion rejection and Isolation                                                                                                             
    //        7: passes the whole selection                                                                                                                            


    // Electron ID in Barrel                                                                                                                                           
    if ((*PatElectrons)[j].pt()>_ept && fabs((*PatElectrons)[j].eta())<_eeta) {
      if (passconv==true) {
        //passes electron Id                                                                                                                                           
          fGoodElectrons.push_back((*PatElectrons)[j]);
	  nGoodElectrons++;
          
        } // eleid in eb                                                                                                                                               
    } // pt eta cuts in eb                                                                                                                                             

  } // pat electron loop     
 
  return;
}

void
TopXana::DoMuonID(const edm::Event& iEvent){

  Handle< vector<Muon> > PatMuons; 
  iEvent.getByLabel("selectedPatMuons", PatMuons); 
  
   for (unsigned int j=0; j<PatMuons->size(); j++) {
  
    double relIso = ((*PatMuons)[j].trackIso()  +
                     (*PatMuons)[j].ecalIso()   +
                     (*PatMuons)[j].hcalIso()) / (*PatMuons)[j].pt();

    int    nValidHits        = -1;
    int    nValidTrackerHits = -1;
    int    nValidPixelHits   = -1;
    
    if ((*PatMuons)[j].globalTrack().isNonnull()) {
      nValidHits        = (*PatMuons)[j].globalTrack()->hitPattern().numberOfValidMuonHits();
      nValidTrackerHits = (*PatMuons)[j].globalTrack()->hitPattern().numberOfValidTrackerHits();
      nValidPixelHits   = (*PatMuons)[j].globalTrack()->hitPattern().numberOfValidPixelHits();
    }

    int stations = 0;
    unsigned stationMask((*PatMuons)[j].stationMask());
    for(unsigned i=0; i < 8; ++i)
      if(stationMask & 1 << i) ++stations;

    if ((*PatMuons)[j].pt()>_mpt && fabs((*PatMuons)[j].eta())<_meta) {
     
      if((*PatMuons)[j].isGlobalMuon()  &&
	 (*PatMuons)[j].isTrackerMuon() && 
	 nValidHits                >  0 && 
	 nValidTrackerHits         > 10 &&
	 nValidPixelHits           >  0 &&
	 (*PatMuons)[j].dB()       <  0.02 &&
	 (*PatMuons)[j].globalTrack()->normalizedChi2() < 10 && 
	 stations                > 0) {

	//	 stations                > 1) {

	if( (relIso  >  0.15) && (relIso < 0.25) ) {
	  fFakeMuons.push_back((*PatMuons)[j]);
	  
	}

	if( relIso  <  0.15) { // is good muon
	  //fill cut muon for all good muons that pass reliso

	 fGoodMuons.push_back((*PatMuons)[j]);
	 nGoodMuons++;

	  
	} // reliso
      } // global, tracker muons
    } // eta pt of muon
  } // muon loop 
  
  return;
}
void
TopXana::DoPhotonID(const edm::Event& iEvent){
  Handle< vector<Photon> > PatPhotons;
  iEvent.getByLabel("selectedPatPhotons", PatPhotons);

  // Photon Selection
  for (size_t i = 0; i != PatPhotons->size(); ++i) {
    if ((*PatPhotons)[i].et()>_phpt && fabs((*PatPhotons)[i].superCluster()->position().eta())<_pheta) {
        // tight photons
        if ( ((*PatPhotons)[i].ecalRecHitSumEtConeDR04()< 4.2+0.006*(*PatPhotons)[i].et()) &&
            ((*PatPhotons)[i].hcalTowerSumEtConeDR04()< 2.2+0.0025*(*PatPhotons)[i].et()) && 
            ((*PatPhotons)[i].hadronicOverEm() < 0.05) &&
            ((*PatPhotons)[i].trkSumPtHollowConeDR04() < 2+0.001*(*PatPhotons)[i].et()) &&
            ((*PatPhotons)[i].sigmaIetaIeta() <0.013) &&
            (!((*PatPhotons)[i].hasPixelSeed()))) { 
          fGoodPhotons.push_back((*PatPhotons)[i]);
	  nGoodPhotons++;
        }
      }
  }

  

  return;
}

void
TopXana::DoCleanUp(vector<Muon >fGoodMuons,vector<Electron >fGoodElectrons,vector<Photon >fGoodPhotons,vector<Jet >fGoodJets){
   for (size_t im = 0; im != fGoodMuons.size(); ++im) {
    fCleanMuons.push_back(fGoodMuons[im] );
    nCleanMuons++;
  }
 // Keep non-overlapping electrons
  for (size_t ie = 0; ie != fGoodElectrons.size(); ++ie) {
    bool HasOverlap = false;
    TLorentzVector Electron(fGoodElectrons[ie].px(), fGoodElectrons[ie].py(), fGoodElectrons[ie].pz(), fGoodElectrons[ie].energy()); 
    for (size_t im = 0; im != fGoodMuons.size(); ++im) {
      TLorentzVector Muon(fGoodMuons[im].px(), fGoodMuons[im].py(), fGoodMuons[im].pz(), fGoodMuons[im].p()); 
      if (Muon.DeltaR( Electron ) < 0.4) {
        HasOverlap = true;
	if(_debug && HasOverlap) cout<<"Overlap Electron Muon with pt: "<<fGoodElectrons[ie].pt()<<" eta: "<<fGoodElectrons[ie].eta()<<endl;

      }
    }
    if (!HasOverlap) {
      fCleanElectrons.push_back( fGoodElectrons[ie]);
      nCleanElectrons++;
    }
  }

  // Keep non-overlapping photons
  for (size_t ip = 0; ip != fGoodPhotons.size(); ++ip) {
    bool HasOverlap = false;
    TLorentzVector Photon(fGoodPhotons[ip].px(), fGoodPhotons[ip].py(), fGoodPhotons[ip].pz(), fGoodPhotons[ip].energy()); 
    for (size_t ie = 0; ie != fGoodElectrons.size(); ++ie) {
      TLorentzVector Electron(fGoodElectrons[ie].px(), fGoodElectrons[ie].py(), fGoodElectrons[ie].pz(), fGoodElectrons[ie].energy());
      if (Electron.DeltaR(Photon) < 0.4) {
        HasOverlap = true;
      }
    }
    if (!HasOverlap) {
      fCleanPhotons.push_back( fGoodPhotons[ip] );
      nCleanPhotons++;
    }
  }
  // Keep non-overlapping jets
  for (size_t ij = 0; ij != fGoodJets.size(); ++ij) {
    bool HasOverlap = false;
    TLorentzVector Jet(fGoodJets[ij].px(), fGoodJets[ij].py(), fGoodJets[ij].pz(), fGoodJets[ij].energy()); 
    for (size_t ie = 0; ie != fCleanElectrons.size(); ++ie) {
      TLorentzVector Electron(fCleanElectrons[ie].px(), fCleanElectrons[ie].py(), fCleanElectrons[ie].pz(), fCleanElectrons[ie].energy());
      if (Electron.DeltaR(Jet) < 0.4) {
        HasOverlap = true;
      }
    }
    for (size_t ip = 0; ip != fCleanPhotons.size(); ++ip) {
      TLorentzVector Photon(fCleanPhotons[ip].px(), fCleanPhotons[ip].py(), fCleanPhotons[ip].pz(), fCleanPhotons[ip].energy());
      if (Photon.DeltaR(Jet) < 0.4) {
        HasOverlap = true;
      }
    }
    for (size_t im = 0; im != fCleanMuons.size(); ++im) {
      TLorentzVector Muon(fCleanMuons[im].px(), fCleanMuons[im].py(), fCleanMuons[im].pz(), fCleanMuons[im].energy());
      if (Muon.DeltaR(Jet) < 0.4) {
        HasOverlap = true;
      }
    }

    if (!HasOverlap) {
      fCleanJets.push_back( fGoodJets[ij] );
      nCleanJets++;
    }
  }

  return;
}
void 
TopXana::GetMCTruth(const edm::Event& iEvent){
  if(!_isData){
    
    Handle< vector<reco::GenParticle> > GenParticles; 
    iEvent.getByLabel("genParticles", GenParticles);  
    for (unsigned int p=0; p<(*GenParticles).size(); p++) { 
      //cout<<p<<endl; 
      //use only that hard process
      if(p<200){
	pdgID[p]=(*GenParticles)[p].pdgId();
       
	MCpx[p]=(*GenParticles)[p].px();
	MCpy[p]=(*GenParticles)[p].py();
	MCpz[p]=(*GenParticles)[p].pz();
	MCe[p]=(*GenParticles)[p].energy();
	//cout<<MCpx[p]<<" "<<MCpy[p]<<" "<<MCpz[p]<<" "<<MCe[p]<<endl;
      }

     
    }
  }
  return;
}
 
void
TopXana::MakeTriplets(vector<Jet >fCleanJets){
   const int nCombs = TMath::Factorial(nCleanJets)/(TMath::Factorial(nCleanJets - 3)*TMath::Factorial(3));
   for(int a=0; a<nCombs;a++){
 Triplet.push_back(std::vector<pat::Jet > ());
   }
     for (int i=0+0; i<nCleanJets-2; i++) {
       for (int j=i+1; j<nCleanJets-1; j++) {
	 for (int k=j+1; k<nCleanJets-0; k++) {
	   Jet1=fCleanJets[i]; Jet2=fCleanJets[j]; Jet3=fCleanJets[k];
	  
	   if(_debug){
	     cout <<nTriplets<<"  jet" << i <<" pt = "<< Jet1.pt() 
		  << ", jet" << j <<" pt = "<< Jet2.pt() 
		  << ", jet" << k <<" pt = "<< Jet3.pt() << " mass:" << (Jet1.p4()+Jet2.p4()+Jet3.p4()).mass()<< endl;
	   }//debug
	   //save all the triplet infos and the jets -> eventually we might write this out in a tree
	  
	   Triplet[nTriplets].push_back(Jet1);
	   Triplet[nTriplets].push_back(Jet2);
	   Triplet[nTriplets].push_back(Jet3);

	   triplet_jet1pt[nTriplets]=Jet1.pt();
	   triplet_jet1px[nTriplets]=Jet1.px();
	   triplet_jet1py[nTriplets]=Jet1.py();
	   triplet_jet1pz[nTriplets]=Jet1.pz();
	   triplet_jet1e[nTriplets]=Jet1.energy();

	   triplet_jet2pt[nTriplets]=Jet2.pt();
	   triplet_jet2px[nTriplets]=Jet2.px();
	   triplet_jet2py[nTriplets]=Jet2.py();
	   triplet_jet2pz[nTriplets]=Jet2.pz();
	   triplet_jet2e[nTriplets]=Jet2.energy();

	   triplet_jet3pt[nTriplets]=Jet3.pt();
	   triplet_jet3px[nTriplets]=Jet3.px();
	   triplet_jet3py[nTriplets]=Jet3.py();
	   triplet_jet3pz[nTriplets]=Jet3.pz();
	   triplet_jet3e[nTriplets]=Jet3.energy();

	   sumScalarPtTriplet.push_back(Jet1.pt()+Jet2.pt()+Jet3.pt());
	   massTriplet.push_back((Jet1.p4()+Jet2.p4()+Jet3.p4()).mass());
	   sumVectorPtTriplet.push_back((Jet1.p4()+Jet2.p4()+Jet3.p4()).pt());



	   triplet_sumScalarPt[nTriplets]=Jet1.pt()+Jet2.pt()+Jet3.pt();
	   triplet_mass[nTriplets]=(Jet1.p4()+Jet2.p4()+Jet3.p4()).mass();
	   triplet_sumVectorPt[nTriplets]=(Jet1.p4()+Jet2.p4()+Jet3.p4()).pt();




	   nTriplets++;
	  
	   
	 }//k 
       }//j
     }//i
     return;
}

void
TopXana::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(TopXana);
