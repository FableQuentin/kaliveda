/***************************************************************************
$Id: KVBase.cpp,v 1.57 2009/04/22 09:38:39 franklan Exp $
                          kvbase.cpp  -  description
                             -------------------
    begin                : Thu May 16 2002
    copyright            : (C) 2002 by J.D. Frankland
    email                : frankland@ganil.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <cassert>
#include "Riostream.h"
#include "TMath.h"
#include "TFile.h"
#include "KVBase.h"
#include "TClass.h"
#include "KVString.h"
#include "TSystem.h"
#include "TInterpreter.h"
#include "TEnv.h"
#include "TPluginManager.h"
#include "KVNameValueList.h"
#include "TSystemDirectory.h"
#include "KVVersion.h"
#ifdef WITH_BZR_INFOS
#include "KVBzrInfo.h"
#endif
#ifdef WITH_GIT_INFOS
#include "KVGitInfo.h"
#endif
#include "TROOT.h"
#include "TDatime.h"
#include "THashList.h"
#include "TError.h"
#include "TGMimeTypes.h"
#include "TGClient.h"
#include "TContextMenu.h"
#include <TKey.h>
#include "TTree.h"
#ifdef WITH_GRULIB
#include "GNetClientRoot.h"
#endif

using namespace std;

ClassImp(KVBase)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVBase</h2>
<h4>Base class for KaliVeda framework</h4>
This is the base class for many classes in the KaliVeda framework. Each
KVBase object has:

 - a name - Get/SetName()
 - a type - Get/SetType()
 - a number - Get/SetNumber()
 - a label - Get/SetLabel()

When objects are accessed through a TObject/TNamed base pointer, it is possible
to test whether an object is derived from KVBase, using the bit
`KVBase::kIsKaliVedaObject`:

    TObject* ob = (address of some object)
    if( ob->TestBit( KVBase::kIsKaliVedaObject ) ){

This class also provides a number of general utilities, often as static
(stand-alone) methods.

<h3>KaliVeda build/installation information</h3>

The static methods

    KVBase::GetKVSourceDir()
    KVBase::GetKVBuildDate()
    KVBase::GetKVBuildType()
    KVBase::GetKVBuildUser()
    KVBase::GetKVVersion()

give info on the sources, when and where they were built, and by whom.

<h3>Initialisation</h3>

The entire KaliVeda framework is initialised by the static method KVBase::InitEnvironment()


<h3>Finding/opening files</h3>

Static methods for easily locating and/or opening files within the
KaliVeda installation tree are given:

    KVBase::SearchKVFile(...)
    KVBase::SearchAndOpenKVFile(...)

Note that in the second case, two methods exist: one for reading, the
other for writing the (ascii) files. A global function for searching
files is also defined:

    Bool_t SearchFile(const Char_t* name, TString&amp; fullpath, int ndirs, ...)

This will search for a
file in an arbitrary number of locations, return kTRUE if file is found
and put full path to file in 'fullpath':

 - 'name' is a filename (not an absolute pathname) i.e. "toto.dat"
 - 'fullpath' will contain the full path to the file if it is found (if file not found, fullpath="")
 - 'ndirs' is the number of directories to search in

The remaining arguments are the names of 'ndirs' paths to search in,

<h3>Finding class source files</h3>

Source files for a class can be found using static method

    KVBase::FindClassSourceFiles(...)

It will look for appropriately-named files corresponding to the header
and implementation file of a class, testing several popular suffixes
in each case.

<h3>Finding executables</h3>

To find an executable in the current user's 'PATH' (or elsewhere), use
static method

    KVBase::FindExecutable(...)

<h3>Temporary files</h3>

The static methods

    KVBase::GetTempFileName(...)
    KVBase::OpenTempFile(...)

can generate and handle uniquely-named temporary (ascii) files.

<h3>Backing-up files</h3>

The static method

    KVBase::BackupFileWithDate(...)

can be used to create a dated backup of an existing file before it is
replaced with a new version.

<h3>Handling plugins</h3>

As plugins are extensively used in the KaliVeda framework, a few
utilities for handling them are defined. They are static methods


    KVBase::LoadPlugin(...)
    KVBase::GetPluginURI(...)

<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

#define xstr(s) str(s)
#define str(s) #s
UInt_t KVBase::fNbObj = 0;
TString KVBase::fWorkingDirectory = "$(HOME)/.kaliveda";
Bool_t KVBase::fEnvIsInit = kFALSE;

const Char_t* KVBase::GetETCDIRFilePath(const Char_t* namefile)
{
   if (strcmp(namefile, "")) return Form("%s/%s", xstr(ETCDIR), namefile);
   return Form("%s", xstr(ETCDIR));
}
const Char_t* KVBase::GetDATADIRFilePath(const Char_t* namefile)
{
   if (strcmp(namefile, "")) return Form("%s/%s", xstr(DATADIR), namefile);
   return Form("%s", xstr(DATADIR));
}
const Char_t* KVBase::GetTEMPLATEDIRFilePath(const Char_t* namefile)
{
   if (strcmp(namefile, "")) return Form("%s/%s", xstr(TEMPLATEDIR), namefile);
   return Form("%s", xstr(TEMPLATEDIR));
}
const Char_t* KVBase::GetDATABASEFilePath()
{
   return Form("%s/db", fWorkingDirectory.Data());
}
const Char_t* KVBase::GetLIBDIRFilePath(const Char_t* namefile)
{
   if (strcmp(namefile, "")) return Form("%s/%s", xstr(LIBDIR), namefile);
   return Form("%s", xstr(LIBDIR));
}
const Char_t* KVBase::GetINCDIRFilePath(const Char_t* namefile)
{
   if (strcmp(namefile, "")) return Form("%s/%s", xstr(INCDIR), namefile);
   return Form("%s", xstr(INCDIR));
}
const Char_t* KVBase::GetBINDIRFilePath(const Char_t* namefile)
{
   if (strcmp(namefile, "")) return Form("%s/%s", xstr(BINDIR), namefile);
   return Form("%s", xstr(BINDIR));
}
const Char_t* KVBase::GetWORKDIRFilePath(const Char_t* namefile)
{
   if (strcmp(namefile, "")) return Form("%s/%s", fWorkingDirectory.Data(), namefile);
   return fWorkingDirectory;
}

//_______________
void KVBase::init()
{
   //Default initialisation

   InitEnvironment();
   fNumber = 0;
   fNbObj++;
   fLabel = "";
   SetBit(kIsKaliVedaObject);
}

void KVBase::InitEnvironment()
{
   // STATIC method to Initialise KaliVeda environment
   // Reads config files in <code>\$(pkgdatadir)/etc</code> and sets up environment
   // (data repositories, datasets, etc. etc.).
   // Adds directory where kaliveda shared libs are installed to the dynamic
   // path - for finding and loading plugins.
   // Resets the `gRandom` random number sequence using a clock-based seed
   // (i.e. random sequences do not repeat).
   //
   // Normally, the first object created which inherits from KVBase will
   // perform this initialisation; if you need to set up the environment before
   // creating a KVBase object, or if you just want to be absolutely sure that
   // the environment has been initialised, you can call this method.
   //
   // #### Note for GNU-style installations
   // If KaliVeda is built with the cmake option `-Dgnuinstall=yes` then each
   // user will have a working directory which will be used to store any files
   // generated by KaliVeda at runtime.
   //
   // By default the location of user's working directory is set to
   //~~~~~~~~~
   //    $(HOME)/.kaliveda
   //~~~~~~~~~
   // but can be changed with variable
   //~~~~~~~~~
   //    KaliVeda.WorkingDirectory:   [directory]
   //~~~~~~~~~
   // in configuration file `.kvrootrc`.
   //`[directory]` must be an absolute pathname, but can use shell variables like <code>\$(HOME)</code>.

   if (!fEnvIsInit) {//test if environment already initialised

      // Add path to kaliveda libraries to dynamic loader path
      // This is needed to find plugins
      // and also to be able to compile with kaliveda in the interpreter
      TString libdir = GetLIBDIRFilePath();
      gSystem->AddDynamicPath(libdir);
#ifdef ADD_DYN_PATH
      // add paths to utility libraries to dynamic path to avoid bug with on-demand class loading for ROOT5/CINT
      KVString add_dyn_path(ADD_DYN_PATH);
      add_dyn_path.Begin(";");
      while (!add_dyn_path.End()) {
         gSystem->AddDynamicPath(add_dyn_path.Next(kTRUE));
      }
#endif
      // force re-reading of rootmap files in new dynamic path
      gInterpreter->LoadLibraryMap();
      // Add path to kaliveda header files
      // This is needed to be able to compile with kaliveda in the interpreter
      TString incdir = GetINCDIRFilePath();
      incdir.Prepend("-I");
      gSystem->AddIncludePath(incdir);

      //set up environment using kvrootrc file
      if (!gEnv->Defined("DataSet.DatabaseFile")) {
         ReadConfigFiles();
      }
#ifdef WITH_GNU_INSTALL
      // set working directory & create if needed
      fWorkingDirectory = gEnv->GetValue("KaliVeda.WorkingDirectory", "$(HOME)/.kaliveda");
      gSystem->ExpandPathName(fWorkingDirectory);
      gSystem->mkdir(fWorkingDirectory, kTRUE);
#else
      // set environment variable used in database makefiles
      fWorkingDirectory = KV_ROOT;
#endif
      // set environment variable used in database makefiles
      gSystem->Setenv("KV_WORK_DIR", fWorkingDirectory);

      //generate new seed from system clock
      gRandom->SetSeed(0);

      // initialisation has been performed
      fEnvIsInit = kTRUE;

   }
}

void KVBase::ReadConfigFiles()
{
   // Read all configuration files
   // System config files are read first in the order they appear in file
   //    ${ETCDIR}/config.files
   // Then we read any of the following files if they exist:
   //    ${HOME}/.kvrootrc
   //    ${PWD}/.kvrootrc

   TString tmp = GetETCDIRFilePath("config.files");
   ifstream conflist;
   conflist.open(tmp.Data());
   if (!conflist.good()) {
      ::Fatal("KVBase::ReadConfigFiles", "Cannot open %s", tmp.Data());
      return;
   }
   KVString file;
   file.ReadLine(conflist);
   conflist.close();
   file.Begin(";");
   while (!file.End()) {
      tmp = GetETCDIRFilePath(file.Next().Data());
      //skip over any missing files - this is needed when installing from
      //e.g. ubuntu packages if not all packages are installed
      if (!gSystem->AccessPathName(tmp.Data())) gEnv->ReadFile(tmp.Data(), kEnvChange);
   }

   AssignAndDelete(tmp, gSystem->ConcatFileName(gSystem->Getenv("HOME"), ".kvrootrc"));
   gEnv->ReadFile(tmp.Data(), kEnvChange);

   tmp = "./.kvrootrc";
   gEnv->ReadFile(tmp.Data(), kEnvChange);

   // load plugin handlers
   gROOT->GetPluginManager()->LoadHandlersFromEnv(gEnv);

   // load mime types/icon definitions when not in batch (i.e. GUI-less) mode
   if (!gROOT->IsBatch()) ReadGUIMimeTypes();
}

//_______________________________________________________________________________
KVBase::KVBase()
{
   //Default constructor.
   init();
}

//_______________________________________________________________________________
KVBase::KVBase(const Char_t* name, const Char_t* type): TNamed(name, type)
{
   //Ctor for object with given name and type.
   init();
}

//______________________
KVBase::KVBase(const KVBase& obj) : TNamed()
{
   //copy ctor
   init();
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   obj.Copy(*this);
#else
   ((KVBase&) obj).Copy(*this);
#endif
}

//_______________________________________________________________________________
KVBase::~KVBase()
{
   fNbObj--;
}

//_______________________________________________________________________________

void KVBase::Clear(Option_t* opt)
{
   //Clear object properties : name, type/title, number, label
   TNamed::Clear(opt);
   fNumber = 0;
   fLabel = "";
}

//___________________________________________________________________________________
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
void KVBase::Copy(TObject& obj) const
#else
void KVBase::Copy(TObject& obj)
#endif
{
   //Make a copy of this object

   TNamed::Copy(obj);
   ((KVBase&) obj).SetNumber(fNumber);
   ((KVBase&) obj).SetLabel(fLabel);
}

//____________________________________________________________________________________
void KVBase::Print(Option_t*) const
{
   cout << "KVBase object: Name=" << GetName() << " Type=" << GetType();
   if (fLabel != "")
      cout << " Label=" << GetLabel();
   if (fNumber != 0)
      cout << " Number=" << GetNumber();
   cout << endl;
}

void KVBase::Streamer(TBuffer& R__b)
{
   //Backwards compatible streamer for KVBase objects
   //Needed to handle 'fLabel' char array in class version 1
   //Objects written with version < 3 did not have kIsKaliVedaObject bit set,
   //we set it here when reading object.

   if (R__b.IsReading()) {
      UInt_t R__s, R__c;
      Version_t R__v = R__b.ReadVersion(&R__s, &R__c);
      if (R__v > 1) {
         if (R__v < 4) {
            TNamed::Streamer(R__b);
            R__b >> fNumber;
            R__b >> fLabel;
            if (R__v < 3) SetBit(kIsKaliVedaObject);
            R__b.CheckByteCount(R__s, R__c, KVBase::IsA());
         }
         else {
            //AUTOMATIC STREAMER EVOLUTION FOR CLASS VERSION > 3
            KVBase::Class()->ReadBuffer(R__b, this, R__v, R__s, R__c);
         }
         return;
      }
      //OLD STREAMER FOR CLASS VERSION 1
      TNamed::Streamer(R__b);
      R__b >> fNumber;
      UInt_t LabelLength;
      R__b >> LabelLength;
      if (LabelLength) {
         Char_t* Label = new Char_t[LabelLength];
         R__b.ReadFastArray(Label, LabelLength);
         fLabel = Label;
         delete[]Label;
      }
      SetBit(kIsKaliVedaObject);
      R__b.CheckByteCount(R__s, R__c, KVBase::IsA());
   }
   else {
      KVBase::Class()->WriteBuffer(R__b, this);
   }
}

//________________________________________________________________________________//

Bool_t SearchFile(const Char_t* name, TString& fullpath, int ndirs, ...)
{
   //Search for file in an arbitrary number of locations, return kTRUE if file found and put full path to file in 'fullpath"
   //
   //'name' is a filename (not an absolute pathname) i.e. "toto.dat"
   //'fullpath" will contain the full path to the file if it is found (if file not found, fullpath="")
   //'ndirs" is the number of directories to search in
   //the remaining arguments are the names of 'ndirs' paths to search in, i.e.
   //
   //      SearchFile("toto.dat", fullpath, 2, gSystem->pwd(), gSystem->HomeDirectory());
   //
   //means: search for a file 'toto.dat' in current working directory, then user's home directory.

   if (ndirs <= 0)
      return kFALSE;

   va_list args;
   va_start(args, ndirs);

   for (; ndirs; ndirs--) {     //loop over directories

      AssignAndDelete(fullpath,
                      gSystem->ConcatFileName(va_arg(args, const char*),
                                              name));
      if (!gSystem->AccessPathName(fullpath.Data())) {
         va_end(args);
         return kTRUE;
      }

   }

   va_end(args);
   fullpath = "";               //clear fullpath string to avoid using it by mistake
   return kFALSE;
}

Bool_t KVBase::SearchKVFile(const Char_t* name, TString& fullpath,
                            const Char_t* kvsubdir)
{
   //search for files in the following order:
   //  if 'name' = absolute path the function returns kTRUE if the file exists
   //  if name != absolute path:
   //      1. a. if 'kvsubdir'="" (default) look for file in $(pkgdatadir) directory
   //      1. b. if 'kvsubdir'!="" look for file in $(pkgdatadir)/'kvsubdir'
   //      2. look for file with this name in user's home directory
   //      3. look for file with this name in working directory
   //in all cases the function returns kTRUE if the file was found.
   //'fullpath' then contains the absolute path to the file

   if (gSystem->IsAbsoluteFileName(name)) {
      //absolute path
      fullpath = name;
      return !gSystem->AccessPathName(name);
   }

   TString kvfile_dir;
   if (strcmp(kvsubdir, "")) {
      //subdirectory name given
      kvfile_dir = GetDATADIRFilePath(kvsubdir);
   }
   else
      kvfile_dir = GetDATADIRFilePath();
   return SearchFile(name, fullpath, 3, kvfile_dir.Data(),
                     gSystem->HomeDirectory(), gSystem->pwd());
}

//________________________________________________________________________________//

Bool_t KVBase::SearchAndOpenKVFile(const Char_t* name, ifstream& file, const Char_t* kvsubdir, KVLockfile* locks)
{
   //Search and open for READING a file:
   //
   //search for ascii file (and open it, if found) in the following order:
   //  if 'name' = absolute path the function returns kTRUE if the file exists
   //  if name != absolute path:
   //      1. a. if 'kvsubdir'="" (default) look for file in $(pkdatadir) directory
   //      1. b. if 'kvsubdir'!="" look for file in $(pkdatadir)/'kvsubdir'
   //      2. look for file with this name in user's home directory
   //      3. look for file with this name in working directory
   //if the file is not found, kFALSE is returned.
   //if file is found and can be opened, file' is then an ifstream connected to the open (ascii) file
   //
   //LOCKFILE:
   //If a KVLockfile pointer is given, we use it to get a lock on the file before opening it.
   //If this lock is not successful, the file is not opened and we return an error message.

   TString fullpath;
   if (SearchKVFile(name, fullpath, kvsubdir)) {
      //put lock on file if required
      if (locks && !locks->Lock(fullpath.Data())) return kFALSE;
      file.open(fullpath.Data());
      if (file.good()) {
         //cout << "Opened file : " << fullpath.Data() << endl;
         return kTRUE;
      }
      //unlock file if not opened successfully
      if (locks) locks->Release();
   }
   return kFALSE;
}
//________________________________________________________________________________//

Bool_t KVBase::SearchAndOpenKVFile(const Char_t* name, ofstream& file, const Char_t* kvsubdir, KVLockfile* locks)
{
   //Search and CREATE i.e. open for WRITING a file:
   //
   //open for writing an ascii file in the location determined in the following way:
   //  if 'name' = absolute path we use the full path
   //  if name != absolute path:
   //      1. a. if 'kvsubdir'="" (default) file will be in $(pkdatadir) directory
   //      1. b. if 'kvsubdir'!="":
   //               if 'kvsubdir' is an absolute pathname, file in 'kvsubdir'
   //               if 'kvsubdir' is not an absolute pathname,
   //                    file will be in '$(pkdatadir)/kvsubdir'
   //if an existing file is found, a warning is printed and the existing file 'toto' is renamed
   //"toto.date". where 'date' is created with TDatime::AsSQLDate
   // file' is then an ofstream connected to the opened file
   //
   //LOCKFILE:
   //If a KVLockfile pointer is given, we use it to get a lock on the file before opening it.
   //If this lock is not successful, the file is not opened and we return an error message.

   KVString fullpath;
   if (gSystem->IsAbsoluteFileName(name)) {
      fullpath = name;
   }
   else if (gSystem->IsAbsoluteFileName(kvsubdir)) {
      AssignAndDelete(fullpath,
                      gSystem->ConcatFileName(kvsubdir, name));
   }
   else if (strcmp(kvsubdir, "")) {
      KVString path = GetDATADIRFilePath(kvsubdir);
      AssignAndDelete(fullpath,
                      gSystem->ConcatFileName(path.Data(), name));
   }
   else {
      fullpath = GetDATADIRFilePath(name);
   }
   //Backup file if necessary
   BackupFileWithDate(fullpath.Data());
   //put lock on file if required
   if (locks && !locks->Lock(fullpath.Data())) return kFALSE;
   file.open(fullpath.Data());
   return kTRUE;
}

//________________________________________________________________________________//

void KVBase::BackupFileWithDate(const Char_t* path)
{
   //`path` gives the full path (can include environment variables, special symbols)
   //to a file which will be renamed with an extension containing the current date and time
   //(in SQL format).
   //
   //Example:
   //
   //    KVBase::BackupFileWithDate("$(HOME)/toto.txt")
   //
   //The file `toto.txt` will be renamed `toto.txt.2007-05-02_16:22:37`

   KVString fullpath = path;
   gSystem->ExpandPathName(fullpath);
   if (!gSystem->AccessPathName(fullpath.Data())) {//does the file exist ?
      //backup file
      TDatime now;
      KVString date(now.AsSQLString());
      date.ReplaceAll(' ', '_');
      TString backup = fullpath + "." + date;
      //lock both files
      KVLockfile lf1(fullpath.Data()), lf2(backup.Data());
      if (lf1.Lock() && lf2.Lock()) {
         gSystem->Rename(fullpath.Data(), backup.Data());
         printf("Info in <KVBase::BackupFileWithDate(const Char_t *)> : Existing file %s renamed %s\n",
                fullpath.Data(), backup.Data());
      }
   }
}

//________________________________________________________________________________//

TPluginHandler* KVBase::LoadPlugin(const Char_t* base, const Char_t* uri)
{
   //Load plugin library in order to extend capabilities of base class "base", depending on
   //the given uri (these arguments are used to call TPluginManager::FindHandler).
   //Returns pointer to TPluginHandler.
   //Returns 0 in case of problems.

   //does plugin exist for given name ?
   TPluginHandler* ph =
      (TPluginHandler*) gROOT->GetPluginManager()->FindHandler(base,
            uri);
   if (!ph)
      return 0;

   //check plugin library/macro is available
   if (ph->CheckPlugin() != 0)
      return 0;

   //load plugin module
   if (ph->LoadPlugin() != 0)
      return 0;

   return ph;
}

//__________________________________________________________________________________________________________________

void KVBase::OpenTempFile(TString& base, ofstream& fp)
{
   //Opens a uniquely-named file in system temp directory (gSystem->TempDirectory)
   //Name of file is "basexxxxxxxxxx" where "xxxxxxxxx" is current time as returned
   //by gSystem->Now().
   //After opening file, 'base' contains full path to file.

   GetTempFileName(base);
   fp.open(base.Data());
}

//__________________________________________________________________________________________________________________

void KVBase::GetTempFileName(TString& base)
{
   //When called with base="toto.dat", the returned value of 'base' is
   //"/full/path/to/temp/dir/toto.dat15930693"
   //i.e. the full path to a file in the system temp directory (gSystem->TempDirectory)
   //appended with the current time as returned by gSystem->Now() in order to make
   //its name unique

   TString tmp1;
   AssignAndDelete(tmp1,
                   gSystem->ConcatFileName(gSystem->TempDirectory(),
                                           base.Data()));
   long lnow = (long) gSystem->Now();
   base = tmp1 + lnow;
   //make sure no existing file with same name
   while (!gSystem->AccessPathName(base)) {
      base = tmp1 + (++lnow);
   }
}

//__________________________________________________________________________________________________________________

const Char_t* KVBase::GetKVVersion()
{
   //Returns KaliVeda version string
   static TString tmp(KV_VERSION);
   return tmp.Data();
}

//__________________________________________________________________________________________________________________

const Char_t* KVBase::GetKVBuildUser()
{
   // Returns username of person who performed build
   static TString tmp(KV_BUILD_USER);
   return tmp.Data();
}

//__________________________________________________________________________________________________________________

const Char_t* KVBase::GetKVBuildDate()
{
   //Returns KaliVeda build date
   static TString tmp(KV_BUILD_DATE);
   return tmp.Data();
}

//__________________________________________________________________________________________________________________

const Char_t* KVBase::GetKVBuildType()
{
   //Returns KaliVeda build type (cmake build: Release, Debug, RelWithDebInfo, ...)
   static TString tmp(KV_BUILD_TYPE);
   return tmp.Data();
}

//__________________________________________________________________________________________________________________

const Char_t* KVBase::GetKVSourceDir()
{
   //Returns top-level directory of source tree used for build
   static TString tmp(KV_SOURCE_DIR);
   return tmp.Data();
}

//__________________________________________________________________________________________________________________

const Char_t* KVBase::GetKVBuildDir()
{
   //Returns top-level directory used for build
   static TString tmp(KV_BUILD_DIR);
   return tmp.Data();
}

//__________________________________________________________________________________________________________________
#ifdef WITH_GIT_INFOS
const Char_t* KVBase::gitBranch()
{
   // Returns git branch of sources
   static TString tmp(KV_GIT_BRANCH);
   return tmp.Data();
}

//__________________________________________________________________________________________________________________
const Char_t* KVBase::gitCommit()
{
   // Returns last git commit of sources
   static TString tmp(KV_GIT_COMMIT);
   return tmp.Data();
}
#endif
//__________________________________________________________________________________________________________________
#ifdef WITH_BZR_INFOS
const Char_t* KVBase::bzrRevisionId()
{
   // Returns Bazaar branch revision-id of sources
   static TString tmp(BZR_REVISION_ID);
   return tmp.Data();
}

//__________________________________________________________________________________________________________________

const Char_t* KVBase::bzrRevisionDate()
{
   // Returns date of Bazaar branch revision of sources
   static TString tmp(BZR_REVISION_DATE);
   return tmp.Data();
}

//__________________________________________________________________________________________________________________

const Char_t* KVBase::bzrBranchNick()
{
   // Returns nickname of Bazaar branch of sources
   static TString tmp(BZR_BRANCH_NICK);
   return tmp.Data();
}

//__________________________________________________________________________________________________________________

Int_t KVBase::bzrIsBranchClean()
{
   // Returns 1 if Bazaar branch of sources contained uncommitted
   // changes at time of building; 0 if all changes had been committed.
   // WARNING: this doesn't really work (ignore)
   return BZR_BRANCH_IS_CLEAN;
}

//__________________________________________________________________________________________________________________

Int_t KVBase::bzrRevisionNumber()
{
   // Returns Bazaar branch revision number of sources
   return BZR_REVISION_NUMBER;
}
#endif
//__________________________________________________________________________________________________________________

Bool_t KVBase::FindExecutable(TString& exec, const Char_t* path)
{
   //By default, `FindExecutable(exec)` will look for the executable named by `exec`
   //in the directories contained in the environment variable `PATH`. You can override
   //this by giving your own search `path` as second argument (remember to write
   //environment variables as <code>\$(PATH)</code>, for cross-platform compatibility).
   //
   //If `exec` is not found, and if it does not end with `.exe`, we look for `exec.exe`
   //This is for compatibility with Windows/cygwin environments.
   //
   //If the executable is found, returns `kTRUE` and `exec` then holds full path to executable.
   //Returns `kFALSE` if `exec` not found in paths.
   //
   //If `exec` is an absolute pathname, we return `kTRUE` if the file exists
   //(we do not use `path`).

   TString spath(path), backup(exec), backup2(exec), expandexec(exec);
   gSystem->ExpandPathName(expandexec);
   if (gSystem->IsAbsoluteFileName(expandexec.Data())) {
      //executable given as absolute path
      //we check if it exists
      if (!gSystem->AccessPathName(expandexec)) {
         exec = expandexec;
         return kTRUE;
      }
      else {
         //try with ".exe" in case of Windows system
         if (!expandexec.EndsWith(".exe")) {
            expandexec += ".exe";
            if (!gSystem->AccessPathName(expandexec)) {
               exec = expandexec;
               return kTRUE;
            }
         }
      }
      exec = backup;
      return kFALSE;
   }
   gSystem->ExpandPathName(spath);
   if (KVBase::FindFile(spath.Data(), exec))
      return kTRUE;
   if (!backup.EndsWith(".exe")) {
      backup += ".exe";
      if (KVBase::FindFile(spath.Data(), backup)) {
         exec = backup;
         return kTRUE;
      }
   }
   exec = backup2;
   return kFALSE;
}

//__________________________________________________________________________________________________________________

const Char_t* KVBase::FindFile(const Char_t* search, TString& wfil)
{
   //Backwards compatible fix for `TSystem::FindFile` which only exists from ROOT version 5.12/00 onwards.
   //Use this method as a replacement for `gSystem->FindFile` (same arguments)
#ifdef __WITHOUT_TSYSTEM_FINDFILE
   Char_t* result = gSystem->Which(search, wfil.Data());
   if (result) {
      wfil = result;
      delete[]result;
   }
   else {
      wfil = "";
   }
   return wfil.Data();
#else
   return gSystem->FindFile(search, wfil);
#endif
}

//__________________________________________________________________________________________________________________

Bool_t KVBase::FindClassSourceFiles(const Char_t* class_name, KVString& imp_file, KVString& dec_file, const Char_t* dir_name)
{
   //Look for the source files corresponding to `class_name`
   //i.e. taking class_name as a base, we look for one of
   //
   //~~~~~~~
   //class_name.C,class_name.cpp,class_name.cxx
   //~~~~~~~
   //
   //and one of
   //
   //~~~~~~~
   //class_name.h,class_name.hh,class_name.H
   //~~~~~~~
   //
   //By default we look in the current working directory, unless argument `dir_name`
   //is given
   //
   //If found, the names of the two files are written in `imp_file` and
   //`dec_file`

   KVNameValueList impl_alt;
   int i = 0;
   impl_alt.SetValue("%s.C", i);
   impl_alt.SetValue("%s.cpp", i);
   impl_alt.SetValue("%s.cxx", i);
   KVNameValueList decl_alt;
   decl_alt.SetValue("%s.h", i);
   decl_alt.SetValue("%s.hh", i);
   decl_alt.SetValue("%s.H", i);

   TString _dir_name = dir_name;
   gSystem->ExpandPathName(_dir_name);
   TSystemDirectory dir("LocDir", _dir_name.Data());
   TList* lf = dir.GetListOfFiles();
   Bool_t ok_imp, ok_dec;
   ok_imp = ok_dec = kFALSE;

   //look for implementation file
   for (i = 0; i < impl_alt.GetNpar(); i++) {
      if (lf->FindObject(Form(impl_alt.GetParameter(i)->GetName(), class_name))) {
         imp_file = Form(impl_alt.GetParameter(i)->GetName(), class_name);
         ok_imp = kTRUE;
      }
   }
   //look for header file
   for (i = 0; i < decl_alt.GetNpar(); i++) {
      if (lf->FindObject(Form(decl_alt.GetParameter(i)->GetName(), class_name))) {
         dec_file = Form(decl_alt.GetParameter(i)->GetName(), class_name);
         ok_dec = kTRUE;
      }
   }
   delete lf;
   return (ok_imp && ok_dec);
}

//__________________________________________________________________________________________________________________

const Char_t* KVBase::GetPluginURI(const Char_t* base, const Char_t* derived)
{
   //Inverse of `gPluginMgr->FindHandler(const Char_t* base, const Char_t* uri)`
   //
   //Given a base class `base` and a derived class `derived`, we search `gEnv` to find the
   //URI corresponding to this plugin.
   //
   //Example: given a plugin such as
   //
   //~~~~~~~~
   //Plugin.KVIDTelescope:      ^PHOS$           KVIDPhoswich         KVIndra         "KVIDPhoswich()"
   //~~~~~~~~
   //
   //then calling `KVBase::GetPluginURI("KVIDTelescope", "KVIDPhoswich")` will return `"PHOS"`.
   //
   //Most of the code is copied from `TPluginManager::LoadHandlersFromEnv`

   TIter next(gEnv->GetTable());
   TEnvRec* er;
   static TString tmp;

   while ((er = (TEnvRec*) next())) {
      const char* s;
      if ((s = strstr(er->GetName(), "Plugin."))) {
         // use s, i.e. skip possible OS and application prefix to Plugin.
         // so that GetValue() takes properly care of returning the value
         // for the specified OS and/or application
         const char* val = gEnv->GetValue(s, (const char*)0);
         if (val) {
            Int_t cnt = 0;
            s += 7;
            //is it the right base class ?
            if (strcmp(s, base)) continue;  //skip to next env var if not right base

            char* v = StrDup(val);
            while (1) {
               TString regexp = strtok(!cnt ? v : 0, "; ");
               if (regexp.IsNull()) break;
               TString clss   = strtok(0, "; ");
               if (clss.IsNull()) break;
               TString plugin = strtok(0, "; ");
               if (plugin.IsNull()) break;
               TString ctor = strtok(0, ";\"");
               if (!ctor.Contains("("))
                  ctor = strtok(0, ";\"");
               if (clss == derived) {
                  //found the required plugin
                  //we remove the 'regexp' operator '^' from the beginning
                  //and '$' from the end of the URI, if necessary
                  if (regexp.MaybeRegexp()) {
                     regexp.Remove(TString::kBoth, '^');
                     regexp.Remove(TString::kBoth, '$');
                  }
                  tmp = regexp;
                  delete [] v;
                  return tmp.Data();
               }
               cnt++;
            }
            delete [] v;
         }
      }
   }
   tmp = "";
   return tmp;
}

//__________________________________________________________________________________________________________________

const Char_t* KVBase::GetListOfPlugins(const Char_t* base)
{
   // Return whitespace-separated list of all plugin classes defined for
   // the given base class.
   //
   // E.g. if plugins exist for BaseClass:
   //
   //  Plugin.BaseClass:    URI     PluginClass     PluginLibrary    "PluginClassConstructor(arguments)"
   //  +Plugin.BaseClass:    URI2     PluginClass2     PluginLibrary2    "PluginClass2Constructor(arguments)"
   //
   // then KVBase::GetListOfPlugins("BaseClass") will return "PluginClass PluginClass2"
   //
   // Most of the code is copied from `TPluginManager::LoadHandlersFromEnv`

   TIter next(gEnv->GetTable());
   TEnvRec* er;
   static TString tmp;
   tmp = "";
   while ((er = (TEnvRec*) next())) {
      const char* s;
      if ((s = strstr(er->GetName(), "Plugin."))) {
         // use s, i.e. skip possible OS and application prefix to Plugin.
         // so that GetValue() takes properly care of returning the value
         // for the specified OS and/or application
         const char* val = gEnv->GetValue(s, (const char*)0);
         if (val) {
            Int_t cnt = 0;
            s += 7;
            //is it the right base class ?
            if (strcmp(s, base)) continue;  //skip to next env var if not right base

            char* v = StrDup(val);
            while (1) {
               TString regexp = strtok(!cnt ? v : 0, "; ");
               if (regexp.IsNull()) break;
               TString clss   = strtok(0, "; ");
               if (clss.IsNull()) break;
               TString plugin = strtok(0, "; ");
               if (plugin.IsNull()) break;
               TString ctor = strtok(0, ";\"");
               if (!ctor.Contains("("))
                  ctor = strtok(0, ";\"");
               tmp += clss;
               tmp += " ";
               cnt++;
            }
            delete [] v;
         }
      }
   }
   //remove final trailing whitespace
   tmp.Remove(TString::kTrailing, ' ');
   return tmp;
}

const Char_t* KVBase::GetListOfPluginURIs(const Char_t* base)
{
   // For a given base class, return a whitespace-separated list of plugin identifiers
   // which are known/defined.
   // E.g. if plugins exist for BaseClass:
   //
   //  Plugin.BaseClass:    URI     PluginClass     PluginLibrary    "PluginClassConstructor(arguments)"
   //  +Plugin.BaseClass:    URI2     PluginClass2     PluginLibrary2    "PluginClass2Constructor(arguments)"
   //
   // then KVBase::GetListOfPluginURIs("BaseClass") will return "URI URI2"

   static TString tmp;
   KVString plugs = KVBase::GetListOfPlugins(base);
   plugs.Begin(" ");
   tmp = "";
   while (!plugs.End()) {
      if (tmp != "") tmp += " ";
      tmp += KVBase::GetPluginURI(base, plugs.Next());
   }
   return tmp;
}

Bool_t KVBase::IsThisAPlugin(const TString& uri, TString& base)
{
   // Returns kTRUE if 'uri' is the name of a defined plugin, in which case 'base'
   // is the name of the base class extended by this plugin.
   //
   // Most of the code is copied from `TPluginManager::LoadHandlersFromEnv`

   TIter next(gEnv->GetTable());
   TEnvRec* er;
   while ((er = (TEnvRec*) next())) {
      KVString ername = er->GetName();
      if (ername.BeginsWith("Plugin.")) {
         base = ername;
         base.Remove(0, 7);
         KVString erval = gEnv->GetValue(ername, "");
         erval.Begin(" ");
         while (!erval.End()) {
            if (erval.Next() == uri) return kTRUE;
         }
      }
   }
   return kFALSE;
}

//__________________________________________________________________________________________________________________

void KVBase::ReadGUIMimeTypes()
{
   // Add to standard ROOT mime types some new ones defined in `.kvrootrc`
   // for icons associated with graphs, runs, etc. by lines such as:
   //
   //~~~~~~~~~
   //  KaliVeda.GUI.MimeTypes :   KVIDMap
   //  KaliVeda.GUI.MimeTypes.KVIDMap.Icon :   rootdb_t.xpm
   //  +KaliVeda.GUI.MimeTypes :   KVIDZAGrid
   //  KaliVeda.GUI.MimeTypes.KVIDZAGrid.Icon :   draw_t.xpm
   //~~~~~~~~~
   //
   // etc.

   KVString mimetypes = gEnv->GetValue("KaliVeda.GUI.MimeTypes", "");
   if (mimetypes != "") {

      mimetypes.Begin(" ");
      while (!mimetypes.End()) {

         KVString classname = mimetypes.Next(kTRUE);
         KVString icon = gEnv->GetValue(Form("KaliVeda.GUI.MimeTypes.%s.Icon", classname.Data()), "draw_t.xpm");
         KVString type = classname;
         type.ToLower();

         if (gClient) gClient->GetMimeTypeList()->AddType(Form("[kaliveda/%s]", type.Data()),
                  classname.Data(), icon.Data(), icon.Data(), "");

      }
   }
}

//__________________________________________________________________________________________________________________
#ifdef WITH_GRULIB
Int_t KVBase::TestPorts(Int_t port)
{
   // Test ports for availability. Start from 'port' and go up to port+2000 at most.
   // Returns -1 if no ports available.

   GNetClientRoot testnet((char*) "localhost");
   Int_t ret;
   ret = port;

   for (int i = 0; i < 2000; i++) {
      ret = testnet.TestPortFree(port, (char*) "localhost");
      if (ret > 0)
         break;
      if ((ret <= 0))
         port++;
   }

   return ret;

}
#endif

Bool_t KVBase::AreEqual(Double_t A, Double_t B, Long64_t maxdif)
{
   // \brief Comparison between two 64-bit floating-point values
   //
   // Returns `kTRUE` if the integer representations of the two values are within
   // `maxdif` of each other. By default `maxdif=1`, which means that we consider that `x==y` if the
   // difference between them is no greater than the precision of `Double_t`
   // variables, i.e. `4.94065645841246544e-324`
   //
   // Based on the function `AlmostEqual2sComplement(float, float, int)`
   // by Bruce Dawson http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm

   union converter {
      Double_t f;
      Long64_t i;
   } zero, val1, val2;

   assert(maxdif > 0);

   if (A == B) return true;

   /* rustine to obtain the (64-bit) constant value 0x8000000000000000
      even on 32-bit machines (there is probably an easier way!)      */
   zero.i = 1;
   zero.f = -zero.f;
   zero.i -= 1;

   val1.f = A;
   val2.f = B;
   Long64_t Aint, Bint;
   Aint = val1.i;
   Bint = val2.i;
   if (Aint < 0) Aint = zero.i - Aint;
   if (Bint < 0) Bint = zero.i - Bint;

   Long64_t intDiff = abs(val1.i - val2.i);

   if (intDiff <= maxdif) return true;

   return false;
}

