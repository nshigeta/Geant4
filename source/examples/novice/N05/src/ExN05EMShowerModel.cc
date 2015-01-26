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
//
// $Id: ExN05EMShowerModel.cc,v 1.15 2006-11-14 09:21:44 gcosmo Exp $
// GEANT4 tag $Name: not supported by cvs2svn $
//
#include "ExN05EMShowerModel.hh"
#include "ExN05EnergySpot.hh"

#include "Randomize.hh"

#include "G4Electron.hh"
#include "G4Positron.hh"
#include "G4Gamma.hh"
#include "G4TransportationManager.hh"
#include "G4VSensitiveDetector.hh"
#include "G4TouchableHandle.hh"

ExN05EMShowerModel::ExN05EMShowerModel(G4String modelName, G4Region* envelope)
: G4VFastSimulationModel(modelName, envelope)
{
  fFakeStep          = new G4Step();
  fFakePreStepPoint  = fFakeStep->GetPreStepPoint();
  fFakePostStepPoint = fFakeStep->GetPostStepPoint();
  fTouchableHandle   = new G4TouchableHistory();
  fpNavigator        = new G4Navigator();
  fNaviSetup         = false;
}

ExN05EMShowerModel::ExN05EMShowerModel(G4String modelName)
: G4VFastSimulationModel(modelName)
{
  fFakeStep          = new G4Step();
  fFakePreStepPoint  = fFakeStep->GetPreStepPoint();
  fFakePostStepPoint = fFakeStep->GetPostStepPoint();
  fTouchableHandle   = new G4TouchableHistory();
  fpNavigator        = new G4Navigator();
  fNaviSetup         = false;
}

ExN05EMShowerModel::~ExN05EMShowerModel()
{
  delete fFakeStep;
  delete fpNavigator;
}

G4bool ExN05EMShowerModel::IsApplicable(const G4ParticleDefinition& particleType)
{
  return 
    &particleType == G4Electron::ElectronDefinition() ||
    &particleType == G4Positron::PositronDefinition() ||
    &particleType == G4Gamma::GammaDefinition();
}

G4bool ExN05EMShowerModel::ModelTrigger(const G4FastTrack& fastTrack)
{
  // Applies the parameterisation above 100 MeV:
  return fastTrack.GetPrimaryTrack()->GetKineticEnergy() > 100*MeV;
}

void ExN05EMShowerModel::DoIt(const G4FastTrack& fastTrack, 
		     G4FastStep& fastStep)
{
  // Kill the parameterised particle:
  fastStep.KillPrimaryTrack();
  fastStep.ProposePrimaryTrackPathLength(0.0);
  fastStep.ProposeTotalEnergyDeposited(fastTrack.GetPrimaryTrack()->GetKineticEnergy());

  // split into "energy spots" energy according to the shower shape:
  Explode(fastTrack);
  
  // and put those energy spots into the crystals:
  BuildDetectorResponse();
  
}

