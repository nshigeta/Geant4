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
// neutron_hp -- source file
// J.P. Wellisch, Nov-1996
// A prototype of the low energy neutron transport model.
//
// 080612 SampleDiscreteTwoBody contribution from Benoit Pirard and Laurent Desorgher (Univ. Bern) #3
//
#include "G4NeutronHPLegendreStore.hh"
#include "G4NeutronHPVector.hh"
#include "G4NeutronHPInterpolator.hh"
#include "G4NeutronHPFastLegendre.hh"
#include "Randomize.hh"
#include <iostream>



//080612TK contribution from Benoit Pirard and Laurent Desorgher (Univ. Bern) #3 
G4double G4NeutronHPLegendreStore::SampleDiscreteTwoBody (G4double anEnergy)
{
  G4double result;
  
  G4int i0;
  G4int low(0), high(0);
  G4NeutronHPFastLegendre theLeg;
  for (i0=0; i0<nEnergy; i0++)
  {
    high = i0;
    if(theCoeff[i0].GetEnergy()>anEnergy) break;
  }
  low = std::max(0, high-1);
  G4NeutronHPInterpolator theInt;
  G4double x, x1, x2;
  x = anEnergy;
  x1 = theCoeff[low].GetEnergy();
  x2 = theCoeff[high].GetEnergy();
  G4double theNorm = 0;
  G4double try01=0, try02=0;
  G4double max1, max2, costh;
  max1 = 0; max2 = 0;
  G4int l,m;
  for(i0=0; i0<601; i0++)
  {
      costh = G4double(i0-300)/300.;
      try01 = 0.5;
      for(m=0; m<theCoeff[low].GetNumberOfPoly() ; m++)
      {  
	  l=m+1;
	  try01 += (2.*l+1)/2.*theCoeff[low].GetCoeff(m)*theLeg.Evaluate(l, costh);
      } 
      if(try01>max1) max1=try01;
      try02 = 0.5;
      for(m=0; m<theCoeff[high].GetNumberOfPoly() ; m++)
      {
	  l=m+1;
	  try02 += (2.*l+1)/2.*theCoeff[high].GetCoeff(m)*theLeg.Evaluate(l, costh);
      }
      if(try02>max2) max2=try02;
  } 
  theNorm = theInt.Interpolate(theManager.GetScheme(high), x, x1, x2, max1, max2);
  
  G4double value, random;
  G4double v1, v2;
  do
  {
    v1 = 0.5;
    v2 = 0.5;
    result = 2.*G4UniformRand()-1.;
    for(m=0; m<theCoeff[low].GetNumberOfPoly() ; m++)
    {
	l=m+1;	
	G4double legend = theLeg.Evaluate(l, result); // @@@ done to avoid optimization error on SUN
	v1 += (2.*l+1)/2.*theCoeff[low].GetCoeff(m)*legend;
    } 
    for(m=0; m<theCoeff[high].GetNumberOfPoly() ; m++)
    {	
	l=m+1;
	G4double legend = theLeg.Evaluate(l, result); // @@@ done to avoid optimization error on SUN
	v2 += (2.*l+1)/2.*theCoeff[high].GetCoeff(m)*legend;
    } 
    // v1 = std::max(0.,v1); // Workaround in case one of the distributions is fully non-physical.
    // v2 = std::max(0.,v2); 
    value = theInt.Interpolate(theManager.GetScheme(high), x, x1, x2, v1, v2);
    random = G4UniformRand();
    if(0>=theNorm) break; // Workaround for negative cross-section values. @@@@ 31 May 2000
  }
  while(random>value/theNorm);

  return result;
}



