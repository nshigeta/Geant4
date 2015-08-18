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
/// \file persistency/P03/include/ExTGPrimaryGeneratorAction.hh
/// \brief Definition of the ExTGPrimaryGeneratorAction class
//
// $Id: ExTGPrimaryGeneratorAction.hh 68025 2013-03-13 13:43:46Z gcosmo $
 
#ifndef ExTGPrimaryGeneratorAction_h
#define ExTGPrimaryGeneratorAction_h 1

#include "G4VUserPrimaryGeneratorAction.hh"

class G4ParticleGun;
class G4Event;

/// Example of primary generator action

class ExTGPrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
  public:

    ExTGPrimaryGeneratorAction(); 
   ~ExTGPrimaryGeneratorAction();

    void GeneratePrimaries(G4Event*);

  private:

    G4ParticleGun* fParticleGun;
};

// ---------------------------------------------------------------------------

#endif


