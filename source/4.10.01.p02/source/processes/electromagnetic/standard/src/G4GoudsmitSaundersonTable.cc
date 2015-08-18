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
// $Id: G4GoudsmitSaundersonTable.cc 78837 2014-01-28 08:44:36Z gcosmo $
//
// -------------------------------------------------------------------
//
// GEANT4 Header file
//
// File name:     G4GoudsmitSaundersonTable
//
// Author:        Omrane Kadri
//
// Creation date: 20.02.2009
//
// Modifications:
// 04.03.2009 V.Ivanchenko cleanup and format according to Geant4 EM style
// 26.08.2009 O.Kadri: avoiding unuseful calculations and optimizing the root 
//                     finding parameter error's within SampleTheta method
// 08.02.2010 O.Kadri: reduce delared variables; reduce error of finding root 
//                     in secant method
// 26.03.2010 O.Kadri: minimum of used arrays in computation within the dichotomie 
//                     finding method the error was the lowest value of uvalues
// 12.05.2010 O.Kadri: changing of sqrt((b-a)*(b-a)) with fabs(b-a)
//

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
  
#include "G4GoudsmitSaundersonTable.hh"
#include "G4Log.hh"
#include "G4ios.hh"
#include <fstream>

using namespace std;

static const G4double LAMBDAN[76]={1.0,1.1659144,1.3593564,1.5848932,1.8478498,2.1544347,2.5118864,2.9286446,
  3.4145489,3.9810717,4.6415888,5.4116953,6.3095734,7.3564225,8.576959,10,11.659144,13.593564,15.848932,
  18.478498,21.544347,25.118864,29.286446,34.145489,39.810717,46.415888,54.116953,63.095734,73.564225,
  85.76959,100,116.59144,135.93564,158.48932,184.78498,215.44347,251.18864,292.86446,341.45489,398.10717,
  464.15888,541.16953,630.95734,735.64225,857.6959,1000,1165.9144,1359.3564,1584.8932,1847.8498,2154.4347,
  2511.8864,2928.6446,3414.5489,3981.0717,4641.5888,5411.6953,6309.5734,7356.4225,8576.959,10000,
  11659.144,13593.564,15848.932,18478.498,21544.347,25118.864,29286.446,34145.489,39810.717,46415.888,
  54116.953,63095.734,73564.225,85769.59,100000.};

