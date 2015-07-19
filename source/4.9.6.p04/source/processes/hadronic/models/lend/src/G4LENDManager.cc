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
// Class Description
// Manager of LEND (Low Energy Nuclear Data) target (nucleus) 
// LEND is Geant4 interface for GIDI (General Interaction Data Interface) 
// which gives a discription of nuclear and atomic reactions, such as
//    Binary collision cross sections
//    Particle number multiplicity distributions of reaction products
//    Energy and angular distributions of reaction products
//    Derived calculational constants
// GIDI is developped at Lawrence Livermore National Laboratory
// Class Description - End

// 071025 First implementation done by T. Koi (SLAC/SCCS)
// 101118 Name modifications for release T. Koi (SLAC/PPA)

#include "G4LENDManager.hh"
#include "G4HadronicException.hh"

#include "G4Neutron.hh"

G4LENDManager* G4LENDManager::lend_manager = NULL;

G4LENDManager::G4LENDManager()
:verboseLevel( 0 )
{

   printBanner();

   if(!getenv("G4LENDDATA")) 
      throw G4HadronicException(__FILE__, __LINE__, " Please setenv G4LENDDATA to point to the LEND files." );

   G4String xmcf = getenv("G4LENDDATA");
   xmcf = xmcf+"/xmcf.n_1.map";

// for neutron

   G4GIDI* axLEND = new G4GIDI( 1 , xmcf );

   if ( proj_lend_map.find ( G4Neutron::Neutron() ) == proj_lend_map.end() )
   {
      proj_lend_map.insert ( std::pair < G4ParticleDefinition* , G4GIDI* > ( G4Neutron::Neutron() , axLEND ) ); 
   }
      
   v_lend_target.clear();

   ionTable = new G4IonTable();
   nistElementBuilder = new G4NistElementBuilder( 0 );

}
   


G4LENDManager::~G4LENDManager()
{
   
// deleting target
   for ( std::vector < lend_target >::iterator 
         it = v_lend_target.begin() ; it != v_lend_target.end() ; it++ )
   {
        (*it).lend->freeTarget( it->target ); 
   }

// deleting lend
   for ( std::map < G4ParticleDefinition* , G4GIDI* >::iterator 
         it = proj_lend_map.begin() ; it != proj_lend_map.end() ; it++ )
   {
      delete it->second;
   }

   delete ionTable;
   delete nistElementBuilder;

}



G4GIDI_target* G4LENDManager::GetLENDTarget( G4ParticleDefinition* proj , G4String evaluation , G4int iZ , G4int iA , G4int iM )
{

   G4GIDI_target* anLENDTarget = NULL;

   G4int iTarg = ionTable->GetNucleusEncoding( iZ , iA );
                                                     // G4double E=0.0, G4int J=0);

   for ( std::vector < lend_target >::iterator 
         it = v_lend_target.begin() ; it != v_lend_target.end() ; it++ )
   {
//    find the target
      if ( it->proj == proj && it->target_code == iTarg && it->evaluation == evaluation ) 
      {
         return it->target;
      }
   }

   if ( proj_lend_map.find ( proj ) == proj_lend_map.end() ) 
   {
      G4cout << proj->GetParticleName() << " is not supported by this LEND." << G4endl;
      return anLENDTarget; // return NULL 
   }

   G4GIDI* xlend = proj_lend_map.find ( proj ) -> second; 

   if ( xlend->isThisDataAvailable( evaluation, iZ, iA , iM ) )
   {

      if ( verboseLevel > 1 ) 
         G4cout << evaluation << " for " << ionTable->GetIonName( iZ , iA , 0 ) << " is exist in this LEND." << G4endl;

      anLENDTarget = xlend->readTarget( evaluation , iZ , iA , iM );

      lend_target new_target; 
      new_target.lend = xlend; 
      new_target.target = anLENDTarget; 
      new_target.proj = proj;
      new_target.evaluation = evaluation;
      new_target.target_code = iTarg;
      
      v_lend_target.push_back( new_target );

//    found EXACT
      return anLENDTarget;

   }
   else 
   {
//    NO EXACT DATA (Evaluatino & Z,A,M)
                                                                        // This is for ground state
      if ( verboseLevel > 1 ) 
         G4cout << evaluation << " for " << ionTable->GetIonName( iZ , iA , 0 ) << " is not exist in this LEND." << G4endl;

      std::vector< std::string >* available =  xlend->getNamesOfAvailableLibraries( iZ, iA , iM );
      if ( available->size() > 0 )
      {
//       EXACT Z,A,M but Evaluation is different 
         if ( verboseLevel > 1 ) 
         {
            G4cout << " However you can use following evaluation(s) for the target. " << G4endl;

            std::vector< std::string >::iterator its;
            for ( its = available->begin() ; its != available->end() ; its++ ) 
               G4cout << *its << G4endl;

            G4cout << G4endl;
         }
      }
//      
//    checking natural abundance data for Z
//
      else if ( xlend->isThisDataAvailable( evaluation, iZ, 0 , iM ) )
      {
//       EXACT natural abundance data for the evaluation 
         if ( verboseLevel > 1 ) 
            G4cout << " However you can use natural abundance data for the target. " << G4endl;
      }
      else
      {
         std::vector< std::string >* available_nat =  xlend->getNamesOfAvailableLibraries( iZ, 0 , iM );
//
         if ( available_nat->size() > 0 )
         {
//          EXACT natural abundance data for Z but differnet evaluation
            if ( verboseLevel > 1 ) 
            {
               G4cout << " However you can use following evaluation(s) for natural abundace of the target. " << G4endl;

               std::vector< std::string >::iterator its;
               for ( its = available_nat->begin() ; its != available_nat->end() ; its++ ) 
                  G4cout << *its << G4endl;
               G4cout << G4endl;
            }
         }
      }

//    return NULL if exact data is not available               
      return anLENDTarget; // return NULL   
   }
   
   return anLENDTarget; 
}



