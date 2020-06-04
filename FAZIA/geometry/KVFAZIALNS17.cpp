//Created by KVClassFactory on Tue Jan 27 11:38:09 2015
//Author: ,,,

#include "KVFAZIALNS17.h"
#include "KVUnits.h"
#include "KVFAZIABlock.h"
#include "TSystem.h"
#include "KVEnv.h"

#include <TGeoMatrix.h>

ClassImp(KVFAZIALNS17)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVFAZIALNS17</h2>
<h4>Description of the FAZIA set up</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVFAZIALNS17::KVFAZIALNS17()
{
   // Default constructor
}

KVFAZIALNS17::~KVFAZIALNS17()
{
   // Destructor
}

void KVFAZIALNS17::GetGeometryParameters()
{
   //Defined number of blocks, the distance from the target and the minimum polar angle
   fNblocks = 6;
   fFDist = 100.;
   fFThetaMin = 1.65;
}

void KVFAZIALNS17::RutherfordTelescope()
{
   // Telescope for elastic scattering monitoring
   // Two 5mm diameter silicon detectors of 525um thickness
   // placed 2m20 from the target at theta=1.84deg phi=-90deg.
   // distance between centres of detectors = 1mm

   KVMaterial silicon("Si");

   const double radius = 9 * KVUnits::mm / 2.;

   const double thick = 525 * KVUnits::um;
   const double centre_dist = 1 * KVUnits::mm;
   double total_thickness = thick + centre_dist;

   TGeoVolume* si_det = gGeoManager->MakeTube("DET_SI", silicon.GetGeoMedium(), 0., radius, thick / 2);

   TGeoVolumeAssembly* ruth_tel = gGeoManager->MakeVolumeAssembly("STRUCT_RUTH");

   ruth_tel->AddNode(si_det, 1, new TGeoTranslation(0, 0, -centre_dist / 2));
   ruth_tel->AddNode(si_det, 2, new TGeoTranslation(0, 0, centre_dist / 2));

   // front entrance of first detector at 2 metres from target
   const double distance = 2.27 * KVUnits::m + 0.5 * total_thickness;
   const double theta = 0.99;
   const double phi = -54.07;
   gGeoManager->GetTopVolume()->AddNode(ruth_tel, 1,
                                        GetVolumePositioningMatrix(distance, theta, phi)
                                       );
}


void KVFAZIALNS17::BuildFAZIA()
{
   //Build geometry of FAZIASYM
   //All telescopes are : Si(300?m)-Si(500?m)-CsI(10cm)
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
                   GetVolumePositioningMatrix(block->GetNominalDistanceTargetBlockCentre(distance_block_cible),
                                              theta, phi)
                  );
   }

   // add telescope for elastic scattering monitoring
   RutherfordTelescope();
   // Change default geometry import angular range for rutherford telescope
   SetGeometryImportParameters(.25, 1., 0.5) ;
}

void KVFAZIALNS17::SetNameOfDetectors(KVEnv& env)
{

   KVFAZIA::SetNameOfDetectors(env);
   env.SetValue("RUTH_SI_1", "SI1-RUTH");
   env.SetValue("RUTH_SI_2", "SI2-RUTH");

}