Bool_t KVBase::OpenContextMenu(const char* method, TObject* obj, const char* alt_method_name)
{
   // Open context menu for given method of object *obj.
   // By default title of menu is 'obj->ClassName()::method'
   // You can give an alternative method name in 'alt_method_name'
   // Returns kFALSE if the given method is not defined for the class of object in question.
   //
   // WARNING: even if this method returns kTRUE, this is no guarantee that the method
   // has indeed been executed. The user may have pressed the 'Cancel' button...

   TMethod* m = obj->IsA()->GetMethodAllAny(method);
   if (!m) {
      obj->Warning("OpenContextMenu", "%s is not a method of %s", method, obj->ClassName());
      return kFALSE;
   }
   TString Method = alt_method_name;
   if (Method == "") Method = method;
   TContextMenu* cm = new TContextMenu(Method, Form("%s::%s", obj->ClassName(), Method.Data()));
   cm->Action(obj, m);
   delete cm;
   return kTRUE;
}

void KVBase::CombineFiles(const Char_t* file1, const Char_t* file2, const Char_t* newfilename, Bool_t keep)
{
   // STATIC method which allows to combine the contents of two ROOT files
   // (file1 and file2) into a new ROOT file (newfilename).
   // All objects from the two files will be written in the new file.
   //
   // if keep=kFALSE, the two files will be deleted after the operation

   ::Info("KVBase::CombineFiles", "Copying all objects from %s and %s ===> into new file %s", file1, file2, newfilename);
   TFile* f1 = TFile::Open(file1);
   TList objL1;//list of objects in file 1
   TList treeL1;//list of trees in file 1
   TIter next(f1->GetListOfKeys());
   TKey* key;
   while ((key = (TKey*)next())) {
      if (!TClass::GetClass(key->GetClassName(), kFALSE, kTRUE)->InheritsFrom("TDirectory")) {//avoid subdirectories!
         if (!TClass::GetClass(key->GetClassName(), kFALSE, kTRUE)->InheritsFrom("TTree"))
            objL1.Add(f1->Get(key->GetName()));
         else
            treeL1.Add(f1->Get(key->GetName()));
      }
   }
   TFile* f2 = TFile::Open(file2);
   TList objL2;//list of objects in file 2
   TList treeL2;//list of trees in file 2
   TIter next2(f2->GetListOfKeys());
   while ((key = (TKey*)next2())) {
      if (!TClass::GetClass(key->GetClassName(), kFALSE, kTRUE)->InheritsFrom("TDirectory")) {//avoid subdirectories!
         if (!TClass::GetClass(key->GetClassName(), kFALSE, kTRUE)->InheritsFrom("TTree"))
            objL2.Add(f2->Get(key->GetName()));
         else
            treeL2.Add(f2->Get(key->GetName()));
      }
   }
   TFile* newfile = new TFile(newfilename, "recreate");
   objL1.Execute("Write", "");
   objL2.Execute("Write", "");
   if (treeL1.GetEntries()) {
      TIter nxtT(&treeL1);
      TTree* t;
      while ((t = (TTree*)nxtT())) t->CloneTree(-1, "fast")->Write();
   }
   if (treeL2.GetEntries()) {
      TIter nxtT(&treeL2);
      TTree* t;
      while ((t = (TTree*)nxtT())) t->CloneTree(-1, "fast")->Write();
   }
   newfile->Close();
   f1->Close();
   f2->Close();
   if (!keep) {
      gSystem->Unlink(file1);
      gSystem->Unlink(file2);
   }
}

