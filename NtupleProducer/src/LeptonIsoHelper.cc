/** \file
 *
 *  $Date: 2013/05/13 17:10:20 $
 *  $Revision: 1.6 $
 *  \author N. Amapane
 */

#include <LLRHiggsTauTau/NtupleProducer/interface/LeptonIsoHelper.h>
#include <Muon/MuonAnalysisTools/interface/MuonEffectiveArea.h>
//#include <EGamma/EGammaAnalysisTools/interface/ElectronEffectiveArea.h>
#include <LLRHiggsTauTau/NtupleProducer/interface/CustomElectronEffectiveArea.h>

#include <iostream>

using namespace std;
using namespace edm;
using namespace pat;
using namespace reco;


int correctionType = 2; //1 = rho; 2 = dbeta;

InputTag LeptonIsoHelper::getMuRhoTag(int sampleType, int setup) {
  InputTag rhoTag;
  if (sampleType ==2011) {
    //rhoTag = InputTag("kt6PFJetsForIso","rho");//RH
    rhoTag = InputTag("fixedGridRhoFastjetAll","");
  } else if (sampleType ==2012) { 
    //rhoTag = InputTag("kt6PFJetsCentralNeutral","rho");//RH
    rhoTag = InputTag("fixedGridRhoFastjetAll","");
  } else {
    cout << "LeptonIsoHelper: Incorrect setup: " << sampleType << " " << setup << endl;
    abort();
  }
  return rhoTag;
}

InputTag LeptonIsoHelper::getEleRhoTag(int sampleType, int setup) {
  InputTag rhoTag;
  if (sampleType ==2011) {
    //rhoTag = InputTag("kt6PFJetsForIso","rho");
    rhoTag = InputTag("fixedGridRhoFastjetAll","");
  } else if (sampleType ==2012) {
    //rhoTag = InputTag("kt6PFJets","rho","RECO");
    rhoTag = InputTag("fixedGridRhoFastjetAll",""); // or "fixedGridRhoFastjetCentralNeutral"? 
  } else {
    cout << "LeptonIsoHelper: Incorect setup: " << sampleType << endl;
    abort();
  }
  return rhoTag;
}


float LeptonIsoHelper::combRelIsoPF(int sampleType, int setup, double rho, const pat::Muon& l, float fsr) {
  float PFChargedHadIso   = l.chargedHadronIso();
  float PFNeutralHadIso   = l.neutralHadronIso();
  float PFPhotonIso       = l.photonIso();
  //float PFPUChargedHadIso = l.puChargedHadronIso();
    
  MuonEffectiveArea::MuonEffectiveAreaTarget EAsetup;
  if (sampleType==2011) {
    EAsetup = MuonEffectiveArea::kMuEAData2011;
  } else if (sampleType ==2012) { 
    EAsetup = MuonEffectiveArea::kMuEAData2012;
  } else abort();

  if (correctionType==1) {
    float EA = MuonEffectiveArea::GetMuonEffectiveArea(MuonEffectiveArea::kMuGammaAndNeutralHadronIso04, 
						       l.eta(), EAsetup);
    return  (PFChargedHadIso + max(0., PFNeutralHadIso + PFPhotonIso - fsr - rho * EA))/l.pt();

  } else if (correctionType==2) {
    //return  (PFChargedHadIso + max(0., PFNeutralHadIso + PFPhotonIso - fsr - 0.5*PFPUChargedHadIso))/l.pt();
    return (l.pfIsolationR03().sumChargedHadronPt + max(
           l.pfIsolationR03().sumNeutralHadronEt +
           l.pfIsolationR03().sumPhotonEt - 
           0.5 * l.pfIsolationR03().sumPUPt, 0.0)) / l.pt();
  }
  return 0;
}

float LeptonIsoHelper::combRelIsoPF(const pat::Tau& l) {

  float PFChargedHadIso   = l.tauID ("chargedIsoPtSum");
  float PFNeutralHadIso   = l.tauID ("neutralIsoPtSum");
  float PFPhotonIso       = 0;//l.photonIso();
  float PFPUChargedHadIso = l.tauID ("puCorrPtSum");

  return  (PFChargedHadIso + max(0., PFNeutralHadIso + PFPhotonIso - 0.5*PFPUChargedHadIso))/l.pt();

//    return (l.pfIsolationVariables().sumChargedHadronPt + max(
//           l.pfIsolationVariables().sumNeutralHadronEt +
//           l.pfIsolationVariables().sumPhotonEt -
//           0.5 * l.pfIsolationVariables().sumPUPt, 0.0)) / l.pt();

}

float LeptonIsoHelper::combRelIsoPF(int sampleType, int setup, double rho, const pat::Electron& l, float fsr) {
  float PFChargedHadIso   = l.chargedHadronIso();
  float PFNeutralHadIso   = l.neutralHadronIso();
  float PFPhotonIso       = l.photonIso();
  if(correctionType==1){
  ElectronEffectiveArea::ElectronEffectiveAreaTarget EAsetup;
  if (sampleType ==2011) {
    EAsetup = ElectronEffectiveArea::kEleEAData2011;
  } else if (sampleType ==2012) { 
    EAsetup = ElectronEffectiveArea::kEleEAData2012; // Legacy
    // EAsetup = ElectronEffectiveArea::kEleEASpring14MC_PU20bx25 // retuned by Simon
  } else {
    abort();
  }

  float EA = ElectronEffectiveArea::GetElectronEffectiveArea(ElectronEffectiveArea::kEleGammaAndNeutralHadronIso04,
							     l.superCluster()->eta(), EAsetup);
  return  (PFChargedHadIso + max(0., PFNeutralHadIso + PFPhotonIso - fsr - rho * EA))/l.pt();
  }else{
    return (l.pfIsolationVariables().sumChargedHadronPt + max(
           l.pfIsolationVariables().sumNeutralHadronEt +
           l.pfIsolationVariables().sumPhotonEt - 
           0.5 * l.pfIsolationVariables().sumPUPt, 0.0)) / l.pt();
  }

}


float LeptonIsoHelper::combRelIsoPF(int sampleType, int setup, double rho, const Candidate* lep, float fsr) {
  // should check if lep->hasMasterClone()?  
  if (lep->isMuon()) {
    const pat::Muon* mu = dynamic_cast<const pat::Muon*>(lep->masterClone().get());
    return combRelIsoPF(sampleType, setup, rho, *mu, fsr);
  } else if (lep->isElectron()) {
    const pat::Electron* ele = dynamic_cast<const pat::Electron*>(lep->masterClone().get());
    return combRelIsoPF(sampleType, setup, rho, *ele, fsr);    
  }else {
    cout << "ERROR: LeptonIsoHelper: unknown type" << endl;
    abort();
  }
  return 0;
}