std::vector< G4String > G4LENDManager::IsLENDTargetAvailable ( G4ParticleDefinition* proj , G4int iZ , G4int iA , G4int iM )
{

   std::vector< G4String > answer;
   if ( proj_lend_map.find ( proj ) == proj_lend_map.end() ) 
   {
      G4cout << proj->GetParticleName() << " is not supported by this LEND." << G4endl;
      return answer; // return NULL 
   }

   G4GIDI* xlend = proj_lend_map.find ( proj ) -> second; 
   std::vector< std::string >* available =  xlend->getNamesOfAvailableLibraries( iZ, iA , iM );

   if ( available->size() > 0 )
   {
      std::vector< std::string >::iterator its;
      for ( its = available->begin() ; its != available->end() ; its++ ) 
         answer.push_back ( *its );
   }

   return answer;
}



void G4LENDManager::printBanner()
{
   G4cout << " <<BEGIN-copyright>> " << G4endl;
   G4cout << " Copyright (c) 2010, Lawrence Livermore National Security, LLC.  " << G4endl;
   G4cout << " Produced at the Lawrence Livermore National Laboratory " << G4endl;
   G4cout << " Written by Bret R. Beck, beck6@llnl.gov.  " << G4endl;
   G4cout << " CODE-461393 " << G4endl;
   G4cout << " All rights reserved.  " << G4endl;
   G4cout << "  " << G4endl;
   G4cout << " This file is part of GIDI. For details, see nuclear.llnl.gov.  " << G4endl;
   G4cout << " Please also read the \"Additional BSD Notice\" at nuclear.llnl.gov.  " << G4endl;
   G4cout << " " << G4endl;
   G4cout << " Redistribution and use in source and binary forms, with or without modification, " << G4endl;
   G4cout << " are permitted provided that the following conditions are met: " << G4endl;
   G4cout << " " << G4endl;
   G4cout << "      1) Redistributions of source code must retain the above copyright notice, " << G4endl;
   G4cout << "         this list of conditions and the disclaimer below.  " << G4endl;
   G4cout << "      2) Redistributions in binary form must reproduce the above copyright notice, " << G4endl;
   G4cout << "         this list of conditions and the disclaimer (as noted below) in the " << G4endl;
   G4cout << "          documentation and/or other materials provided with the distribution.  " << G4endl;
   G4cout << "      3) Neither the name of the LLNS/LLNL nor the names of its contributors may be " << G4endl;
   G4cout << "         used to endorse or promote products derived from this software without " << G4endl;
   G4cout << "         specific prior written permission.  " << G4endl;
   G4cout << " " << G4endl;
   G4cout << " THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\" AND ANY " << G4endl;
   G4cout << " EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES " << G4endl;
   G4cout << " OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT " << G4endl;
   G4cout << " SHALL LAWRENCE LIVERMORE NATIONAL SECURITY, LLC, THE U.S. DEPARTMENT OF ENERGY OR " << G4endl;
   G4cout << " CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR " << G4endl;
   G4cout << " CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS " << G4endl;
   G4cout << " OR SERVICES;  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED " << G4endl; 
   G4cout << " AND ON  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT " << G4endl; 
   G4cout << " (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, " << G4endl;
   G4cout << " EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  " << G4endl;
   G4cout << " <<END-copyright>> " << G4endl;
}


G4bool G4LENDManager::RequestChangeOfVerboseLevel( G4int newValue )
{
   G4bool result=false;
   if ( newValue >= verboseLevel) 
   {
      verboseLevel = newValue;
      result=true;
   }
   else
   {
      G4cout << "Since other LEND model or cross section have set the higher verbose level (" << verboseLevel << ") in LENDManager, you cannot change the value now." << G4endl; 
   }

   return result;
}