G4double G4NeutronHPLegendreStore::SampleMax (G4double anEnergy)
{
  G4double result;
  
  G4int i0;
  G4int low(0), high(0);
  G4NeutronHPFastLegendre theLeg;
  for (i0=0; i0<nEnergy; i0++)
  {
    high = i0;
    if(theCoeff[i0].GetEnergy()>anEnergy) break;
  }
  low = std::max(0, high-1);
  G4NeutronHPInterpolator theInt;
  G4double x, x1, x2;
  x = anEnergy;
  x1 = theCoeff[low].GetEnergy();
  x2 = theCoeff[high].GetEnergy();
  G4double theNorm = 0;
  G4double try01=0, try02=0;
  G4double max1, max2, costh;
  max1 = 0; max2 = 0;
  G4int l;
  for(i0=0; i0<601; i0++)
  {
    costh = G4double(i0-300)/300.;
    try01 = 0;
    for(l=0; l<theCoeff[low].GetNumberOfPoly() ; l++)
    {
      try01 += (2.*l+1)/2.*theCoeff[low].GetCoeff(l)*theLeg.Evaluate(l, costh);
    } 
    if(try01>max1) max1=try01;
    try02 = 0;
    for(l=0; l<theCoeff[high].GetNumberOfPoly() ; l++)
    {
      try02 += (2.*l+1)/2.*theCoeff[high].GetCoeff(l)*theLeg.Evaluate(l, costh);
    }
    if(try02>max2) max2=try02;
  } 
  theNorm = theInt.Interpolate(theManager.GetScheme(high), x, x1, x2, max1, max2);
  
  G4double value, random;
  G4double v1, v2;
  do
  {
    v1 = 0;
    v2 = 0;
    result = 2.*G4UniformRand()-1.;
    for(l=0; l<theCoeff[low].GetNumberOfPoly() ; l++)
    {
      G4double legend = theLeg.Evaluate(l, result); // @@@ done to avoid optimization error on SUN
      v1 += (2.*l+1)/2.*theCoeff[low].GetCoeff(l)*legend;
    } 
    for(l=0; l<theCoeff[high].GetNumberOfPoly() ; l++)
    {
      G4double legend = theLeg.Evaluate(l, result); // @@@ done to avoid optimization error on SUN
      v2 += (2.*l+1)/2.*theCoeff[high].GetCoeff(l)*legend;
    } 
    v1 = std::max(0.,v1); // Workaround in case one of the distributions is fully non-physical.
    v2 = std::max(0.,v2); 
    value = theInt.Interpolate(theManager.GetScheme(high), x, x1, x2, v1, v2);
    random = G4UniformRand();
    if(0>=theNorm) break; // Workaround for negative cross-section values. @@@@ 31 May 2000
  }
  while(random>value/theNorm);
  
  return result;
}


G4double G4NeutronHPLegendreStore::SampleElastic (G4double anEnergy)
{
  G4double result;
  
  G4int i0;
  G4int low(0), high(0);
  G4NeutronHPFastLegendre theLeg;
  for (i0=0; i0<nEnergy; i0++)
  {
    high = i0;
    if(theCoeff[i0].GetEnergy()>anEnergy) break;
  }
  low = std::max(0, high-1);
  G4NeutronHPInterpolator theInt;
  G4double x, x1, x2;
  x = anEnergy;
  x1 = theCoeff[low].GetEnergy();
  x2 = theCoeff[high].GetEnergy();
  G4double theNorm = 0;
  G4double try01=0, try02=0, try11=0, try12=0;
  G4double try1, try2;
  G4int l;
  for(l=0; l<theCoeff[low].GetNumberOfPoly(); l++)
  {
    try01 += (2.*l+1)/2.*theCoeff[low].GetCoeff(l)*theLeg.Evaluate(l, -1.);
    try11 += (2.*l+1)/2.*theCoeff[low].GetCoeff(l)*theLeg.Evaluate(l, +1.);
  } 
  for(l=0; l<theCoeff[high].GetNumberOfPoly(); l++)
  {
    try02 += (2.*l+1)/2.*theCoeff[high].GetCoeff(l)*theLeg.Evaluate(l, -1.);
    try12 += (2.*l+1)/2.*theCoeff[high].GetCoeff(l)*theLeg.Evaluate(l, +1.);
  } 
  try1 = theInt.Interpolate(theManager.GetScheme(high), x, x1, x2, try01, try02);
  try2 = theInt.Interpolate(theManager.GetScheme(high), x, x1, x2, try11, try12);
  theNorm = std::max(try1, try2);
  
  G4double value, random;
  G4double v1, v2;
  do
  {
    v1 = 0;
    v2 = 0;
    result = 2.*G4UniformRand()-1.;
    for(l=0; l<theCoeff[low].GetNumberOfPoly() ; l++)
    {
      G4double legend = theLeg.Evaluate(l, result); // @@@ done to avoid optimization error on SUN
      v1 += (2.*l+1)/2.*theCoeff[low].GetCoeff(l)*legend;
    } 
    for(l=0; l<theCoeff[high].GetNumberOfPoly() ; l++)
    {
      G4double legend = theLeg.Evaluate(l, result); // @@@ done to avoid optimization error on SUN
      v2 += (2.*l+1)/2.*theCoeff[high].GetCoeff(l)*legend;
    } 
    value = theInt.Interpolate(theManager.GetScheme(high), x, x1, x2, v1, v2);
    random = G4UniformRand();
  }
  while(random>value/theNorm);
  
  return result;
}