static const G4double scrA[76*11]={5.6991615e-05,0.00013145766,0.0003032221,0.00069941638,0.0016132837,0.0037212229,0.0085834253,0.01979865,0.045667845,0.1053381,0.24297434,
4.7936556e-05,0.00010863608,0.0002461962,0.00055794142,0.0012644331,0.0028655179,0.0064939715,0.014716944,0.033352231,0.075584397,0.17129292,
4.0337064e-05,9.0113601e-05,0.00020131513,0.00044974099,0.0010047281,0.0022445775,0.0050144194,0.011202288,0.025026077,0.055908629,0.12490071,
3.3955914e-05,7.4956228e-05,0.00016546267,0.00036525176,0.00080627763,0.0017798234,0.003928884,0.0086728433,0.019144931,0.042261615,0.093290708,
2.8595186e-05,6.2482352e-05,0.00013652803,0.00029832267,0.00065185455,0.0014243448,0.0031122867,0.00680055,0.014859647,0.0324693,0.070947545,
2.40896e-05,5.2174455e-05,0.00011300203,0.00024474542,0.00053008182,0.0011480776,0.0024865636,0.0053855232,0.011664234,0.025262979,0.054715818,
2.0301081e-05,4.3629599e-05,9.3765545e-05,0.00020151406,0.00043307928,0.00093074232,0.0020002834,0.0042988628,0.0092388016,0.019855357,0.042671682,
1.7114155e-05,3.6528558e-05,7.7966778e-05,0.00016641277,0.00035519244,0.00075812493,0.0016181465,0.0034537819,0.0073717733,0.015734358,0.033583511,
1.4432191e-05,3.0615355e-05,6.494509e-05,0.00013776959,0.00029225395,0.00061996534,0.0013151474,0.002789854,0.0059181845,0.012554388,0.026631925,
1.2174324e-05,2.5683001e-05,5.4180958e-05,0.00011430036,0.00024112847,0.00050868556,0.001073125,0.0022638687,0.004775866,0.010075185,0.02125465,
1.0272776e-05,2.156287e-05,4.5261124e-05,9.500448e-05,0.00019941731,0.00041858301,0.00087861852,0.0018442471,0.003871131,0.0081256222,0.017055929,
8.6707436e-06,1.8116946e-05,3.7854164e-05,7.9093776e-05,0.00016526123,0.00034530242,0.00072148662,0.0015074987,0.003149819,0.0065813388,0.013751273,
7.3205871e-06,1.5231781e-05,3.1692422e-05,6.5941709e-05,0.00013720343,0.00028547608,0.00059398363,0.0012358884,0.002571485,0.0053504309,0.011132521,
6.1823301e-06,1.2813803e-05,2.6558521e-05,5.5046502e-05,0.0001140921,0.00023647292,0.00049012546,0.0010158583,0.002105518,0.0043640006,0.0090450433,
5.2224117e-06,1.0785627e-05,2.2275102e-05,4.6003832e-05,9.5009779e-05,0.0001962197,0.0004052443,0.000836934,0.0017284846,0.0035697665,0.0073724887,
4.4126431e-06,9.0831013e-06,1.8696896e-05,3.8486186e-05,7.9220983e-05,0.00016307057,0.00033566877,0.00069094949,0.0014222687,0.0029276356,0.0060263225,
3.7293369e-06,7.652937e-06,1.570452e-05,3.2227096e-05,6.6132919e-05,0.00013571074,0.00027849072,0.00057148817,0.0011727455,0.0024065799,0.0049385198,
3.1525796e-06,6.4507944e-06,1.3199587e-05,2.7008936e-05,5.5265564e-05,0.00011308415,0.00023139229,0.00047347387,0.00096882012,0.0019823954,0.0040563687,
2.6656237e-06,5.4397244e-06,1.1100817e-05,2.2653381e-05,4.6228636e-05,9.4338538e-05,0.00019251616,0.00039286674,0.00080172111,0.0016360681,0.0033387157,
2.2543784e-06,4.5888959e-06,9.3409186e-06,1.901389e-05,3.8703689e-05,7.8783224e-05,0.00016036705,0.00032643485,0.00066447387,0.0013525686,0.0027532185,
1.9069837e-06,3.8725517e-06,7.8640717e-06,1.5969735e-05,3.2430074e-05,6.585643e-05,0.00013373603,0.00027158055,0.00055150431,0.0011199513,0.0022743086,
1.6134537e-06,3.2691521e-06,6.6238996e-06,1.3421231e-05,2.7193867e-05,5.5099745e-05,0.00011164215,0.00022620741,0.00045833755,0.00092867565,0.0018816666,
1.3653772e-06,2.7606661e-06,5.581811e-06,1.1285904e-05,2.2819053e-05,4.6138009e-05,9.3286778e-05,0.00018861722,0.00038136653,0.00077108777,0.001559068,
1.1556674e-06,2.331988e-06,4.7056515e-06,9.4953989e-06,1.9160492e-05,3.8663407e-05,7.8017777e-05,0.00015742983,0.00031767312,0.00064102346,0.0012935028,
9.7835084e-07,1.9704515e-06,3.9685959e-06,7.9929669e-06,1.6098268e-05,3.2422782e-05,6.5301237e-05,0.00013152022,0.00026488885,0.00053350047,0.0010744988,
8.2839142e-07,1.6654293e-06,3.3482418e-06,6.7314314e-06,1.3533123e-05,2.72075e-05,5.4698983e-05,0.0001099689,0.00022108561,0.00044447883,0.00089359698,
7.0154172e-07,1.4079982e-06,2.8258605e-06,5.6715182e-06,1.1382769e-05,2.2845283e-05,4.5850614e-05,9.2022444e-05,0.00018468957,0.00037067303,0.00074394288,
5.9421855e-07,1.1906618e-06,2.3857812e-06,4.7804943e-06,9.5788859e-06,1.9193634e-05,3.8459125e-05,7.7062235e-05,0.00015441298,0.00030940405,0.00061996646,
5.0339814e-07,1.007118e-06,2.0148796e-06,4.0310468e-06,8.0646696e-06,1.6134493e-05,3.2279297e-05,6.4579222e-05,0.00012919971,0.00025848199,0.00051712917,
4.2652819e-07,8.5206673e-07,1.7021564e-06,3.4003635e-06,6.7928372e-06,1.3569913e-05,2.7108341e-05,5.4153784e-05,0.00010818192,0.00021611283,0.00043172422,
3.6145372e-07,7.2104791e-07,1.4383863e-06,2.8693727e-06,5.7239836e-06,1.141852e-05,2.2778296e-05,4.5439406e-05,9.0645045e-05,0.00018082376,0.00036071725,
3.0635477e-07,6.1030666e-07,1.2158264e-06,2.4221165e-06,4.8252353e-06,9.6126241e-06,1.9149852e-05,3.8149503e-05,7.5999782e-05,0.00015140346,0.0003016194,
2.5969396e-07,5.166805e-07,1.0279744e-06,2.0452317e-06,4.0691409e-06,8.0958593e-06,1.6107316e-05,3.2046706e-05,6.3759311e-05,0.0001268539,0.0002523853,
2.2017227e-07,4.3750469e-07,8.693663e-07,1.7275192e-06,3.4327563e-06,6.8212359e-06,1.3554489e-05,2.6934148e-05,5.3520891e-05,0.00010635145,0.00021133115,
1.8669185e-07,3.7053306e-07,7.3540835e-07,1.4595876e-06,2.8968884e-06,5.7495434e-06,1.1411296e-05,2.2648349e-05,4.4950876e-05,8.9215386e-05,0.00017706852,
1.5832463e-07,3.138715e-07,6.2223622e-07,1.2335555e-06,2.4454686e-06,4.8480322e-06,9.6110072e-06,1.9053392e-05,3.7772498e-05,7.4882289e-05,0.00014845079,
1.3429812e-07,2.6594404e-07,5.2663605e-07,1.0428718e-06,2.0651483e-06,4.0895127e-06,8.0982631e-06,1.6036597e-05,3.1756493e-05,6.2885839e-05,0.00012452977,
1.1392341e-07,2.2535736e-07,4.4579019e-07,8.8183895e-07,1.7444079e-06,3.4506969e-06,6.8259889e-06,1.3502816e-05,2.6710567e-05,5.2837454e-05,0.0001045203,
9.665235e-08,1.9099697e-07,3.7743359e-07,7.4585536e-07,1.4739022e-06,2.9126127e-06,5.7556822e-06,1.1373938e-05,2.2476306e-05,4.4415955e-05,8.7771409e-05,
8.2010037e-08,1.6190202e-07,3.1962263e-07,6.3099043e-07,1.2456844e-06,2.4591968e-06,4.8548804e-06,9.5843749e-06,1.8921216e-05,3.7353758e-05,7.3742789e-05,
6.959459e-08,1.3726093e-07,2.7071877e-07,5.3393676e-07,1.0530798e-06,2.0769819e-06,4.096417e-06,8.0793348e-06,1.5934816e-05,3.1428127e-05,6.1985476e-05,
5.9065875e-08,1.1638807e-07,2.2934024e-07,4.5191014e-07,8.9047944e-07,1.754671e-06,3.4575425e-06,6.8130148e-06,1.3424903e-05,2.6453489e-05,5.2126045e-05,
5.0135964e-08,9.8704051e-08,1.9432138e-07,3.8256584e-07,7.5316789e-07,1.4827823e-06,2.9191944e-06,5.7470982e-06,1.131447e-05,2.2275109e-05,4.3853619e-05,
4.2561071e-08,8.3719116e-08,1.6467843e-07,3.2392823e-07,6.3717816e-07,1.2533517e-06,2.4653867e-06,4.8495019e-06,9.5391399e-06,1.8763822e-05,3.6909094e-05,
3.6134756e-08,7.1019192e-08,1.3958101e-07,2.7433229e-07,5.3917224e-07,1.0596883e-06,2.0827096e-06,4.0933542e-06,8.0450718e-06,1.5811771e-05,3.107643e-05,
3.0682173e-08,6.0254106e-08,1.1832791e-07,2.323741e-07,4.563397e-07,8.9616668e-07,1.7599054e-06,3.4561284e-06,6.7871961e-06,1.3328796e-05,2.6175288e-05,
2.6055209e-08,5.1127655e-08,1.0032685e-07,1.9686953e-07,3.8631344e-07,7.5805572e-07,1.4875187e-06,2.9189305e-06,5.7277633e-06,1.1239484e-05,2.2055032e-05,
2.2128376e-08,4.338923e-08,8.5077425e-08,1.6681947e-07,3.2709893e-07,6.4137425e-07,1.257604e-06,2.4659048e-06,4.8351362e-06,9.4807153e-06,1.8589748e-05,
1.8795341e-08,3.6826758e-08,7.2156716e-08,1.4138067e-07,2.7701501e-07,5.4277093e-07,1.0634813e-06,2.0837381e-06,4.0827839e-06,7.9996258e-06,1.5674112e-05,
1.5965985e-08,3.1260736e-08,6.1207222e-08,1.198412e-07,2.3464409e-07,4.5942338e-07,8.995319e-07,1.7612461e-06,3.4484467e-06,6.7519154e-06,1.321997e-05,
1.3563922e-08,2.6539201e-08,5.192666e-08,1.0159982e-07,1.9879044e-07,3.8895383e-07,7.6102795e-07,1.489029e-06,2.9134376e-06,5.7004385e-06,1.1153491e-05,
1.1524398e-08,2.2533483e-08,4.4059382e-08,8.6148651e-08,1.6844517e-07,3.2935832e-07,6.4398939e-07,1.2591828e-06,2.4620613e-06,4.8140317e-06,9.4128041e-06,
9.7925058e-09,1.9134608e-08,3.7389126e-08,7.3058552e-08,1.427568e-07,2.7894754e-07,5.4506498e-07,1.0650599e-06,2.0811326e-06,4.0665442e-06,7.946049e-06,
8.321689e-09,1.6250266e-08,3.1732878e-08,6.1966712e-08,1.2100615e-07,2.3629604e-07,4.6142958e-07,9.0106147e-07,1.7595573e-06,3.435994e-06,6.7096736e-06,
7.0724613e-09,1.3802258e-08,2.6935788e-08,5.2566522e-08,1.0258617e-07,2.0020197e-07,3.9070405e-07,7.6247826e-07,1.488014e-06,2.9039329e-06,5.6671686e-06,
6.0113304e-09,1.1724321e-08,2.2866768e-08,4.4598666e-08,8.6983916e-08,1.6965085e-07,3.3088199e-07,6.4534243e-07,1.2586568e-06,2.4548468e-06,4.7878605e-06,
5.109884e-09,9.9602974e-09,1.9414829e-08,3.7843808e-08,7.3765977e-08,1.4378625e-07,2.802713e-07,5.4631094e-07,1.0648812e-06,2.0756897e-06,4.0459796e-06,
4.3440169e-09,8.4625895e-09,1.6485991e-08,3.2116397e-08,6.2566028e-08,1.2188503e-07,2.3744451e-07,4.625662e-07,9.011263e-07,1.7554863e-06,3.4198669e-06,
3.6932746e-09,7.1908446e-09,1.400065e-08,2.7259414e-08,5.3074365e-08,1.0333635e-07,2.0119696e-07,3.917326e-07,7.6270751e-07,1.4849996e-06,2.8913098e-06,
3.1402991e-09,6.1108495e-09,1.1891377e-08,2.3139967e-08,4.5029104e-08,8.7624161e-08,1.705118e-07,3.318066e-07,6.456774e-07,1.2564527e-06,2.444988e-06,
2.6703582e-09,5.1935918e-09,1.010104e-08,1.964556e-08,3.8208739e-08,7.431235e-08,1.4453043e-07,2.8109789e-07,5.4670856e-07,1.063296e-06,2.0680092e-06,
2.2709466e-09,4.4144654e-09,8.5812256e-09,1.668094e-08,3.2425876e-08,6.3032266e-08,1.2252765e-07,2.3818001e-07,4.6299523e-07,9.0001078e-07,1.74952e-06,
1.9314485e-09,3.7525989e-09,7.2909004e-09,1.4165444e-08,2.752195e-08,5.347222e-08,1.0389083e-07,2.0184884e-07,3.9217081e-07,7.6194615e-07,1.4803803e-06,
1.6428508e-09,3.1902859e-09,6.1952822e-09,1.2030747e-08,2.3362756e-08,4.5368618e-08,8.8102257e-08,1.7108759e-07,3.3223854e-07,6.4518089e-07,1.2528901e-06,
1.3975005e-09,2.7125035e-09,5.2648821e-09,1.0218967e-08,1.9834686e-08,3.8498488e-08,7.4724326e-08,1.4503751e-07,2.8151315e-07,5.4640797e-07,1.0605603e-06,
1.1888988e-09,2.306504e-09,4.474696e-09,8.6810621e-09,1.6841555e-08,3.2673188e-08,6.3387093e-08,1.2297311e-07,2.3857201e-07,4.6283778e-07,8.97921e-07,
1.0115263e-09,1.9614686e-09,3.8035184e-09,7.37547e-09,1.4301904e-08,2.7733076e-08,5.3777699e-08,1.0428129e-07,2.022137e-07,3.9211616e-07,7.6035939e-07,
8.6069493e-10,1.6682144e-09,3.2333632e-09,6.2669628e-09,1.214674e-08,2.3543031e-08,4.5631526e-08,8.8443846e-08,1.7142346e-07,3.3225603e-07,6.4398462e-07,
7.3242257e-10,1.4189468e-09,2.7489734e-09,5.3256784e-09,1.0317616e-08,1.9988663e-08,3.8724708e-08,7.5022678e-08,1.4534395e-07,2.8157972e-07,5.4551385e-07,
6.2332613e-10,1.2070514e-09,2.337417e-09,4.5263342e-09,8.7651034e-09,1.6973346e-08,3.286835e-08,6.3648523e-08,1.2325336e-07,2.3867624e-07,4.6218901e-07,
5.3053132e-10,1.0269023e-09,1.9876836e-09,3.8473824e-09,7.4470363e-09,1.4414566e-08,2.7900995e-08,5.4005476e-08,1.045336e-07,2.023364e-07,3.9164459e-07,
4.5159583e-10,8.7373189e-10,1.6904661e-09,3.2706549e-09,6.3279493e-09,1.2243096e-08,2.3687516e-08,4.5829781e-08,8.8669866e-08,1.7155537e-07,3.3191936e-07,
3.8444428e-10,7.4348804e-10,1.4378533e-09,2.7807065e-09,5.3776896e-09,1.0400071e-08,2.0113002e-08,3.8897122e-08,7.5224283e-08,1.4547844e-07,2.8134501e-07,
3.2731284e-10,6.3272904e-10,1.2231296e-09,2.3644341e-09,4.5706918e-09,8.8356126e-09,1.7080139e-08,3.3017647e-08,6.3826472e-08,1.2338307e-07,2.3851203e-07,
2.7870239e-10,5.3853141e-10,1.0405942e-09,2.0107207e-09,3.8852779e-09,7.5074498e-09,1.4506505e-08,2.8030648e-08,5.4163097e-08,1.0465834e-07,2.0222935e-07,
2.3733868e-10,4.5841161e-10,8.8540646e-10,1.7101325e-09,3.3030629e-09,6.3797539e-09,1.2322278e-08,2.3800062e-08,4.5969012e-08,8.8787587e-08,1.7149021e-07
};

