/***************************************************************************
$Id: KVDBRun.h,v 1.15 2009/03/12 14:01:02 franklan Exp $
                          KVDBRun.h  -  description
                             -------------------
    begin                : jeu f�v 13 2003
    copyright            : (C) 2003 by Alexis Mignon
    email                : mignon@ganil.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef KV_DB_RUN_H
#define KV_DB_RUN_H

#include "KVDBSystem.h"
#include "KVTarget.h"
#include "KVString.h"
#include "TDatime.h"
#include "KVParameterList.h"
#include <RQ_OBJECT.h>

#define KV__GET_INT(__param) return (fIntPar.HasParameter(__param) ? fIntPar.GetParameter(__param) : 0);
#define KV__GET_DBL(__param) return (fFloatPar.HasParameter(__param) ? fFloatPar.GetParameter(__param) : 0);
#define KV__GET_STR(__param) return (fStringPar.HasParameter(__param) ? fStringPar.GetParameter(__param).Data() : "");
#define KV__SET_INT(__param,__val) fIntPar.SetParameter(__param,__val);
#define KV__SET_DBL(__param,__val) fFloatPar.SetParameter(__param,__val);
#define KV__SET_STR(__param,__val) fStringPar.SetParameter(__param,__val);

class KVDBRun:public KVDBRecord {

   RQ_OBJECT("KVDBRun")
	Bool_t fBlockSignals;//!

protected:

       KVParameterList<Int_t> fIntPar;//list of scalers for run
       KVParameterList<Double_t> fFloatPar;//list of floating point parameters
       KVParameterList<KVString> fStringPar;//list of string parameters
       TDatime fDatime;             //!set dynamically with date&time of ROOT file corresponding to run

 public:

  	void Modified() // *SIGNAL*
	{
		// Signal sent out when run properties change (used by GUI)
		if(!fBlockSignals) Emit("Modified()");
	};
	void BlockSignals(Bool_t yes=kTRUE)
	{
		// if yes=kTRUE, block 'Modified()' signal
		// if yes=kFALSE, allow 'Modified()' signal
		fBlockSignals = yes;
	};

       //Returns kTRUE if a parameter with the given name (either integer, floating point or string) has been set.
       //For string parameters, if the parameter exists we also check if the string is empty (unless check_whitespace=kFALSE)
       Bool_t Has(const Char_t* param, Bool_t check_whitespace=kTRUE) const {
         if(check_whitespace&&fStringPar.HasParameter(param)){
            return !(fStringPar.GetParameter(param).IsWhitespace());
         }
         return (fIntPar.HasParameter(param)||fFloatPar.HasParameter(param)||fStringPar.HasParameter(param));
       };

    KVDBRun();
    KVDBRun(Int_t number, const Char_t * title);
    virtual ~ KVDBRun();

    TDatime & GetDatime() {
      return fDatime;
   };
	const Char_t* GetDatimeString(){
		return fDatime.AsString();
	};
   void SetDatime(TDatime & dat) {
      dat.Copy(fDatime);
      Modified();
   };

   KVDBSystem *GetSystem() const;
	const Char_t *GetSystemName() const
	{
		return (GetSystem() ? GetSystem()->GetName() : "");
	};

   Int_t GetEvents() const { return GetScaler("Events"); };
   Double_t GetTime() const { return Get("Length (min.)"); };
   Double_t GetSize() const { return Get("Size (MB)"); };

   const Char_t *GetComments() const { return GetString("Comments"); };
   const Char_t *GetStartDate() const { return GetString("Start Date"); };
   const Char_t *GetDate() const {
      return GetStartDate();
   };
   const Char_t *GetEndDate() const { return GetString("End Date"); };

   //Return target used for this run (actually target of KVDBSystem associated to run)
   KVTarget *GetTarget() const
{
   return (GetSystem()? GetSystem()->GetTarget() : 0);
};

   void SetEvents(Int_t evt_number){ SetScaler("Events", evt_number);  };
   void SetTime(Double_t time){ Set("Length (min.)",time);  };
   void SetSize(Double_t s){ Set("Size (MB)",s);  };
   void SetComments(const KVString& comments){ Set("Comments",comments);  };
   void SetStartDate(const KVString& date){ Set("Start Date",date);  };
   void SetDate(const KVString& d) {
      SetStartDate(d);
   };
   void SetEndDate(const KVString& d) { Set("End Date",d); };

   virtual void SetSystem(KVDBSystem * system);
   virtual void UnsetSystem();

   virtual void SetNumber(Int_t n);

   virtual void Print(Option_t * option = "") const;

   virtual void WriteRunListLine(ostream &, Char_t delim = '|') const;
   virtual void ReadRunListLine(const KVString&);
   virtual void WriteRunListHeader(ostream &, Char_t delim = '|') const;

   //Set value for the scaler with the given name for this run
   virtual void SetScaler(const Char_t*name, Int_t val)
   {
      KV__SET_INT(name,val)
       Modified();
   };
   //Get value of scaler with the given name for this run
   virtual Int_t GetScaler(const Char_t*name) const
   {
      KV__GET_INT(name)
   };
   //Set numerical (non-scaler) characteristic of run
   void Set(const Char_t*param, Double_t val)
   {
      KV__SET_DBL(param,val)
       Modified();
   };
   //Get numerical (non-scaler) characteristic of run
   Double_t Get(const Char_t*param) const
   {
      KV__GET_DBL(param)
   };
   //Set characteristic of run
   void Set(const Char_t*param, const KVString& val)
   {
      KV__SET_STR(param,val)
       Modified();
   };
   //Get characteristic of run
   const Char_t* GetString(const Char_t*param) const
   {
      KV__GET_STR(param)
   };

   ClassDef(KVDBRun, 10)         //Base class for an experiment run
};
#endif
