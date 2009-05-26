/*******************************************************************************
$Id: KVDatime.cpp,v 1.7 2009/01/15 16:10:13 franklan Exp $
$Revision: 1.7 $
$Date: 2009/01/15 16:10:13 $
$Author: franklan $
*******************************************************************************/

#include "KVDatime.h"
#include "KVList.h"
#include "KVString.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "KVError.h"

ClassImp(KVDatime)

KVList *KVDatime::fmonths = 0;
Int_t KVDatime::ndatime = 0;

void KVDatime::init()
{
   //called by all ctors
   //sets up static list of months' names if not already done
   if (!fmonths) {
      fmonths = new KVList;
      fmonths->Add(new TObjString("JAN"));
      fmonths->Add(new TObjString("FEB"));
      fmonths->Add(new TObjString("MAR"));
      fmonths->Add(new TObjString("APR"));
      fmonths->Add(new TObjString("MAY"));
      fmonths->Add(new TObjString("JUN"));
      fmonths->Add(new TObjString("JUL"));
      fmonths->Add(new TObjString("AUG"));
      fmonths->Add(new TObjString("SEP"));
      fmonths->Add(new TObjString("OCT"));
      fmonths->Add(new TObjString("NOV"));
      fmonths->Add(new TObjString("DEC"));
   }
   ndatime++;
	fStr="";
}

KVDatime::KVDatime()
		: TDatime()
{
   init();
}

KVDatime::KVDatime(const Char_t * DateString, EKVDateFormat f)
		: TDatime()
{
   //if f = KVDatime::kGANACQ:
   //Decodes GANIL acquisition (INDRA) run-sheet format date into a TDatime
   //Format of date string is:
   //      29-SEP-2005 09:42:17.00
   //if f = KVDatime::kSQL:
   //Decodes SQL format date into a TDatime (i.e. same format as returned
   //by TDatime::AsSQLString(): "2007-05-02 14:52:18")
   
   init();
   switch(f){
      case kGANACQ:
         SetGanacqDate(DateString);
         break;
      case kSQL:
         SetSQLDate(DateString);
         break;
      default:
            Error(KV__ERROR(KVDatime), "Unknown date format");
   }
}


KVDatime::~KVDatime()
{
   ndatime--;
   if (!ndatime) {
      delete fmonths;
      fmonths = 0;
   }
}

void KVDatime::SetSQLDate(const Char_t * DateString)
{
   //Decodes SQL format date into a TDatime (i.e. same format as returned
   //by TDatime::AsSQLString(): "2007-05-02 14:52:18")
   Int_t Y,M,D,h,m,s;
   sscanf(DateString, "%4d-%02d-%02d %02d:%02d:%02d",
         &Y,&M,&D,&h,&m,&s);
   Set(Y,M,D,h,m,s);
}

void KVDatime::SetGanacqDate(const Char_t * GanacqDateString)
{
   //Decodes GANIL acquisition (INDRA) run-sheet format date into a TDatime
   //Format of date string is:
   //      29-SEP-2005 09:42:17.00
   //
	//If format is not respected, we set to current time & date
	
   KVString tmp(GanacqDateString);
   TObjArray *toks = tmp.Tokenize("-:. ");
	if( toks->GetEntries() != 7 ) {
   	// if format is correct, there should be 7 elements in toks
		delete toks;
		Error("SetGanacqDate", "Format is incorrect: %s (should be like \"29-SEP-2005 09:42:17.00\")",
				GanacqDateString);
		Set();
		return;
	}
   KV__TOBJSTRING_TO_INT(toks,0,day)
   TObject *mm =
       fmonths->FindObject(((TObjString *) toks->At(1))->String().Data());
   Int_t month = 0;
   if (mm)
      month = fmonths->IndexOf(mm) + 1;
   KV__TOBJSTRING_TO_INT(toks,2,year)
   KV__TOBJSTRING_TO_INT(toks,3,hour)
   KV__TOBJSTRING_TO_INT(toks,4,min)
   KV__TOBJSTRING_TO_INT(toks,5,sec)
   delete toks;
   Set(year, month, day, hour, min, sec);
}

const Char_t *KVDatime::AsGanacqDateString() const
{
   //Return date and time string with format "29-SEP-2005 09:42:17.00"
   //Copy the string immediately if you want to reuse/keep it
   return Form("%d-%s-%4d %02d:%02d:%02d.00",
               GetDay(),
               ((TObjString *) fmonths->At(GetMonth() - 1))->String().
               Data(), GetYear(), GetHour(), GetMinute(), GetSecond());
}

const Char_t* KVDatime::String(EKVDateFormat fmt)
{
	// Returns date & time as a string in required format:
	//  fmt = kCTIME (default)  :  ctime format e.g. Thu Apr 10 10:48:34 2008
	//  fmt = kSQL              :  SQL format e.g. 1997-01-15 20:16:28
	//  fmt = kGANACQ           :  GANIL acquisition format e.g. 29-SEP-2005 09:42:17.00
	switch(fmt){
		case kCTIME:
			fStr = AsString();
			break;
		case kSQL:
			fStr = AsSQLString();
			break;
		case kGANACQ:
			fStr = AsGanacqDateString();
			break;
		default:
			fStr = "";
	}
	return fStr.Data();
}
