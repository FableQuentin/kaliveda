//Created by KVClassFactory on Sun Jan 20 13:31:10 2013
//Author: John Frankland,,,

#include "KVSimReader_ELIE.h"

ClassImp(KVSimReader_ELIE)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVSimReader_ELIE</h2>
<h4>Read ascii files containing events generated by Elie</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVSimReader_ELIE::KVSimReader_ELIE()
{
   // Default constructor
   init();
}

//________________________________________________________________

KVSimReader_ELIE::KVSimReader_ELIE(KVString filename) : KVSimReader()
{
   // Write your code here
   init();
	if (!OpenFileToRead(filename)) return;
	if (!ReadHeader()) return;
   SetROOTFileName(filename+".root");
   tree_title.Form("ELIE events %s + %s %.1f MeV/nuc.",
         proj.GetSymbol(),targ.GetSymbol(),ebeam);
	Run(root_file_name);
   SaveTree();
	CloseFile();
}

KVSimReader_ELIE::~KVSimReader_ELIE()
{
   // Destructor
}


void KVSimReader_ELIE::ReadFile()
{
	while (IsOK()){
		while (ReadEvent()){
			if (nevt && nevt%1000==0) Info("ReadFile","%d evts lus",nevt);
			if (HasToFill()) FillTree();
		}
	}	
}

Bool_t KVSimReader_ELIE::ReadHeader()
{
	// File header contains info on colliding nuclei
   // and number of simulated events:
   //
   // 54 129 50 119 32
   // 20000
   //
   // for 20000 events of 129Xe+119Sn@32MeV/nucleon
   
	Int_t res = ReadLineAndCheck(5," ");
	switch (res){
	case 0:
      Info("ReadHeader", "Can't read system");
		return kFALSE; 
	case 1:
		AddInfo("Aproj",GetReadPar(1));
		AddInfo("Zproj",GetReadPar(0));
		AddInfo("Atarg",GetReadPar(3));
		AddInfo("Ztarg",GetReadPar(2));
		AddInfo("Ebeam",GetReadPar(4));
      proj.SetZAandE(GetIntReadPar(0),GetIntReadPar(1),GetIntReadPar(1)*GetDoubleReadPar(4));
      targ.SetZandA(GetIntReadPar(2),GetIntReadPar(3));
      ebeam=GetDoubleReadPar(4);
		break;
	default:
		return kFALSE;	
	}
	
	res = ReadLineAndCheck(1," ");
	switch (res){
	case 0:
      Info("ReadHeader", "Can't read events");
		return kFALSE; 
	case 1:
		AddInfo("Nevents", GetReadPar(0));
      return kTRUE;
      
	default:
		return kFALSE;	
	}

	return kFALSE;	
}

Bool_t KVSimReader_ELIE::ReadEvent()
{
   // Events begin with event number, multiplicity and (?) impact parameter,
   // followed by list of particles:
   // 0 61 0
   // 0 11 24.0 57.4156777729 306.493830835 107.248038513
   // 1 2 3.0 44.7736647511 50.6065561553 127.526000603
   // 2 7 15.0 57.7807658304 121.672537111 55.9601768553
   // 3 9 19.0 35.5421741719 70.5034488937 236.567079783
   // etc.

	evt->Clear();

	Int_t res = ReadLineAndCheck(3," ");
   Int_t mult=0;
	switch (res){
	case 0:
		Info("ReadEvent","case 0 line est vide"); 
		return kFALSE; 
	case 1:
		evt->SetNumber(GetIntReadPar(0));
      mult=GetIntReadPar(1);
		evt->GetParameters()->SetValue("b",GetDoubleReadPar(2));
		break;
	default:		
		return kFALSE;	
	}
	for (Int_t mm=0; mm<mult; mm++){	
		nuc = (KVSimNucleus* )evt->AddParticle();
		if (!ReadNucleus()) return kFALSE;
	}
   nevt++;
	return kTRUE;	
}

Bool_t KVSimReader_ELIE::ReadNucleus()
{
   // Particles are given as:
   // 0 11 24.0 57.4156777729 306.493830835 107.248038513
   // # Z  A     theta(deg)      phi(deg)    energy(MeV)
   
	Int_t res = ReadLineAndCheck(6," ");
	switch (res){
	case 0:
		Info("ReadNucleus","case 0 line est vide"); 
		return kFALSE; 
	
	case 1:
		nuc->SetZ((int)GetDoubleReadPar(1));
		nuc->SetA((int)GetDoubleReadPar(2));
		nuc->SetEnergy(GetDoubleReadPar(5));
		nuc->SetTheta(GetDoubleReadPar(3));
		nuc->SetPhi(GetDoubleReadPar(4));
		return kTRUE;	
		break;

	default:
		return kFALSE;	
	}

	return kFALSE;	
}