static const G4double uvalues[320]={0,7.61544e-09,3.04617e-08,6.85389e-08,1.21847e-07,1.90386e-07,2.74156e-07,
  3.73156e-07,4.87388e-07,6.1685e-07,7.61543e-07,9.21467e-07,1.09662e-06,1.28701e-06,1.49262e-06,
  1.71347e-06,1.94955e-06,2.20086e-06,2.4674e-06,2.74917e-06,3.04617e-06,3.3584e-06,3.68587e-06,
  4.02856e-06,4.38648e-06,4.75964e-06,5.14803e-06,5.55164e-06,5.97049e-06,6.40457e-06,6.85388e-06,
  7.31842e-06,7.79819e-06,8.29319e-06,8.80342e-06,9.32888e-06,9.86957e-06,1.04255e-05,1.09966e-05,
  1.1583e-05,1.21846e-05,1.28015e-05,1.34336e-05,1.40809e-05,1.47434e-05,1.54212e-05,1.61142e-05,
  1.68224e-05,1.75459e-05,1.82845e-05,1.90385e-05,7.61524e-05,0.000171338,0.000304586,0.000475889,
  0.000685233,0.000932601,0.00121797,0.00154133,0.00190265,0.0023019,0.00273905,0.00321407,0.00372692,
  0.00427757,0.00486597,0.00549207,0.00615583,0.0068572,0.00759612,0.00837255,0.00918641,0.0100376,
  0.0109262,0.011852,0.012815,0.013815,0.0148521,0.0159262,0.0170371,0.0181848,0.0193692,0.0205901,
  0.0218476,0.0231415,0.0244717,0.0258382,0.0272407,0.0286793,0.0301537,0.0316639,0.0332098,0.0347912,
  0.0364081,0.0380602,0.0397476,0.04147,0.0432273,0.0450194,0.0468461,0.0487074,0.050603,0.0525328,
  0.0544967,0.0564946,0.0585262,0.0605914,0.0626901,0.0648222,0.0669873,0.0691854,0.0714163,0.0736799,
  0.075976,0.0783043,0.0806647,0.0830571,0.0854812,0.0879369,0.090424,0.0929422,0.0954915,0.0980716,
  0.100682,0.103323,0.105995,0.108696,0.111427,0.114188,0.116978,0.119797,0.122645,0.125522,0.128428,
  0.131361,0.134323,0.137313,0.14033,0.143375,0.146447,0.149545,0.152671,0.155823,0.159001,0.162205,
  0.165435,0.16869,0.17197,0.175276,0.178606,0.181961,0.18534,0.188743,0.192169,0.195619,0.199092,
  0.202589,0.206107,0.209649,0.213212,0.216797,0.220404,0.224032,0.22768,0.23135,0.23504,0.238751,
  0.242481,0.246231,0.25,0.253788,0.257595,0.261421,0.265264,0.269126,0.273005,0.276901,0.280814,
  0.284744,0.288691,0.292653,0.296632,0.300625,0.304634,0.308658,0.312697,0.316749,0.320816,0.324896,
  0.32899,0.333097,0.337216,0.341348,0.345492,0.349647,0.353814,0.357992,0.362181,0.366381,0.37059,
  0.37481,0.379039,0.383277,0.387524,0.39178,0.396044,0.400316,0.404596,0.408882,0.413176,0.417476,
  0.421783,0.426095,0.430413,0.434737,0.439065,0.443398,0.447736,0.452077,0.456422,0.46077,0.465122,
  0.469476,0.473832,0.47819,0.48255,0.486912,0.491274,0.495637,0.5,0.508726,0.51745,0.526168,0.534878,
  0.543578,0.552264,0.560935,0.569587,0.578217,0.586824,0.595404,0.603956,0.612476,0.620961,0.62941,
  0.637819,0.646186,0.654508,0.662784,0.67101,0.679184,0.687303,0.695366,0.703368,0.711309,0.719186,
  0.726995,0.734736,0.742405,0.75,0.757519,0.76496,0.77232,0.779596,0.786788,0.793893,0.800908,0.807831,
  0.81466,0.821394,0.82803,0.834565,0.840999,0.847329,0.853553,0.85967,0.865677,0.871572,0.877355,
  0.883022,0.888573,0.894005,0.899318,0.904508,0.909576,0.914519,0.919335,0.924024,0.928584,0.933013,
  0.93731,0.941474,0.945503,0.949397,0.953154,0.956773,0.960252,0.963592,0.96679,0.969846,0.972759,
  0.975528,0.978152,0.980631,0.982963,0.985148,0.987185,0.989074,0.990814,0.992404,0.993844,0.995134,
  0.996273,0.997261,0.998097,0.998782,0.999315,0.999695,0.999924,1.0};

