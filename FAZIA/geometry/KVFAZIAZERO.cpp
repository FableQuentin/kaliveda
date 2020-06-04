//Created by KVClassFactory on Tue Jan 27 11:38:09 2015
//Author: ,,,

#include "KVFAZIAZERO.h"
#include "KVUnits.h"
#include "KVFAZIABlock.h"
#include "TSystem.h"
#include "KVEnv.h"

#include <TGeoMatrix.h>

ClassImp(KVFAZIAZERO)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVFAZIAZERO</h2>
<h4>Description of the FAZIA set up</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVFAZIAZERO::KVFAZIAZERO()
{
   // Default constructor
}

KVFAZIAZERO::~KVFAZIAZERO()
{
   // Destructor
}

void KVFAZIAZERO::GetGeometryParameters()
{
   //Defined number of blocks, the distance from the target and the minimum polar angle
   fNblocks = 6;
   fFDist = 100.;
   fFThetaMin = 1.65;
}

void KVFAZIAZERO::PlasticDetectors()
{
   // Telescope for elastic scattering monitoring
   // Two 5mm diameter silicon detectors of 525um thickness
   // placed 2m20 from the target at theta=1.84deg phi=-90deg.
   // distance between centres of detectors = 1mm

   KVMaterial silicon("Si");

   const double radius = 2 * KVUnits::cm;

   const double thick = 525 * KVUnits::um;
   const double centre_dist = 6 * KVUnits::cm;

   TGeoVolume* pl1 = gGeoManager->MakeTube("PL1", silicon.GetGeoMedium(), 0., radius, thick / 2);
   TGeoVolume* pl2 = gGeoManager->MakeTube("PL2", silicon.GetGeoMedium(), 0., radius, thick / 2);
   TGeoVolumeAssembly* telpl1 = gGeoManager->MakeVolumeAssembly("STRUCT_PL");
   telpl1->AddNode(pl1, 1, new TGeoTranslation(0, 0, -centre_dist / 2));
   telpl1->AddNode(pl2, 2, new TGeoTranslation(0, 0, centre_dist / 2));
   gGeoManager->GetTopVolume()->AddNode(telpl1, 998, new TGeoTranslation(0, 0, 0));

//   TGeoVolume* pl2 = gGeoManager->MakeTube("PL2", silicon.GetGeoMedium(), 0., radius, thick / 2);
//   gGeoManager->GetTopVolume()->AddNode(pl2, 999, new TGeoTranslation(0,0,-centre_dist / 2));
}


void KVFAZIAZERO::BuildFAZIA()
{
   //Build geometry of FAZIASYM
   //All telescopes are : Si(300µm)-Si(500µm)-CsI(10cm)
   //No attempt has been made to implement real thicknesses
   //
   Info("BuildFAZIA", "Compact geometry, %f cm from target + 2BLK around 20 degrees",
        fFDist);

   TGeoVolume* top = gGeoManager->GetTopVolume();

   Double_t distance_block_cible = fFDist * KVUnits::cm;

   KVFAZIABlock* block = new KVFAZIABlock;

   Double_t theta = 0;
   Double_t phi = 0;

   Double_t theta_min = fFThetaMin;//smallest lab polar angle in degrees
   Double_t centre_hole = 2.*tan(theta_min * TMath::DegToRad()) * distance_block_cible;
   Double_t dx = (block->GetTotalSideWithBlindage()) / 2.;

   TVector3 centre;
   for (Int_t bb = 0; bb < fNblocks; bb += 1) {

      if (bb == 1)        centre.SetXYZ(-1 * (dx - centre_hole / 2), -dx - centre_hole / 2, distance_block_cible);
      else if (bb == 2)   centre.SetXYZ(-1 * (dx + centre_hole / 2), dx - centre_hole / 2, distance_block_cible);
      else if (bb == 3)   centre.SetXYZ(-1 * (-dx + centre_hole / 2), dx + centre_hole / 2, distance_block_cible);
      else if (bb == 0)   centre.SetXYZ(-1 * (-dx - centre_hole / 2), -dx + centre_hole / 2, distance_block_cible);
      else if (bb == 4)   centre.SetXYZ(0, -25, distance_block_cible);
      else if (bb == 5)   centre.SetXYZ(0, 25, distance_block_cible);
      else {
         Warning("BuildFAZIA", "Block position definition is done only for %d blocks", fNblocks);
      }
      theta = centre.Theta() * TMath::RadToDeg();
      phi = centre.Phi() * TMath::RadToDeg();
      printf("BLK #%d => theta=%1.2lf - phi=%1.2lf\n", bb, theta, phi);

      top->AddNode(block, bb,
                   KVMultiDetArray::GetVolumePositioningMatrix(
                      block->GetNominalDistanceTargetBlockCentre(distance_block_cible),
                      theta,
                      phi
                   )
                  );
   }

//   trans.SetDz(250* KVUnits::cm + thick_si1 / 2.);
//   h = trans;
//   ph = new TGeoHMatrix(h);
//   top->AddNode(block, 4, ph);


   // add telescope for elastic scattering monitoring
   PlasticDetectors();
   // Change default geometry import angular range for rutherford telescope
   SetGeometryImportParameters(.25, 1., 0.5) ;
}

void KVFAZIAZERO::SetNameOfDetectors(KVEnv& env)
{

   KVFAZIA::SetNameOfDetectors(env);
//   env.SetValue("RUTH_SI_1", "SI1-RUTH");
//   env.SetValue("RUTH_SI_2", "SI2-RUTH");

}