TObject* KVBase::GetObject() const
{
   // Dummy method (returns NULL).
   // This method may be used in 'container' classes used with KVListView.
   // In order to open the context menu of the 'contained' object,
   // GetLabel() should return the real class of the object,
   // and this method should return its address.
   // Then call KVListView::SetUseObjLabelAsRealClass(kTRUE).
   return NULL;
}


const Char_t* KVBase::GetExampleFilePath(const Char_t* library, const Char_t* namefile)
{
   // Return full path to example file for given library (="KVMultiDet", "BackTrack", etc.)
   static TString path;
   path = KVBase::GetDATADIRFilePath("examples/");
   path += library;
   path += "/";
   path += namefile;
   return path.Data();
}

void KVBase::PrintSplashScreen()
{
   // Prints welcome message and infos on version etc.

   cout << "***********************************************************" <<
        endl;
   cout << "*                    HI COQUINE !!!                       *" <<
        endl;
   cout << "*                                                         *" <<
        endl;
   cout << "*         W E L C O M E     to     K A L I V E D A        *" <<
        endl;
   cout << "*                                                         *" <<
        endl;
   printf("* Version: %-10s                   Built: %-10s *\n", KVBase::GetKVVersion(), KVBase::GetKVBuildDate());
#ifdef WITH_BZR_INFOS
   TString bzrinfo;
   bzrinfo.Form("%s@%d", bzrBranchNick(), bzrRevisionNumber());
   printf("* bzr: %50s *\n", bzrinfo.Data());
#endif
#ifdef WITH_GIT_INFOS
   TString gitinfo;
   gitinfo.Form("%s@%s", gitBranch(), gitCommit());
   printf("* git: %-50s *\n", gitinfo.Data());
#endif
   cout << "*                                                         *" <<
        endl;
   cout << "*               For help, see the website :               *" <<
        endl;
   cout << "*             http://indra.in2p3.fr/kaliveda              *" <<
        endl;
   cout << "*                                                         *" <<
        endl;
   cout << "*                          ENJOY !!!                      *" <<
        endl;
   cout << "***********************************************************" <<
        endl << endl;
}

