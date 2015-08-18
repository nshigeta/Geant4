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
// $Id: ExN04TrackerSD.hh 76296 2013-11-08 13:17:47Z gcosmo $
//
/// \file parallel/ParN04/include/ExN04TrackerSD.hh
/// \brief Definition of the ExN04TrackerSD class
//

#ifndef ExN04TrackerSD_h
#define ExN04TrackerSD_h 1

#include "G4VSensitiveDetector.hh"
#include "ExN04TrackerHit.hh"
class G4Step;
class G4HCofThisEvent;
class G4TouchableHistory;

class ExN04TrackerSD : public G4VSensitiveDetector
{

  public:
      ExN04TrackerSD(G4String name);
      ~ExN04TrackerSD();

      void Initialize(G4HCofThisEvent*HCE);
      G4bool ProcessHits(G4Step*aStep,G4TouchableHistory*ROhist);
      void EndOfEvent(G4HCofThisEvent*HCE);
      void clear();
      void DrawAll();
      void PrintAll();

  private:
      ExN04TrackerHitsCollection *trackerCollection;

  public:
};




#endif

