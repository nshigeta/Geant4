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
// $Id$
//
// Hadronic Process: AntiXiMinus Inelastic Process
// J.L. Chuma, TRIUMF, 20-Feb-1997
// Modified by J.L.Chuma 30-Apr-97: added originalTarget for CalculateMomenta
//
// NOTE:  The FORTRAN version of the cascade, CASAXM, simply called the
//        routine for the XiMinus particle.  Hence, the ApplyYourself function
//        below is just a copy of the ApplyYourself from the XiMinus particle.
 
#include "G4LEAntiXiMinusInelastic.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"

void G4LEAntiXiMinusInelastic::ModelDescription(std::ostream& outFile) const
{
  outFile << "G4LEAntiXiMinusInelastic is one of the Low Energy Parameterized\n"
          << "(LEP) models used to implement inelastic antiXi- scattering\n"
          << "from nuclei.  It is a re-engineered version of the GHEISHA\n"
          << "code of H. Fesefeldt.  It divides the initial collision\n"
          << "products into backward- and forward-going clusters which are\n"
          << "then decayed into final state hadrons.  The model does not\n"
          << "conserve energy on an event-by-event basis.  It may be applied\n"
          << "to antiXi- with initial energies between 0 and 25 GeV.\n";
}

G4HadFinalState*
G4LEAntiXiMinusInelastic::ApplyYourself(const G4HadProjectile& aTrack,
                                        G4Nucleus& targetNucleus)
{ 
  const G4HadProjectile *originalIncident = &aTrack;
  if (originalIncident->GetKineticEnergy()<= 0.1*MeV) {
    theParticleChange.SetStatusChange(isAlive);
    theParticleChange.SetEnergyChange(aTrack.GetKineticEnergy());
    theParticleChange.SetMomentumChange(aTrack.Get4Momentum().vect().unit()); 
    return &theParticleChange;      
  }

  // create the target particle
  G4DynamicParticle* originalTarget = targetNucleus.ReturnTargetParticle();

  if (verboseLevel > 1) {
    const G4Material *targetMaterial = aTrack.GetMaterial();
    G4cout << "G4LEAntiXiMinusInelastic::ApplyYourself called" << G4endl;
    G4cout << "kinetic energy = " << originalIncident->GetKineticEnergy()/MeV << "MeV, ";
    G4cout << "target material = " << targetMaterial->GetName() << ", ";
    G4cout << "target particle = " << originalTarget->GetDefinition()->GetParticleName()
           << G4endl;
  }

  // Fermi motion and evaporation
  // As of Geant3, the Fermi energy calculation had not been Done
  G4double ek = originalIncident->GetKineticEnergy()/MeV;
  G4double amas = originalIncident->GetDefinition()->GetPDGMass()/MeV;
  G4ReactionProduct modifiedOriginal;
  modifiedOriginal = *originalIncident;
    
  G4double tkin = targetNucleus.Cinema( ek );
  ek += tkin;
  modifiedOriginal.SetKineticEnergy( ek*MeV );
  G4double et = ek + amas;
  G4double p = std::sqrt( std::abs((et-amas)*(et+amas)) );
  G4double pp = modifiedOriginal.GetMomentum().mag()/MeV;
  if (pp > 0.0) {
    G4ThreeVector momentum = modifiedOriginal.GetMomentum();
    modifiedOriginal.SetMomentum( momentum * (p/pp) );
  }

  // calculate black track energies
  tkin = targetNucleus.EvaporationEffects(ek);
  ek -= tkin;
  modifiedOriginal.SetKineticEnergy( ek*MeV );
  et = ek + amas;
  p = std::sqrt( std::abs((et-amas)*(et+amas)) );
  pp = modifiedOriginal.GetMomentum().mag()/MeV;
  if (pp > 0.0) {
    G4ThreeVector momentum = modifiedOriginal.GetMomentum();
    modifiedOriginal.SetMomentum( momentum * (p/pp) );
  }
  G4ReactionProduct currentParticle = modifiedOriginal;
  G4ReactionProduct targetParticle;
  targetParticle = *originalTarget;
  currentParticle.SetSide( 1 ); // incident always goes in forward hemisphere
  targetParticle.SetSide( -1 );  // target always goes in backward hemisphere
  G4bool incidentHasChanged = false;
  G4bool targetHasChanged = false;
  G4bool quasiElastic = false;
  G4FastVector<G4ReactionProduct,GHADLISTSIZE> vec;  // vec will contain the secondary particles
  G4int vecLen = 0;
  vec.Initialize(0);
    
  const G4double cutOff = 0.1;
  const G4double anni = std::min( 1.3*currentParticle.GetTotalMomentum()/GeV, 0.4 );
  if ((currentParticle.GetKineticEnergy()/MeV > cutOff) || (G4UniformRand() > anni) )
    Cascade(vec, vecLen, originalIncident, currentParticle, targetParticle,
            incidentHasChanged, targetHasChanged, quasiElastic);
    
  CalculateMomenta(vec, vecLen, originalIncident, originalTarget,
                   modifiedOriginal, targetNucleus, currentParticle,
                   targetParticle, incidentHasChanged, targetHasChanged,
                   quasiElastic);
    
  SetUpChange(vec, vecLen, currentParticle, targetParticle, incidentHasChanged);

  if (isotopeProduction) DoIsotopeCounting(originalIncident, targetNucleus);

  delete originalTarget;
  return &theParticleChange;
}

 
void G4LEAntiXiMinusInelastic::Cascade(
   G4FastVector<G4ReactionProduct,GHADLISTSIZE> &vec,
   G4int& vecLen,
   const G4HadProjectile *originalIncident,
   G4ReactionProduct &currentParticle,
   G4ReactionProduct &targetParticle,
   G4bool &incidentHasChanged,
   G4bool &targetHasChanged,
   G4bool &quasiElastic )
{
    // derived from original FORTRAN code CASAXM by H. Fesefeldt (17-Jan-1989)
    // which is just a copy of casxm (cascade for Xi-).
    //
    // AntiXiMinus undergoes interaction with nucleon within a nucleus.  Check if it is
    // energetically possible to produce pions/kaons.  In not, assume nuclear excitation
    // occurs and input particle is degraded in energy. No other particles are produced.
    // If reaction is possible, find the correct number of pions/protons/neutrons
    // produced using an interpolation to multiplicity data.  Replace some pions or
    // protons/neutrons by kaons or strange baryons according to the average
    // multiplicity per Inelastic reaction.
    //
    const G4double mOriginal = originalIncident->GetDefinition()->GetPDGMass()/MeV;
    const G4double etOriginal = originalIncident->GetTotalEnergy()/MeV;
    const G4double targetMass = targetParticle.GetMass()/MeV;
    G4double centerofmassEnergy = std::sqrt( mOriginal*mOriginal +
                                        targetMass*targetMass +
                                        2.0*targetMass*etOriginal );
    G4double availableEnergy = centerofmassEnergy-(targetMass+mOriginal);
    if( availableEnergy <= G4PionPlus::PionPlus()->GetPDGMass()/MeV )
    {
      quasiElastic = true;
      return;
    }
    static G4bool first = true;
    const G4int numMul = 1200;
    const G4int numSec = 60;
    static G4double protmul[numMul], protnorm[numSec]; // proton constants
    static G4double neutmul[numMul], neutnorm[numSec]; // neutron constants
    // npos = number of pi+, nneg = number of pi-, nzero = number of pi0
    G4int counter, nt=0, npos=0, nneg=0, nzero=0;
    G4double test;
    const G4double c = 1.25;    
    const G4double b[] = { 0.7, 0.7 };
    if( first )       // compute normalization constants, this will only be Done once
    {
      first = false;
      G4int i;
      for( i=0; i<numMul; ++i )protmul[i] = 0.0;
      for( i=0; i<numSec; ++i )protnorm[i] = 0.0;
      counter = -1;
      for( npos=0; npos<(numSec/3); ++npos )
      {
        for( nneg=std::max(0,npos-1); nneg<=(npos+1); ++nneg )
        {
          for( nzero=0; nzero<numSec/3; ++nzero )
          {
            if( ++counter < numMul )
            {
              nt = npos+nneg+nzero;
              if( nt>0 && nt<=numSec )
              {
                protmul[counter] = Pmltpc(npos,nneg,nzero,nt,b[0],c);
                protnorm[nt-1] += protmul[counter];
              }
            }
          }
        }
      }
      for( i=0; i<numMul; ++i )neutmul[i] = 0.0;
      for( i=0; i<numSec; ++i )neutnorm[i] = 0.0;
      counter = -1;
      for( npos=0; npos<numSec/3; ++npos )
      {
        for( nneg=npos; nneg<=(npos+2); ++nneg )
        {
          for( nzero=0; nzero<numSec/3; ++nzero )
          {
            if( ++counter < numMul )
            {
              nt = npos+nneg+nzero;
              if( nt>0 && nt<=numSec )
              {
                neutmul[counter] = Pmltpc(npos,nneg,nzero,nt,b[1],c);
                neutnorm[nt-1] += neutmul[counter];
              }
            }
          }
        }
      }
      for( i=0; i<numSec; ++i )
      {
        if( protnorm[i] > 0.0 )protnorm[i] = 1.0/protnorm[i];
        if( neutnorm[i] > 0.0 )neutnorm[i] = 1.0/neutnorm[i];
      }
    }   // end of initialization
    
    const G4double expxu = 82.;           // upper bound for arg. of exp
    const G4double expxl = -expxu;        // lower bound for arg. of exp
    G4ParticleDefinition *aNeutron = G4Neutron::Neutron();
    G4ParticleDefinition *aProton = G4Proton::Proton();
    G4ParticleDefinition *aKaonMinus = G4KaonMinus::KaonMinus();
    G4ParticleDefinition *aSigmaPlus = G4SigmaPlus::SigmaPlus();
    G4ParticleDefinition *aXiZero = G4XiZero::XiZero();
    //
    // energetically possible to produce pion(s)  -->  inelastic scattering
    //
    G4double n, anpn;
    GetNormalizationConstant( availableEnergy, n, anpn );
    G4double ran = G4UniformRand();
    G4double dum, excs = 0.0;
    if( targetParticle.GetDefinition() == aProton )
    {
      counter = -1;
      for( npos=0; npos<numSec/3 && ran>=excs; ++npos )
      {
        for( nneg=std::max(0,npos-1); nneg<=(npos+1) && ran>=excs; ++nneg )
        {
          for( nzero=0; nzero<numSec/3 && ran>=excs; ++nzero )
          {
            if( ++counter < numMul )
            {
              nt = npos+nneg+nzero;
              if( nt>0 && nt<=numSec )
              {
                test = std::exp( std::min( expxu, std::max( expxl, -(pi/4.0)*(nt*nt)/(n*n) ) ) );
                dum = (pi/anpn)*nt*protmul[counter]*protnorm[nt-1]/(2.0*n*n);
                if( std::fabs(dum) < 1.0 )
                {
                  if( test >= 1.0e-10 )excs += dum*test;
                }
                else
                  excs += dum*test;
              }
            }
          }
        }
      }
      if( ran >= excs )  // 3 previous loops continued to the end
      {
        quasiElastic = true;
        return;
      }
      npos--; nneg--; nzero--;
      //
      // number of secondary mesons determined by kno distribution
      // check for total charge of final state mesons to determine
      // the kind of baryons to be produced, taking into account
      // charge and strangeness conservation
      //
      if( npos < nneg )
      {
        if( npos+1 == nneg )
        {
          currentParticle.SetDefinitionAndUpdateE( aXiZero );
          incidentHasChanged = true;
        }
        else   // charge mismatch
        {
          currentParticle.SetDefinitionAndUpdateE( aSigmaPlus );
          incidentHasChanged = true;
          //
          // correct the strangeness by replacing a pi- by a kaon-
          //
          vec.Initialize( 1 );
          G4ReactionProduct *p = new G4ReactionProduct;
          p->SetDefinition( aKaonMinus );
          (G4UniformRand() < 0.5) ? p->SetSide( -1 ) : p->SetSide( 1 );
          vec.SetElement( vecLen++, p );
          --nneg;
        }
      }
      else if( npos == nneg )
      {
        if( G4UniformRand() >= 0.5 )
        {
          currentParticle.SetDefinitionAndUpdateE( aXiZero );
          incidentHasChanged = true;
          targetParticle.SetDefinitionAndUpdateE( aNeutron );
          targetHasChanged = true;
        }             
      }
      else
      {
        targetParticle.SetDefinitionAndUpdateE( aNeutron );
        targetHasChanged = true;
      }
    }
    else  // target must be a neutron
    {
      counter = -1;
      for( npos=0; npos<numSec/3 && ran>=excs; ++npos )
      {
        for( nneg=npos; nneg<=(npos+2) && ran>=excs; ++nneg )
        {
          for( nzero=0; nzero<numSec/3 && ran>=excs; ++nzero )
          {
            if( ++counter < numMul )
            {
              nt = npos+nneg+nzero;
              if( nt>0 && nt<=numSec )
              {
                test = std::exp( std::min( expxu, std::max( expxl, -(pi/4.0)*(nt*nt)/(n*n) ) ) );
                dum = (pi/anpn)*nt*neutmul[counter]*neutnorm[nt-1]/(2.0*n*n);
                if( std::fabs(dum) < 1.0 )
                {
                  if( test >= 1.0e-10 )excs += dum*test;
                }
                else
                  excs += dum*test;
              }
            }
          }
        }
      }
      if( ran >= excs )  // 3 previous loops continued to the end
      {
        quasiElastic = true;
        return;
      }
      npos--; nneg--; nzero--;
      if( npos+1 < nneg )
      {
        if( npos+2 == nneg )
        {
          currentParticle.SetDefinitionAndUpdateE( aXiZero );
          incidentHasChanged = true;
          targetParticle.SetDefinitionAndUpdateE( aProton );
          targetHasChanged = true;
        }
        else   // charge mismatch
        {
          currentParticle.SetDefinitionAndUpdateE( aSigmaPlus );
          incidentHasChanged = true;
          targetParticle.SetDefinitionAndUpdateE( aProton );
          targetHasChanged = true;
          //
          // correct the strangeness by replacing a pi- by a kaon-
          //
          vec.Initialize( 1 );
          G4ReactionProduct *p = new G4ReactionProduct;
          p->SetDefinition( aKaonMinus );
          (G4UniformRand() < 0.5) ? p->SetSide( -1 ) : p->SetSide( 1 );
          vec.SetElement( vecLen++, p );
          --nneg;
        }
      }
      else if( npos+1 == nneg )
      {
        if( G4UniformRand() < 0.5 )
        {
          currentParticle.SetDefinitionAndUpdateE( aXiZero );
          incidentHasChanged = true;
        }
        else
        {
          targetParticle.SetDefinitionAndUpdateE( aProton );
          targetHasChanged = true;
        }
      }
    }
    SetUpPions( npos, nneg, nzero, vec, vecLen );
    return;
}

 /* end of file */
 