void ExN05EMShowerModel::Explode(const G4FastTrack& fastTrack)
{
  //-----------------------------------------------------
  //
  //-----------------------------------------------------

  // Reduced quantities:
  // -- critical energy in CsI:
  G4double Ec     = 800*MeV/(54. + 1.2); // 54 = mean Z of CsI
  G4double Energy = fastTrack.GetPrimaryTrack()->GetKineticEnergy();
  G4double y      = Energy/Ec;

  // compute value of parameter "a" of longitudinal profile, b assumed = 0.5
  G4double a, tmax, b(0.5), C;
  if (fastTrack.GetPrimaryTrack()->GetDefinition() == G4Gamma::GammaDefinition()) C = 0.5;
  else C = -0.5;
  tmax = 1.0 * (std::log(y) + C);
  a    = 1.0 + b*tmax;

  // t : reduced quantity = z/X0:
  G4double    t, bt;
  G4Material* CsI = G4Material::GetMaterial("CsI");
  G4double    X0  = CsI->GetRadlen();
  // Moliere radius:
  G4double Es = 21*MeV;
  G4double Rm = X0*Es/Ec;

  // axis of the shower, in global reference frame:
  G4ThreeVector xShower, yShower, zShower;
  zShower = fastTrack.GetPrimaryTrack()->GetMomentumDirection();
  xShower = zShower.orthogonal();
  yShower = zShower.cross(xShower);
  // starting point of the shower:
  G4ThreeVector sShower = fastTrack.GetPrimaryTrack()->GetPosition();

  // We shoot 100 spots of energy:
  G4int    nSpots  = 100;
  G4double deposit = Energy/double(nSpots);
  ExN05EnergySpot eSpot;
  eSpot.SetEnergy(deposit);
  G4ThreeVector ePoint;
  G4double z, r, phi;

  feSpotList.clear();
  for (int i = 0; i < nSpots; i++)
    {
      // Longitudinal profile:
      // -- shoot z according to Gamma distribution:
      bt = CLHEP::RandGamma::shoot(a,1.0);
      t  = bt/b;
      z  = t*X0;
      
      // transverse profile:
      // we set 90% of energy in one Rm,
      // the rest between 1 and 3.5 Rm:
      G4double xr = G4UniformRand();
      if (xr < 0.9) r = xr/0.9*Rm;
      else r = ((xr - 0.9)/0.1*2.5 + 1.0)*Rm;
      phi = G4UniformRand()*twopi;

      // build the position:
      ePoint = sShower +
	z*zShower +
	r*std::cos(phi)*xShower + r*std::sin(phi)*yShower;
      
      // and the energy spot:
      eSpot.SetPosition(ePoint);

      // Records the eSpot:
      feSpotList.push_back(eSpot);
    }
}


void ExN05EMShowerModel::BuildDetectorResponse()
{
  // Does the assignation of the energy spots to the sensitive volumes:
  for (size_t i = 0; i < feSpotList.size(); i++)
    {
      // Draw the energy spot:
      feSpotList[i].Draw();
      //      feSpotList[i].Print();
      
      // "converts" the energy spot into the fake
      // G4Step to pass to sensitive detector:
      AssignSpotAndCallHit(feSpotList[i]);
    }
}


void ExN05EMShowerModel::AssignSpotAndCallHit(const ExN05EnergySpot &eSpot)
{
  //
  // "converts" the energy spot into the fake
  // G4Step to pass to sensitive detector:
  //
  FillFakeStep(eSpot);

  //
  // call sensitive part: taken/adapted from the stepping:
  // Send G4Step information to Hit/Dig if the volume is sensitive
  //
  G4VPhysicalVolume* pCurrentVolume = 
    fFakeStep->GetPreStepPoint()->GetPhysicalVolume();
  G4VSensitiveDetector* pSensitive;
  
  if( pCurrentVolume != 0 )
    {
      pSensitive = pCurrentVolume->GetLogicalVolume()->
	GetSensitiveDetector();
      if( pSensitive != 0 )
	{
	  pSensitive->Hit(fFakeStep);
	}
    }
}


void ExN05EMShowerModel::FillFakeStep(const ExN05EnergySpot &eSpot)
{
  //-----------------------------------------------------------
  // find in which volume the spot is.
  //-----------------------------------------------------------
  if (!fNaviSetup)
    {
      fpNavigator->
	SetWorldVolume(G4TransportationManager::GetTransportationManager()->
		       GetNavigatorForTracking()->GetWorldVolume());
      fpNavigator->
	LocateGlobalPointAndUpdateTouchableHandle(eSpot.GetPosition(),
                                            G4ThreeVector(0.,0.,0.),
					    fTouchableHandle,
					    false);
      fNaviSetup = true;
    }
  else
    {
      fpNavigator->
	LocateGlobalPointAndUpdateTouchableHandle(eSpot.GetPosition(),
                                            G4ThreeVector(0.,0.,0.),
					    fTouchableHandle);
     }
  //--------------------------------------
  // Fills attribute of the G4Step needed
  // by our sensitive detector:
  //-------------------------------------
  // set touchable volume at PreStepPoint:
  fFakePreStepPoint->SetTouchableHandle(fTouchableHandle);
  // set total energy deposit:
  fFakeStep->SetTotalEnergyDeposit(eSpot.GetEnergy());
}