const Char_t* KVBase::GetDataSetEnv(const Char_t* dataset, const Char_t* type, const Char_t* defval)
{
   // Static method to interrogate dataset-specific variables in configuration
   // Will look for gEnv->GetValue "dataset.type"
   // then simply "type" if no dataset-specific value is found.
   // If neither resource is defined, return the "defval" default value (="" by default)
   TString temp;
   temp.Form("%s.%s", dataset, type);
   if (gEnv->Defined(temp.Data())) return gEnv->GetValue(temp.Data(), "");
   return gEnv->GetValue(type, defval);
}

Double_t KVBase::GetDataSetEnv(const Char_t* dataset, const Char_t* type, Double_t defval)
{
   // Static method to interrogate dataset-specific variables in configuration
   // Will look for gEnv->GetValue "dataset.type"
   // then simply "type" if no dataset-specific value is found.
   // If neither resource is defined, return the "defval" default value

   TString temp;
   temp.Form("%s.%s", dataset, type);
   if (gEnv->Defined(temp.Data())) return gEnv->GetValue(temp.Data(), 0.0);
   return gEnv->GetValue(type, defval);
}

Bool_t KVBase::GetDataSetEnv(const Char_t* dataset, const Char_t* type, Bool_t defval)
{
   // Static method to interrogate dataset-specific variables in configuration
   //Will look for gEnv->GetValue "dataset.type"
   //then simply "type" if no dataset-specific value is found.
   //If neither resource is defined, return the "defval" default value

   TString temp;
   temp.Form("%s.%s", dataset, type);
   if (gEnv->Defined(temp.Data())) return gEnv->GetValue(temp.Data(), kFALSE);
   return gEnv->GetValue(type, defval);
}


/** \example base_kvbase.C
# Example of use of the KVBase class

This is the base class of most classes in the KaliVeda framework.
It provides information on the version & installation layout of
KaliVeda, plus various general "utility" methods.

To execute this function, do:

    $ kaliveda
    kaliveda[0] .L base_kvbase.C+
    kaliveda[1] kvbase_example()

*/
