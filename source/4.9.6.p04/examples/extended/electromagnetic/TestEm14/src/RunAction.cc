//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
/// \file electromagnetic/TestEm14/src/RunAction.cc
/// \brief Implementation of the RunAction class
//
// $Id$
// 
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "RunAction.hh"

#include "DetectorConstruction.hh"
#include "PrimaryGeneratorAction.hh"
#include "HistoManager.hh"

#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4UnitsTable.hh"
#include "G4EmCalculator.hh"
#include "G4Gamma.hh"

#include "G4SystemOfUnits.hh"
#include "Randomize.hh"
#include <iomanip>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

RunAction::RunAction(DetectorConstruction* det, PrimaryGeneratorAction* prim)
  : fDetector(det), fPrimary(prim)
{
  fHistoManager = new HistoManager(); 
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

RunAction::~RunAction()
{ 
  delete fHistoManager;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunAction::BeginOfRunAction(const G4Run* aRun)
{  
  G4cout << "### Run " << aRun->GetRunID() << " start." << G4endl;
  
  // save Rndm status
  G4RunManager::GetRunManager()->SetRandomNumberStore(false);
  CLHEP::HepRandom::showEngineStatus();

  fTotalCount = 0;
  fSumTrack = fSumTrack2 = 0.;
  fEnTransfer = 0.;
     
  //histograms
  //
  G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
  if ( analysisManager->IsActive() ) {
    analysisManager->OpenFile();
  }     
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunAction::EndOfRunAction(const G4Run* aRun)
{
  G4int NbOfEvents = aRun->GetNumberOfEvent();
  if (NbOfEvents == 0) return;
  
  G4int  prec = G4cout.precision(5);
    
  G4Material* material = fDetector->GetMaterial();
  G4double density = material->GetDensity();
  G4int survive = 0;
   
  G4ParticleDefinition* particle = 
                            fPrimary->GetParticleGun()->GetParticleDefinition();
  G4String Particle = particle->GetParticleName();    
  G4double energy = fPrimary->GetParticleGun()->GetParticleEnergy();
  G4cout << "\n The run consists of " << NbOfEvents << " "<< Particle << " of "
         << G4BestUnit(energy,"Energy") << " through " 
         << G4BestUnit(fDetector->GetSize(),"Length") << " of "
         << material->GetName() << " (density: " 
         << G4BestUnit(density,"Volumic Mass") << ")" << G4endl;
  
  //frequency of processes
  G4cout << "\n Process calls frequency --->";
  std::map<G4String,G4int>::iterator it;  
  for (it = fProcCounter.begin(); it != fProcCounter.end(); it++) {
     G4String procName = it->first;
     G4int    count    = it->second;
     G4cout << "\t" << procName << " = " << count;
     if (procName == "Transportation") survive = count;
  }
      
  if (survive > 0) {
    G4cout << "\n\n Nb of incident particles surviving after "
           << G4BestUnit(fDetector->GetSize(),"Length") << " of "
           << material->GetName() << " : " << survive << G4endl;
  }
  
  if (fTotalCount == 0) fTotalCount = 1;   //force printing anyway
  
  //compute mean free path and related quantities
  //
  G4double MeanFreePath = fSumTrack /fTotalCount;     
  G4double MeanTrack2   = fSumTrack2/fTotalCount;     
  G4double rms = std::sqrt(std::fabs(MeanTrack2 - MeanFreePath*MeanFreePath));
  G4double CrossSection = 1./MeanFreePath;     
  G4double massicMFP = MeanFreePath*density;
  G4double massicCS  = 1./massicMFP;
   
  G4cout << "\n\n MeanFreePath:\t"   << G4BestUnit(MeanFreePath,"Length")
         << " +- "                   << G4BestUnit( rms,"Length")
         << "\tmassic: "             << G4BestUnit(massicMFP, "Mass/Surface")
         << "\n CrossSection:\t"     << CrossSection*cm << " cm^-1 "
         << "\t\t\tmassic: "         << G4BestUnit(massicCS, "Surface/Mass")
         << G4endl;
         
  //compute energy transfer coefficient
  //
  G4double MeanTransfer   = fEnTransfer/fTotalCount;
  G4double massTransfCoef = massicCS*MeanTransfer/energy;
   
  G4cout << "\n mean energy of charged secondaries: " << G4BestUnit(MeanTransfer, "Energy")
         << "\tmass_energy_transfer coef: "           << G4BestUnit(massTransfCoef, "Surface/Mass")
         << G4endl;       
 
  //check cross section from G4EmCalculator
  //
  G4cout << "\n Verification : "
         << "crossSections from G4EmCalculator \n";
  
  G4EmCalculator emCalculator;
  G4double sumc = 0.0;  
  for (it = fProcCounter.begin(); it != fProcCounter.end(); it++) {
    G4String procName = it->first;      
    G4double massSigma = 
    emCalculator.GetCrossSectionPerVolume(energy,particle,
                                              procName,material)/density;
    if (particle == G4Gamma::Gamma())
       massSigma = 
       emCalculator.ComputeCrossSectionPerVolume(energy,particle,
                                              procName,material)/density;                                              
    sumc += massSigma;
    G4cout << "\t" << procName << "= " 
           << G4BestUnit(massSigma, "Surface/Mass");
  }             
  G4cout << "\ttotal= " 
         << G4BestUnit(sumc, "Surface/Mass") << G4endl;
                  
  //restore default format         
  G4cout.precision(prec);
           
  // remove all contents in fProcCounter 
  fProcCounter.clear();
    
  //save histograms      
  G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();  
  if ( analysisManager->IsActive() ) {
    analysisManager->Write();
    analysisManager->CloseFile();
  }     

  // show Rndm status
  CLHEP::HepRandom::showEngineStatus();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
