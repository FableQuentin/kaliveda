/*
$Id: KVReconIdent.h,v 1.3 2007/06/08 15:49:10 franklan Exp $
$Revision: 1.3 $
$Date: 2007/06/08 15:49:10 $
*/

#ifndef KVReconIdent_h
#define KVReconIdent_h

#include "KVSelector.h"

class TFile;
class TTree;

class KVReconIdent:public KVSelector {

   protected:
   TFile *fIdentFile;           //new file
   TTree *fIdentTree;           //new tree
   Int_t fRunNumber;
   Int_t fEventNumber;

 public:
    KVReconIdent() {
      fIdentFile = 0;
      fIdentTree = 0;
   };
   virtual ~ KVReconIdent() {
   };

   virtual void InitRun();
   virtual void EndRun();
   virtual void InitAnalysis();
   virtual Bool_t Analysis();
   virtual void EndAnalysis();

   ClassDef(KVReconIdent, 0)    //Analysis class used to identify previously reconstructed events
};

#endif
