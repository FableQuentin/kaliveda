#include "KVINDRAIDTelescope.h"
#include "KVINDRACodeMask.h"
#include "KVTelescope.h"

ClassImp(KVINDRAIDTelescope)
/////////////////////////////////////////////////////////////////////////////////////////
//KVINDRAIDTelescope
//
/////////////////////////////////////////////////////////////////////////////////////////
    KVINDRAIDTelescope::KVINDRAIDTelescope()
{
   init();
}

void KVINDRAIDTelescope::init()
{
   //default init
   fIDCode = 0;                 //default
   fECode = 0;                 //default
   fZminCode = kIDCode_Zmin;                 //default
}

KVINDRAIDTelescope::~KVINDRAIDTelescope()
{
}


//____________________________________________________________________________________
const Char_t *KVINDRAIDTelescope::GetName() const
{
   // Name of telescope given in the form Det1_Det2_Ring-numberTelescope-number
   // The detectors are signified by their TYPE names i.e. KVDetector::GetType
   //
   //Just a wrapper for GetArrayName to allow polymorphism
   return ((KVINDRAIDTelescope *) this)->GetArrayName();
}
const Char_t *KVINDRAIDTelescope::GetArrayName()
{
   // Name of telescope given in the form Det1_Det2_Ring-numberTelescope-number
   //where ring and telescope numbers are those of the smallest (in angular terms)
   //detector of the telescope (if both are the same size, either one will do).
   // The detectors are signified by their TYPE names i.e. KVDetector::GetType

   //in order to access angular dimensions of detectors, we need their KVTelescopes
   KVTelescope *de_det = GetDetector(1)->GetTelescope();
   KVTelescope *e_det = 0;
   if (GetSize() > 1)
      e_det = GetDetector(2)->GetTelescope();
   UInt_t ring, mod;
   if (!e_det || de_det->IsSmallerThan(e_det)) {
      ring = de_det->GetRingNumber();
      mod = de_det->GetNumber();
   } else {
      ring = e_det->GetRingNumber();
      mod = e_det->GetNumber();
   }
   SetName(GetDetector(1)->GetType());
   if (e_det) {
      fName.Append("_");
      fName.Append(GetDetector(2)->GetType());
   }
   SetType(fName.Data());
   fName.Append("_");
   TString num;
   num.Form("%02d%02d", ring, mod);
   fName += num;

   return fName.Data();
}