G4double G4NeutronHPLegendreStore::Sample (G4double energy) // still in interpolation; do not use
{
  G4int i0;
  G4int low(0), high(0);
//  G4cout << "G4NeutronHPLegendreStore::Sample "<<energy<<" "<<energy<<" "<<nEnergy<<G4endl;
  for (i0=0; i0<nEnergy; i0++)
  {
//     G4cout <<"theCoeff["<<i0<<"].GetEnergy() = "<<theCoeff[i0].GetEnergy()<<G4endl;
    high = i0;
    if(theCoeff[i0].GetEnergy()>energy) break;
  }
  low = std::max(0, high-1);
//  G4cout << "G4NeutronHPLegendreStore::Sample high, low: "<<high<<", "<<low<<G4endl;
  G4NeutronHPVector theBuffer;
  G4NeutronHPInterpolator theInt;
  G4double x1, x2, y1, y2, y;
  x1 = theCoeff[low].GetEnergy();
  x2 = theCoeff[high].GetEnergy();
//  G4cout << "the xes "<<x1<<" "<<x2<<G4endl;
  G4double costh=0;
  for(i0=0; i0<601; i0++)
  {
    costh = G4double(i0-300)/300.;
    y1 = Integrate(low, costh);
    y2 = Integrate(high, costh);
    y = theInt.Interpolate(theManager.GetScheme(high), energy, x1, x2, y1, y2);
    theBuffer.SetData(i0, costh, y);
//     G4cout << "Integration "<<low<<" "<<costh<<" "<<y1<<" "<<y2<<" "<<y<<G4endl;
  }
  G4double rand = G4UniformRand();
  G4int it;
  for (i0=1; i0<601; i0++)
  {
    it = i0;
    if(rand < theBuffer.GetY(i0)/theBuffer.GetY(600)) break;
//    G4cout <<"sampling now "<<i0<<" "
//         << theBuffer.GetY(i0)<<" "
//         << theBuffer.GetY(600)<<" "
//         << rand<<" "
//         << theBuffer.GetY(i0)/theBuffer.GetY(600)<<G4endl;;
  }
  if(it==601) it=600;
//  G4cout << "G4NeutronHPLegendreStore::Sample it "<<rand<<" "<<it<<G4endl;
  G4double norm = theBuffer.GetY(600);
  if(norm==0) return -DBL_MAX; 
  x1 = theBuffer.GetY(it)/norm;
  x2 = theBuffer.GetY(it-1)/norm;
  y1 = theBuffer.GetX(it);
  y2 = theBuffer.GetX(it-1);
//  G4cout << "G4NeutronHPLegendreStore::Sample x y "<<x1<<" "<<y1<<" "<<x2<<" "<<y2<<G4endl;
  return theInt.Interpolate(theManager.GetScheme(high), rand, x1, x2, y1, y2);
}

G4double G4NeutronHPLegendreStore::Integrate(G4int k, G4double costh) // still in interpolation; not used anymore
{
  G4double result=0;
  G4NeutronHPFastLegendre theLeg;
//  G4cout <<"the COEFFS "<<k<<" ";
//  G4cout <<theCoeff[k].GetNumberOfPoly()<<" ";
  for(G4int l=0; l<theCoeff[k].GetNumberOfPoly() ; l++)
  {
    result += theCoeff[k].GetCoeff(l)*theLeg.Integrate(l, costh);
//    G4cout << theCoeff[k].GetCoeff(l)<<" ";
  } 
//  G4cout <<G4endl;
  return result;
}