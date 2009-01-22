/*
$Id: KVINDRAUpDater_e475s.cpp,v 1.3 2007/11/21 11:22:59 franklan Exp $
$Revision: 1.3 $
$Date: 2007/11/21 11:22:59 $
*/

//Created by KVClassFactory on Tue Sep 18 12:14:51 2007
//Author: Eric Bonnet

#include "KVINDRAUpDater_e475s.h"
#include "KVDBRun.h"
#include "KVDetector.h"
#include "KVCalibrator.h"
#include "KVDBParameterSet.h"
#include "KVINDRA.h"
#include "KVSilicon_e475s.h"
#include "KVChIo_e475s.h"

#include "TList.h"
#include "TString.h"

ClassImp(KVINDRAUpDater_e475s)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVINDRAUpDater_e475s</h2>
<h4>Sets run parameters for INDRA_e475s dataset</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVINDRAUpDater_e475s::KVINDRAUpDater_e475s()
{
   //Default constructor
}

KVINDRAUpDater_e475s::~KVINDRAUpDater_e475s()
{
   //Destructor
}

void KVINDRAUpDater_e475s::SetCalibParameters(KVDBRun* kvrun)
{
   
	KVRList *param_list = kvrun->GetLinks("Calibrations");

   if (!param_list)
      return;
   if (!param_list->GetSize())
      return;

   //KVDetector *kvd;
   KVDBParameterSet *kvps;
   KVCalibrator *kvc;
   TIter next_ps(param_list);
   TString str;

   // Setting all calibration parameters available
   while ((kvps = (KVDBParameterSet *) next_ps())) {    // boucle sur les parametres
      str = kvps->GetName();
		if (gIndra->GetDetector(str.Data())){
		   kvc = gIndra->GetDetector(str.Data())->GetCalibrator(kvps->GetTitle());
         if (!kvc) {
            if (gIndra->GetDetector(str.Data())->InheritsFrom("KVSilicon_e475s")) {
					((KVSilicon_e475s *)gIndra->GetDetector(str.Data()))->SetCalibrator(kvps);
            	kvc = ((KVSilicon_e475s *)gIndra->GetDetector(str.Data()))->GetCalibrator(kvps->GetTitle());
					kvc->SetStatus(kTRUE); 
         	}
				else if (gIndra->GetDetector(str.Data())->InheritsFrom("KVChIo_e475s")){
					((KVChIo_e475s *)gIndra->GetDetector(str.Data()))->SetCalibrator(kvps);
					kvc = ((KVChIo_e475s *)gIndra->GetDetector(str.Data()))->GetCalibrator(kvps->GetTitle());
					kvc->SetStatus(kTRUE); 
				}
				else {
					Warning("SetCalibParameters(KVDBRun*)",
                    "Calibrator %s %s not found ! ",
                    kvps->GetName(), kvps->GetTitle());
				}
			}
			else {
				 if (gIndra->GetDetector(str.Data())->InheritsFrom("KVSilicon_e475s"))
				 	((KVSilicon_e475s *)gIndra->GetDetector(str.Data()))->ChangeCalibParameters(kvps);	
				 if (gIndra->GetDetector(str.Data())->InheritsFrom("KVChIo_e475s"))
					((KVChIo_e475s *)gIndra->GetDetector(str.Data()))->ChangeCalibParameters(kvps);	
			}
		}                         //detector found
   }                            //boucle sur les parameters

}
