/*
$Id: KVSilicon_e475s.h,v 1.4 2008/02/21 11:12:00 franklan Exp $
$Revision: 1.4 $
$Date: 2008/02/21 11:12:00 $
*/

//Created by KVClassFactory on Wed Sep 19 13:46:35 2007
//Author: bonnet

#ifndef __KVSILICON_E475S_H
#define __KVSILICON_E475S_H

#include "KVSilicon.h"
#include "KVFunctionCal.h"
#include "KVDBParameterSet.h"

class KVSilicon_e475s : public KVSilicon
{
   
	protected:
	
	KVFunctionCal *fcalibPG;//!channel-energy function conversion (PG)
	KVFunctionCal *fcalibGG;//!channel-energy function conversion (GG)
   
	void init();
	
	public:
	
	KVSilicon_e475s();
	KVSilicon_e475s(Float_t thick);
   virtual ~KVSilicon_e475s(){};
	
	void SetCalibrators();
	void SetCalibrator(KVDBParameterSet *kvdbps);
	void ChangeCalibParameters(KVDBParameterSet *kvdbps);

	Double_t GetOriginalValue(Float_t to,TString signal);
	Double_t GetCalibratedValue(Float_t from,TString signal);

	KVFunctionCal *GetCalibratorBySignal(TString signal) const;
	Bool_t 	IsCalibrated() const;
	Bool_t	IsCalibratedBySignal(TString signal) const;

	Double_t	GetCalibratedEnergy();
	Double_t	GetEnergy();

   virtual Short_t GetCalcACQParam(KVACQParam*) const;
   
	ClassDef(KVSilicon_e475s,1)//derivation of KVSilicon class for E475s experiment

};

#endif