G4double* G4GoudsmitSaundersonTable::PDF  = 0;
G4double* G4GoudsmitSaundersonTable::CPDF = 0;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
G4GoudsmitSaundersonTable::G4GoudsmitSaundersonTable()
{
  if(PDF == 0){ 
    G4cout << "### G4GoudsmitSaundersonTable loading PDF data" << G4endl;

    PDF = new G4double [76*11*320];
    CPDF= new G4double [76*11*320];

    LoadPDFandCPDFdata();
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
G4GoudsmitSaundersonTable::~G4GoudsmitSaundersonTable()
{
  if(PDF) {
    delete [] PDF;
    delete [] CPDF;
    PDF = 0;
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
G4double G4GoudsmitSaundersonTable::SampleTheta(G4double lambda, G4double Chia2, G4double rndm)
{ 
  //Benedito's procedure
  G4double A[11],ThisPDF[320],ThisCPDF[320];
  G4double coeff,Ckj,CkjPlus1,CkPlus1j,CkPlus1jPlus1,a,b,mmm,F;
  G4int Ind0,KIndex=0,JIndex=0,IIndex=0;


  ///////////////////////////////////////////////////////////////////////////
  // Find Lambda and Chia2 Index
  for(G4int k=0;k<75;k++){if((lambda>=LAMBDAN[k])&&(lambda<LAMBDAN[k+1])){KIndex=k;break;}}  
  for(G4int j=0;j<11;j++){A[j]=scrA[11*KIndex+j];}  
  for(G4int j=0;j<10;j++){if((Chia2>=A[j])&&(Chia2<A[j+1])){JIndex=j;break;}}

  ///////////////////////////////////////////////////////////////////////////
  // Calculate some necessary coefficients for PDF and CPDF interpolation 
  coeff=(G4Log(LAMBDAN[KIndex+1]/LAMBDAN[KIndex]))*(G4Log(A[JIndex+1]/A[JIndex]));
  Ckj=(G4Log(LAMBDAN[KIndex+1]/lambda))*(G4Log(A[JIndex+1]/Chia2))/coeff;
  CkjPlus1=(G4Log(LAMBDAN[KIndex+1]/lambda))*(G4Log(Chia2/A[JIndex]))/coeff;
  CkPlus1j=(G4Log(lambda/LAMBDAN[KIndex]))*(G4Log(A[JIndex+1]/Chia2))/coeff;
  CkPlus1jPlus1=(G4Log(lambda/LAMBDAN[KIndex]))*(G4Log(Chia2/A[JIndex]))/coeff;
  ///////////////////////////////////////////////////////////////////////////
  // Calculate Interpolated PDF and CPDF arrays
  Ind0=320*(11*KIndex+JIndex);
  for(G4int i=0 ; i<320 ;i++){
    ThisPDF[i]=Ckj*PDF[Ind0]+CkjPlus1*PDF[Ind0+320]+CkPlus1j*PDF[Ind0+3520]+CkPlus1jPlus1*PDF[Ind0+3840];
    ThisCPDF[i]=Ckj*CPDF[Ind0]+CkjPlus1*CPDF[Ind0+320]+CkPlus1j*CPDF[Ind0+3520]+CkPlus1jPlus1*CPDF[Ind0+3840];  
  // Find u Index using secant method
    if((i!=0)&&((rndm>=ThisCPDF[i-1])&&(rndm<ThisCPDF[i])))  {IIndex=i-1;break;}
    Ind0++;
  }

  ///////////////////////////////////////////////////////////////////////////
  //CPDF^-1(rndm)=x ==> CPDF(x)=rndm;
  a=uvalues[IIndex];
  b=uvalues[IIndex+1];
  
  do{
    mmm=0.5*(a+b);
    F=(ThisCPDF[IIndex]+(mmm-uvalues[IIndex])*ThisPDF[IIndex]
       +((mmm-uvalues[IIndex])*(mmm-uvalues[IIndex])
	 *(ThisPDF[IIndex+1]-ThisPDF[IIndex]))
       /(2.*(uvalues[IIndex+1]-uvalues[IIndex])))-rndm;
    if(F>0.)b=mmm;
    else a=mmm;
  } while(std::fabs(b-a)>1.0e-9);

  return mmm;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void G4GoudsmitSaundersonTable::LoadPDFandCPDFdata()
{ 
  ///////////////////////////////////////
  //Probability and their cumulative loading data
  G4String filename;

  char* path = getenv("G4LEDATA");
  if (!path) {
    G4Exception("G4GoudsmitSaundersonTable::LoadPDFandCPDFdata()","em0006",
		FatalException,
		"Environment variable G4LEDATA not defined");
    return;
  }

  G4String pathString(path);

  for(G4int level = 0; level < 2; level++){
    if(level == 0) { filename = "PDF.dat"; }
    if(level == 1) { filename = "CPDF.dat"; }
 
    G4String dirFile = pathString + "/msc_GS/" + filename;
    
    std::ifstream infile(dirFile, std::ios::in);
    if( !infile.is_open()) {
      G4ExceptionDescription ed;
      ed << "Data file <" + dirFile + "> is not opened!";
      G4Exception("G4GoudsmitSaundersonTable::LoadPDFandCPDFdata()",
		  "em0003",FatalException,ed);
      return;
    }

    // Read parameters into tables. 
    G4double aRead;
    for(G4int k=0 ; k<76 ;k++){
      for(G4int j=0 ; j<11 ;j++){
        for(G4int i=0 ; i<320 ;i++) {
          infile >> aRead;
	  if(infile.fail()) {
	    G4ExceptionDescription ed;
	    ed << "Error reading <" + dirFile + "> k= " << k 
	       << "; j= " << j << "; i= " << i;
	    G4Exception("G4GoudsmitSaundersonTable::LoadPDFandCPDFdata()",
			"em0003",FatalException,ed);
	    return;
	  } else {
	    G4int idx = 320*(11*k+j)+i;
	    if(level == 0)       { PDF[idx]  = aRead; }
	    else if (level == 1) { CPDF[idx] = aRead; }
	  }
        }
      }
    }
    
    infile.close();
  } //End loading PDF and CPDF parameters
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
