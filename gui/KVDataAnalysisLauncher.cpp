//
// Implementation de KVDataAnalysisLauncher
//

#include "KVDataAnalysisLauncher.h"
#include "TGLabel.h"
#include "TGMsgBox.h"
#include "TColor.h"
#include "TGButtonGroup.h"
#include "TGFileDialog.h"
#include "TObjString.h"
#include "TDatime.h"
#include "TApplication.h"
#include "TSocket.h"
#include "TGToolTip.h"
#include "TGFileDialog.h"
#include "Riostream.h"
#include "TClass.h"
#include "KVDataRepositoryManager.h"
#include "KVDataRepository.h"
#include "KVDataSetManager.h"
#include "KVDataSet.h"
#include "KVDataAnalysisTask.h"
#include "KVINDRADBRun.h"
#include "KVDBSystem.h"
#include "KV2Body.h"
#include "KVNucleus.h"
#include "KVINDRADB.h"
#include "KVINDRAReconDataAnalyser.h"
#include "KV_CCIN2P3_BQS.h"
#include "KVBatchSystemManager.h"
#include "TSystemDirectory.h"
#include "KVInputDialog.h"

#define TTDELAY 750

//maximum length of runlist shown next to "Selected Runs :"
#define MAX_LENGTH_SELECTED_RUNS 40

//#define KVDAL_DEBUG 

ClassImp(KVGFileList)

//__________________________________________
void KVGFileList::Init(TString &fileList,const Char_t *title)
{
// Init window
 TGLayoutHints *eXeY=new TGLayoutHints(kLHintsLeft|kLHintsTop|
                                       kLHintsExpandX|kLHintsExpandY,
				       1,1,1,1);
 TGLayoutHints *eX=new TGLayoutHints(kLHintsLeft|kLHintsTop|
                                       kLHintsExpandX,
				       1,1,1,1);

 // File list TGListBox
 lbFileList=new TGListBox(this,LB_Files);
 lbFileList->SetMultipleSelections(kTRUE);
 lbFileList->Resize(350,96);
 this->AddFrame(lbFileList,eXeY);
 
 // File Name TGTextEntry
 teFileName=new TGTextEntry(this,"");
 this->AddFrame(teFileName,eX);
 
 // buttons
 TGCompositeFrame *cf=new TGCompositeFrame(this,1200,350,kHorizontalFrame);
 boutAdd=new TGTextButton(cf,"Add File");
 boutAdd->SetToolTipText("Add a file to the list",TTDELAY);
 boutAdd->Connect("Clicked()",
                      "KVGFileList",
		       this,
		       "AddFile()");
 cf->AddFrame(boutAdd,eX);
 
 boutRem=new TGTextButton(cf,"Remove File");
 boutRem->SetToolTipText("Remove the selected file from the list",TTDELAY);
 boutRem->Connect("Clicked()",
                      "KVGFileList",
		       this,
		       "RemoveFiles()");
 cf->AddFrame(boutRem,eX);
 		     
 boutAllRem=new TGTextButton(cf,"Remove All File");
 boutAllRem->SetToolTipText("Remove all files from the list",TTDELAY);
 boutAllRem->Connect("Clicked()",
                      "KVGFileList",
		       this,
		       "RemoveAllFiles()");
 cf->AddFrame(boutAllRem,eX);
 		     
 this->AddFrame(cf,eX);

 TGTextButton *bout=new TGTextButton(this,"Done");
 bout->SetToolTipText("Close window",TTDELAY);
 bout->Connect("Clicked()",
                      "KVGFileList",
		       this,
		       "Done()");
 this->AddFrame(bout,eX);

 // Map all widgets
 MapSubwindows();
 Resize(GetDefaultSize());
 SetWindowName(title);
 SetIconName(title);

 fileListString=&fileList;
 fileDialogDir=gSystem->Getenv("PWD");

 entryMax=0;
}

//__________________________________________
KVGFileList::KVGFileList(TString &fileList,const Char_t *title,
                         const TGWindow *p, const TGWindow *main,
			 Bool_t ok):
            TGTransientFrame(p,main,10,10)
{
// Createur

 if(ok) 
  {
  Init(fileList,title);
  InitFileList();
  MapRaised();
  fClient->WaitFor(this);
  }
}

//__________________________________________
KVGFileList::~KVGFileList()
{
// Destructeur
delete lbFileList;
delete teFileName;
delete boutAdd;
delete boutRem;
delete boutAllRem;
}

//__________________________________________
void KVGFileList::InitFileList()
{
// Init the file name list box from a TString. Add all fields separated by a
// white space.
TObjArray *oa=fileListString->Tokenize(" ");
TIter next(oa);
TObjString *ost=0;
while((ost=(TObjString *)next()))
 {
 TString fileName=ost->GetString();
 if(fileName.Length() && !lbFileList->FindEntry(fileName.Data()))
  {
  lbFileList->AddEntry(new TGString(fileName.Data()),entryMax);
  entryMax++;
  }
 }
oa->SetOwner(kTRUE);
delete oa;
lbFileList->MapSubwindows();
lbFileList->Layout();
}

//__________________________________________
void KVGFileList::AddFile(void)
{
// Add a file to the listbox. If the TGTextEntry is ampty, a file open dialog 
// appears
TString fileName=teFileName->GetText();
if(!fileName.Length())
 {
 if(Char_t *fn=this->GetFileFromDialog())
  {
  fileName=fn;
  }
 else
  {
  fileName="";
  }
 }
if(fileName.Length() && 
   !lbFileList->FindEntry(fileName.Data()) && 
   this->CanAdd(fileName.Data()))
 {
  lbFileList->AddEntry(new TGString(fileName.Data()),entryMax);
  entryMax++;
 }
lbFileList->MapSubwindows();
lbFileList->Layout();
}

//__________________________________________
void KVGFileList::RemoveFiles(void)
{
// Remove all the selected files from the TGListBox
TList *sel=new TList();
lbFileList->GetSelectedEntries(sel);
TIter next(sel);
TGLBEntry *e=0;
while((e=(TGLBEntry *)next()))
 {
 gSystem->Unload(((TGTextLBEntry *)e)->GetText()->GetString());
 lbFileList->RemoveEntry(e->EntryId());
 }
lbFileList->MapSubwindows();
lbFileList->Layout();
}

//__________________________________________
void KVGFileList::RemoveAllFiles(void)
{
// Remove all the files from the TGListBox
lbFileList->RemoveEntries(0,entryMax);
entryMax=0;
lbFileList->MapSubwindows();
lbFileList->Layout();
}

//__________________________________________
void KVGFileList::Done(void)
{
// build the file list string from the content of the TGListBox
(*fileListString)="";
for(Int_t i=0;i<entryMax;i++)
 {
 if(TGTextLBEntry *e=(TGTextLBEntry *)lbFileList->GetEntry(i))
  {
  if(fileListString->Length()) (*fileListString)+=" ";
  (*fileListString)+=e->GetText()->GetString();
  }
 }
this->CloseWindow();
}

//__________________________________________
Char_t *KVGFileList::GetFileFromDialog(void)
{
// Gets the file name from a TGFileDialog
 TGFileInfo *fil=new TGFileInfo();
 const char *filTypes[] = {"Shared Object Files","*.so",
                            0,                  0 };
 fil->fFileTypes=filTypes;
 fil->fIniDir = StrDup(fileDialogDir.Data());
 new TGFileDialog(gClient->GetRoot(),this,kFDOpen,fil);
 fileDialogDir=fil->fIniDir;
 return fil->fFilename;
}

//__________________________________________
Bool_t KVGFileList::CanAdd(const Char_t *fn)
{
// tells whether the file in ths string fn can be added to the list box
 Bool_t ok=kTRUE;

 FileStat_t fs; TString tmp = fn; gSystem->ExpandPathName(tmp);
 if(gSystem->GetPathInfo(tmp.Data(),fs))
  {
  ok=kFALSE;
  new TGMsgBox(gClient->GetRoot(),this,
                "File does not exist",
		Form("The file \"%s\" does not exist. Nothing added.",
		     fn),
		kMBIconExclamation);
  }
 else
  {
  TString fname=fn;
  if(!fname.EndsWith(".so"))
   {
   ok=kFALSE;
   new TGMsgBox(gClient->GetRoot(),this,
                "Not a share object",
		Form("The file \"%s\" is not a shared object. Nothing added.",
		     fn),
		kMBIconExclamation);
   
   }
  }
 return ok;
}


ClassImp(KVGDirectoryList)

//__________________________________________
void KVGDirectoryList::Init(TString &fileList,const Char_t *title)
{
// init window
KVGFileList::Init(fileList,title);

boutAdd->SetText("Add Directory");
boutAdd->SetToolTipText("Add a directory to the list.",TTDELAY);
boutAdd->Layout();

boutRem->SetText("Remove Directory");
boutRem->SetToolTipText("Remove the selected directories from the list.",TTDELAY);
boutRem->Layout();

boutAllRem->SetText("Remove All Directories");
boutAllRem->SetToolTipText("Remove all directories from the list.",TTDELAY);
boutAllRem->Layout();
}

//__________________________________________
KVGDirectoryList::KVGDirectoryList(TString &fileList,const Char_t *title,
                         const TGWindow *p, const TGWindow *main,
			 Bool_t ok):
            KVGFileList(fileList,title,p,main,kFALSE)
{
// Createur

 if(ok)
  {
  Init(fileList,title);
  InitFileList();
  MapRaised();
  fClient->WaitFor(this);
  }
}

//__________________________________________
KVGDirectoryList::~KVGDirectoryList()
{
// Destructeur
}


//__________________________________________
Char_t *KVGDirectoryList::GetFileFromDialog(void)
{
// Gets the file name from a TGFileDialog
 TGFileInfo *fil=new TGFileInfo();
 const char *filTypes[] = {"Include Files","*.[h,H]*",
                            0,                  0 };
 fil->fFileTypes=filTypes;
 fil->fIniDir= StrDup(fileDialogDir.Data());
 new TGFileDialog(gClient->GetRoot(),this,kFDOpen,fil);
 fileDialogDir=fil->fIniDir;
 if(fil->fFilename)
  return fil->fIniDir;
 else
  return 0;
}

//__________________________________________
Bool_t KVGDirectoryList::CanAdd(const Char_t *fn)
{
// tells whether the file in ths string fn can be added to the list box
 Bool_t ok=kTRUE;

 FileStat_t fs; TString tmp = fn; gSystem->ExpandPathName(tmp);
 if(gSystem->GetPathInfo(tmp.Data(),fs))
  {
  ok=kFALSE;
  new TGMsgBox(gClient->GetRoot(),this,
                "Directory does not exist",
		Form("The directory \"%s\" does not exist. Nothing added.",
		     fn),
		kMBIconExclamation);
  }
 else
  {
  if(!(fs.fMode & kS_IFDIR))
   {
   ok=kFALSE;
   new TGMsgBox(gClient->GetRoot(),this,
                "Not a directory",
		Form("The path \"%s\" is not a directory. Nothing added.",
		     fn),
		kMBIconExclamation);
   
   }
  }
 return ok;
}

ClassImp(KVDataAnalysisLauncher)

//__________________________________________
KVDataAnalysisLauncher::KVDataAnalysisLauncher(const TGWindow *p,UInt_t w,UInt_t h):TGMainFrame(p,w,h)
{
   // Createur
 
	entryMax=-1;
   //Initialisation of resource file
   GUIenv=new TEnv(".KVDataAnalysisGUIrc");
   //initialisation of list used by Get/SetResource
   ResourceNames = new TList;
   ResourceNames->Add( new TNamed("Repository","") );
   ResourceNames->Add( new TNamed("DataSet","") );
   ResourceNames->Add( new TNamed("Task","") );
   ResourceNames->Add( new TNamed("System","") );
   ResourceNames->Add( new TNamed("Trigger","") );
   NbResNames = ResourceNames->GetEntries();
   
   // list of user analysis classes present in working directory
   UserClassNames = new TList;
   UserClassNames->SetOwner(kTRUE);
   
   //initialise repositories, datasets, etc.
   if(!gDataRepositoryManager){
      new KVDataRepositoryManager();
      gDataRepositoryManager->Init();
   }
   if(gDataRepositoryManager->GetListOfRepositories()->GetEntries()==0){
      exit(1);
   }
   
 ia = new KVDataAnalyser;//this will set up gBatchSystemManager 

// Creation de l'environnement d'affichage et ajout des 2 boutons a cet
// environnement 

 TGLayoutHints *eXeY=new TGLayoutHints(kLHintsLeft|kLHintsTop|
                                       kLHintsExpandX|kLHintsExpandY,
				       1,1,1,1);
 TGLayoutHints *eXcY=new TGLayoutHints(kLHintsLeft|kLHintsTop|
                                       kLHintsExpandX|kLHintsCenterY,
				       1,1,1,1);
 TGLayoutHints *eX=new TGLayoutHints(kLHintsLeft|kLHintsTop|
                                       kLHintsExpandX,
				       1,1,1,1);

// Creation des 3 frames dans la fenetre
 TGCompositeFrame *cfSelect=new TGCompositeFrame(this,1600,350,kVerticalFrame);
 
 Int_t justMode=kTextBottom|kTextRight;
 TGCompositeFrame *cf=new TGCompositeFrame(cfSelect,1600,350,kHorizontalFrame);
// Label du Repository
 TGLabel *lab=new TGLabel(cf,"Repository");
 lab->Resize(150,20);
 lab->SetTextJustify(justMode);
 cf->AddFrame(lab); 
// ComboBox du Repository
 cbRepository=new TGComboBox(cf,CB_DataRepository);
 cbRepository->Select(-1);
 cbRepository->Resize(350,20);
 cbRepository->Connect("Selected(char*)",
                       "KVDataAnalysisLauncher",
		       this,
		       "SetDataSetList(char*)");
 cf->AddFrame(cbRepository,eX);
 cfSelect->AddFrame(cf,eX);
 
 cf=new TGCompositeFrame(cfSelect,1600,350,kHorizontalFrame);
// Label du Data Set
 lab=new TGLabel(cf,"Data Set");
 lab->Resize(150,20);
 lab->SetTextJustify(justMode);
 cf->AddFrame(lab,new TGLayoutHints(kLHintsLeft|kLHintsTop,
                                    10,1,1,1)); 
// ComboBox du Data Set
 cbDataSet=new TGComboBox(cf,CB_DataSet);
 cbDataSet->Select(-1);
 cbDataSet->Resize(350,20);
 cbDataSet->Connect("Selected(char*)",
                       "KVDataAnalysisLauncher",
		       this,
		       "SetTaskList(char*)");
 cf->AddFrame(cbDataSet,eX);
 cfSelect->AddFrame(cf,eX);

 cf=new TGCompositeFrame(cfSelect,1600,350,kHorizontalFrame);
// Label du Task
 lab=new TGLabel(cf,"Task");
 lab->SetTextJustify(justMode);
 lab->Resize(150,20);
 cf->AddFrame(lab,new TGLayoutHints(kLHintsLeft|kLHintsTop,
                                    32,1,1,1)); 
// ComboBox du Task
 cbTask=new TGComboBox(cf,CB_AnalysisTask);
 cbTask->Select(-1);
 cbTask->Resize(350,20);
 cbTask->Connect("Selected(int)",
                       "KVDataAnalysisLauncher",
		       this,
		       "SetSystemList(int)");
 cf->AddFrame(cbTask,eX);
 cfSelect->AddFrame(cf,eX);

 cf=new TGCompositeFrame(cfSelect,1600,350,kHorizontalFrame);
// Label du systeme
 lab=new TGLabel(cf,"System");
 lab->SetTextJustify(justMode);
 lab->Resize(150,20);
 cf->AddFrame(lab,new TGLayoutHints(kLHintsLeft|kLHintsTop,
                                    18,1,1,1)); 
// ComboBox du systeme
 cbSystem=new TGComboBox(cf,CB_System);
 cbSystem->Select(-1);
 cbSystem->Resize(350,20);
 cbSystem->Connect("Selected(int)",
                       "KVDataAnalysisLauncher",
		       this,
		       "SetTriggersList(int)");
//  cbSystem->Connect("Selected(int)",
//                        "KVDataAnalysisLauncher",
// 		       this,
// 		       "SetRunsList(int)");
 cf->AddFrame(cbSystem,eX);
 cfSelect->AddFrame(cf,eX);

 cf=new TGCompositeFrame(cfSelect,1600,350,kHorizontalFrame);
// Label du Trigger
 lab=new TGLabel(cf,"Trigger");
 lab->SetTextJustify(justMode);
 lab->Resize(150,20);
 cf->AddFrame(lab,new TGLayoutHints(kLHintsLeft|kLHintsTop,
                                    18,1,1,1)); 
// ComboBox du systeme
 cbTrigger=new TGComboBox(cf,CB_Trigger);
 cbTrigger->Select(-1);
 cbTrigger->Resize(350,20);
 cbTrigger->Connect("Selected(int)",
                       "KVDataAnalysisLauncher",
		       this,
		       "SetTriggerRunsList(int)");
 cf->AddFrame(cbTrigger,eX);
 cfSelect->AddFrame(cf,eX);


 this->AddFrame(cfSelect,new TGLayoutHints(kLHintsLeft|kLHintsTop
                                         |kLHintsRight|kLHintsExpandX,
                                    10,10,1,1));
 
// Frame pour la liste des runs
 TGCompositeFrame *cfRuns=new TGCompositeFrame(this,1600,350,kVerticalFrame);
	lvRuns = new KVListView(KVDBRun::Class(), cfRuns, 500, 250);
	lvRuns->SetDataColumns(4);
	lvRuns->SetDataColumn(0, "Run", "GetNumber");
	lvRuns->SetDataColumn(1, "Events", "", kTextRight);
	lvRuns->SetDataColumn(2, "Date", "GetDatime");
	lvRuns->GetDataColumn(2)->SetIsDateTime();
	lvRuns->SetDataColumn(3, "Comments", "", kTextLeft);
	lvRuns->ActivateSortButtons();
	lvRuns->Connect("SelectionChanged()", "KVDataAnalysisLauncher", this, "UpdateListOfSelectedRuns()");
	cfRuns->AddFrame(lvRuns, eXeY);

// Boutons de selection
 TGCompositeFrame *cfSelAll=new TGCompositeFrame(cfRuns,1600,20,kHorizontalFrame);
 TGTextButton *bout=new TGTextButton(cfSelAll,"Select All");
 bout->SetToolTipText("Select all runs for the analysis.",TTDELAY);
 bout->Connect("Clicked()",
                      "KVDataAnalysisLauncher",
		       this,
		       "SelectAll()");
 cfSelAll->AddFrame(bout,eX);
 bout=new TGTextButton(cfSelAll,"Deselect All");
 bout->SetToolTipText("Deselect all runs.",TTDELAY);
 bout->Connect("Clicked()",
                      "KVDataAnalysisLauncher",
		       this,
		       "DeselectAll()");
 cfSelAll->AddFrame(bout,eX);
 bout=new TGTextButton(cfSelAll,"Runlist");
 bout->SetToolTipText("Enter list of runs to analyse.",TTDELAY);
 bout->Connect("Clicked()",
                      "KVDataAnalysisLauncher",
		       this,
		       "EnterRunlist()");
 cfSelAll->AddFrame(bout,eX);
 
 cfRuns->AddFrame(cfSelAll,eX); 

// label des runs selectionnes
 selectedRuns=new TGLabel(cfRuns,"Selected Runs :");
 cfRuns->AddFrame(selectedRuns,eX); 
  
 this->AddFrame(cfRuns,eXeY);
  
// UserClass and DataSelector et nombre d'evenements
 cfAnalysis=new TGCompositeFrame(this,1600,20,kVerticalFrame);
 cf=new TGCompositeFrame(cfAnalysis,1600,20,kHorizontalFrame);
// Label for User Class name
 fUserClassLabel=new TGLabel(cf,"User Class");
 cf->AddFrame(fUserClassLabel,eX); 
// Label du data selector (only for KVINDRAReconDataAnalyser)
 fDataSelectorLabel=new TGLabel(cf,"-");
 cf->AddFrame(fDataSelectorLabel,eX); 
// Label du Task
 lab=new TGLabel(cf,"Number of events to read");
 cf->AddFrame(lab,eX); 
 
 cfAnalysis->AddFrame(cf,eX);
 
 cf=new TGCompositeFrame(cfAnalysis,1600,20,kHorizontalFrame);
 //teSelector=new TGTextEntry(cf,"");
 //teSelector->Resize(233,20);
 //teSelector->SetToolTipText("Enter the user analysis class name",TTDELAY);
 //cf->AddFrame(teSelector,eX);
 cbUserClass = new TGComboBox(cf);
 cbUserClass->Select(-1);
 cbUserClass->Resize(150,20);
 cf->AddFrame(cbUserClass,eX);
 cbUserClass->Connect("Selected(char*)", "KVDataAnalysisLauncher", this,
       "UserClassSelected(char*)");
 
 teDataSelector=new TGTextEntry(cf,"");
 //teDataSelector->Resize(233,20);
 teDataSelector->SetToolTipText("Enter the KVDataSelector class name.",TTDELAY);
 cf->AddFrame(teDataSelector,eX);

 teNbToRead=new TGNumberEntry(cf,0);
#ifdef __WITHOUT_TGNUMBERENTRY_SETNUMSTYLE
 teNbToRead->SetFormat(TGNumberFormat::kNESInteger, teNbToRead->GetNumAttr());
#else
 teNbToRead->SetNumStyle(TGNumberFormat::kNESInteger);
#endif
#ifdef __WITHOUT_TGNUMBERENTRY_SETNUMATTR
 teNbToRead->SetFormat(teNbToRead->GetNumStyle(), TGNumberFormat::kNEANonNegative);
#else
 teNbToRead->SetNumAttr(TGNumberFormat::kNEANonNegative);
#endif
 teNbToRead->GetNumberEntry()->SetToolTipText("Enter the number of events to read.",TTDELAY);
 //teNbToRead->Resize(233,20);
 cf->AddFrame(teNbToRead,eX);

 cfAnalysis->AddFrame(cf,eX);

 this->AddFrame(cfAnalysis,eX);

 cf=new TGCompositeFrame(this,1600,20,kHorizontalFrame);
 // Frame for the user's libraries
 bout=new TGTextButton(cf,"User's libraries",B_Libs);
 bout->Connect("Clicked()",
               "KVDataAnalysisLauncher",
	       this,
	       "SetUserLibraries()");
 cf->AddFrame(bout,eX);

 bout=new TGTextButton(cf,"User's includes",B_Incs);
 bout->Connect("Clicked()",
               "KVDataAnalysisLauncher",
		this,
		"SetUserIncludes()");
 cf->AddFrame(bout,eX);

 this->AddFrame(cf,eX);
 
 cf=new TGCompositeFrame(this,1600,20,kHorizontalFrame);
 // Radio buttons for the running mode
 TString tit="Running Mode";
 TGVButtonGroup *gb=new TGVButtonGroup(cf,tit);
 rbInteractive=new TGRadioButton(gb,"Interactive");
 rbInteractive->SetToolTipText("Run the analysis in the interactive mode.",TTDELAY);
 rbSequential=new TGRadioButton(gb,"Sequential Batch");
 rbSequential->
  SetToolTipText("Run the analysis in batch mode (1 batch for all runs).",TTDELAY);
 rbParallel=new TGRadioButton(gb,"Parallel Batch");
 rbParallel->
  SetToolTipText("Run the analysis in batch mode (1 batch for each run).",TTDELAY);
 gb->Connect("Clicked(int)","KVDataAnalysisLauncher",this,"SetBatch()");
 cf->AddFrame(gb,eXcY);

 justMode=kTextCenterY|kTextLeft;
 TGCompositeFrame *cf2=new TGCompositeFrame(cf,1600,20,kVerticalFrame);
 TGCompositeFrame *cf3=new TGCompositeFrame(cf2,1600,20,kHorizontalFrame);
 lab=new TGLabel(cf3," Batch Name ");
 lab->SetTextJustify(justMode);
 cf3->AddFrame(lab,eX); 
 chIsBatchNameAuto=new TGCheckButton(cf3," Auto");
 chIsBatchNameAuto->SetTextJustify(justMode);
 chIsBatchNameAuto->
  SetToolTipText("Builds the batch name automatically by using the Batch Name Format.",TTDELAY);
 chIsBatchNameAuto->Connect("Clicked()",
                    "KVDataAnalysisLauncher",this,"SetBatchNameAuto()");
 cf3->AddFrame(chIsBatchNameAuto,eX);
 cf2->AddFrame(cf3,eX); 
 teBatchName=new TGTextEntry(cf2,"");
 teBatchName->SetToolTipText("Enter batch name.",TTDELAY);
 cf2->AddFrame(teBatchName,eX);
 lab=new TGLabel(cf2," Batch Name Format");
 lab->SetTextJustify(justMode);
 cf2->AddFrame(lab,eX); 
 teBatchNameFormat=new TGTextEntry(cf2,"");
 teBatchNameFormat->
  SetToolTipText(Form("%s %s",
  	              "Enter batch name format.",
		      "Examples : $UserClass_$System_$Date or $System_$DataSelector"),
                 TTDELAY);
 cf2->AddFrame(teBatchNameFormat,eX);
 cf->AddFrame(cf2,eXcY);

 this->AddFrame(cf,eX);
 
// Batch parameters
 //if the default batch system does not have time/memory/disk parameters,
 //we do not display the corresponding widgets
if( gBatchSystemManager->GetDefaultBatchSystem()->IsA()->GetMethodAllAny("SetJobTime") ){
   withBatchParams = kTRUE;
 TGCompositeFrame *cfBatchPar=new TGCompositeFrame(this,1600,20,kVerticalFrame);
 cf=new TGCompositeFrame(cfBatchPar,1600,20,kHorizontalFrame);
 fBatchMemLab=new TGLabel(cf,"Batch Memory");
 cf->AddFrame(fBatchMemLab,eX); 
 fBatchDiskLab=new TGLabel(cf,"Batch Disk");
 cf->AddFrame(fBatchDiskLab,eX); 
 fBatchTimeLab=new TGLabel(cf,"Batch Time");
 cf->AddFrame(fBatchTimeLab,eX);
 cfBatchPar->AddFrame(cf,eX); 
 cf=new TGCompositeFrame(cfBatchPar,1600,20,kHorizontalFrame);
 teBatchMemory=new TGTextEntry(cf,"256MB");
 teBatchMemory->SetToolTipText("Enter max memory per job (xKB/xMB/xGB)",TTDELAY);
 teBatchMemory->SetAlignment(kTextRight);
 cf->AddFrame(teBatchMemory,eX);
 teBatchDisk=new TGTextEntry(cf,"200MB");
 teBatchDisk->SetToolTipText("Enter max scratch disk per job (xKB/xMB/xGB)",TTDELAY);
 teBatchDisk->SetAlignment(kTextRight);
 cf->AddFrame(teBatchDisk,eX);
#ifdef USE_KVTIMEENTRY
 teBatchTime=new KVTimeEntry(cf);
 teBatchTime->GetTextEntry()->SetToolTipText("Enter max CPU time per job (ss/mn:ss/hh:mn:ss)",TTDELAY);
#else
 teBatchTime=new TGNumberEntry(cf);
 teBatchTime->GetNumberEntry()->SetToolTipText("Enter max CPU time per job in seconds",TTDELAY);
#ifdef __WITHOUT_TGNUMBERENTRY_SETNUMSTYLE
 teBatchTime->SetFormat(TGNumberFormat::kNESInteger, teBatchTime->GetNumAttr());
#else
 teBatchTime->SetNumStyle(TGNumberFormat::kNESInteger);
#endif
#ifdef __WITHOUT_TGNUMBERENTRY_SETNUMATTR
 teBatchTime->SetFormat(teBatchTime->GetNumStyle(), TGNumberFormat::kNEANonNegative);
#else
 teBatchTime->SetNumAttr(TGNumberFormat::kNEANonNegative);
#endif
#endif
 teBatchTime->SetTime(2,30,0);
 cf->AddFrame(teBatchTime,eX);
 cfBatchPar->AddFrame(cf,eX);  
 this->AddFrame(cfBatchPar,eX); 
}
else
{withBatchParams = kFALSE;}

// Process et Quit
#ifdef KVDAL_DEBUG 
 cout << "Creation Process/Quit" << endl;
#endif
 TGCompositeFrame *cfProcess=new TGCompositeFrame(this,1600,20,kHorizontalFrame);
// Bouton de process
 bout=new TGTextButton(cfProcess,"&Process",B_Process);
 bout->SetToolTipText("Run the analysis.",TTDELAY);
 bout->Connect("Clicked()","KVDataAnalysisLauncher",this,"Process()");
// bout->Associate(this);
 cfProcess->AddFrame(bout,eX);
// Bouton de sortie
 bout=new TGTextButton(cfProcess,"&Quit",B_Quit);
 bout->SetToolTipText("Close GUI and quit.",TTDELAY);
 bout->Connect("Clicked()","KVDataAnalysisLauncher",this,"Exit()");
// bout->Associate(this);
 cfProcess->AddFrame(bout,eX);
 
 this->AddFrame(cfProcess,eX);
// On affiche tout le monde maintenant
 MapSubwindows();

 Resize(GetDefaultSize());

 SetWindowName(Form("KaliVeda Analysis Launcher on %s",gSystem->HostName()));
 SetIconName(Form("KaliVeda Analysis Launcher on %s",gSystem->HostName()));

 MapWindow();
 
 SetRepositoryList();
 ia=0;
 
 FillListOfUserClasses();
 //fill drop down list of user classes in working directory
 SetUserClassList();
 
 // Reset last known state of interface
 TString tmp(GetResource("Repository",""));
 SetRepository(tmp.Data());
 
 tmp=GUIenv->GetValue("KVDataAnalysisLauncher.BatchName","");
 teBatchName->SetText(tmp.Data());
 
 if(GUIenv->GetValue("KVDataAnalysisLauncher.Batch",kFALSE))
  {
#ifdef KVDAL_DEBUG 
  cout << "ajustement des boutons Par/Seq" << endl;
#endif
  if(GUIenv->GetValue("KVDataAnalysisLauncher.SequentialBatch",kTRUE))
   rbSequential->SetState(kButtonDown);
  else
   rbParallel->SetState(kButtonDown);
  }
 else
  rbInteractive->SetState(kButtonDown);

 if(GUIenv->GetValue("KVDataAnalysisLauncher.AutoBatchName",kFALSE))
  chIsBatchNameAuto->SetState(kButtonDown);
 else
  chIsBatchNameAuto->SetState(kButtonUp);

 tmp=GUIenv->GetValue("KVDataAnalysisLauncher.BatchNameFormat","");
 teBatchNameFormat->SetText(tmp.Data());
 
 if(withBatchParams){
   teBatchMemory->SetText(GUIenv->GetValue("KVDataAnalysisLauncher.BatchMemory","256MB"));
   teBatchDisk->SetText(GUIenv->GetValue("KVDataAnalysisLauncher.BatchDisk","200MB"));
   teBatchTime->SetIntNumber(GUIenv->GetValue("KVDataAnalysisLauncher.BatchTime",9000));
}

 fUserLibraries=GUIenv->GetValue("KVDataAnalysisLauncher.UserLibraries","");
 fUserIncludes=GUIenv->GetValue("KVDataAnalysisLauncher.UserIncludes","");
 checkCompilation=kFALSE;

// Connections are made when all the other widgets are mapped
 //teSelector->Connect("TextChanged(const char*)",
 //                    "KVDataAnalysisLauncher",this,"SetAutoBatchName()");
 teDataSelector->Connect("TextChanged(const char*)",
                         "KVDataAnalysisLauncher",this,"SetAutoBatchName()");
 teBatchNameFormat->Connect("TextChanged(const char*)",
                         "KVDataAnalysisLauncher",this,"SetAutoBatchName()");
 
 SetAutoBatchName();
 
}

//__________________________________________
KVDataAnalysisLauncher::~KVDataAnalysisLauncher()
{
   // Destructeur
   if (ia) delete ia;
   if (GUIenv) delete GUIenv;
   delete ResourceNames;
   delete UserClassNames;
}

// On va maintenant agir si un evenemnt se produit
//__________________________________________
Bool_t KVDataAnalysisLauncher::ProcessMessage(Long_t msg, Long_t par1, Long_t par2)
{
// Process messages
#ifdef KVDAL_DEBUG 
 cout << "KVDataAnalysisLauncher::ProcessMessage " << endl;
 cout << msg << " : " << par1 << " / " << par2 << endl;
#endif
 switch(GET_MSG(msg))
  {
//       case kC_CONTAINER:
//          switch (GET_SUBMSG(msg)) {
//             case kCT_ITEMCLICK:
//                if (par1 == kButton1) {
// 						cout << "Now with " << lcRuns->NumSelected() << " selected items" << endl;
//                   if (lcRuns->NumSelected() > 0) {
// 							UpdateListOfSelectedRuns();
//                   }
// 						else {
// 							ClearListOfSelectedRuns();
// 						}
//                }
//                break;
// 					
//             case kCT_SELCHANGED:
// 					cout << "SELCHANGED: number selected = " << par2 << endl;
// 					break;
// 					
//    			default:
//    				break;
// 			}
			
  default:
   break;
  }
 return kTRUE;
}

//__________________________________________
void KVDataAnalysisLauncher::SetRepositoryList(void)
{
// Sets the list of all possible repositories in the repository combo box
#ifdef __WITHOUT_TGCOMBOBOX_REMOVEALL
     RemoveAll(cbRepository);
#else
  cbRepository->RemoveAll();
#endif
  cbRepository->Select(-1);
  TIter next((TList *)gDataRepositoryManager->GetListOfRepositories());
  TObject *o=0;
  Int_t i=0;
  while((o=next()))
   {
   if(((KVDataRepository *)o)->GetDataSetManager()->GetNavailable())
    cbRepository->AddEntry(o->GetName(),i);
   i++;
   }

  cbRepository->Layout();

}

//__________________________________________
void KVDataAnalysisLauncher::SetDataSetList(Char_t *repository)
{
   // Sets the list of all available data sets in the data sets combo box
  SetResource("Repository",repository);
  
  //TString ds=GetDataSet();
  TString ds=GetSavedResource("DataSet", "");
  
#ifdef __WITHOUT_TGCOMBOBOX_REMOVEALL
     RemoveAll(cbDataSet);
#else
  cbDataSet->RemoveAll();
#endif
  cbDataSet->Select(-1);
  gDataRepositoryManager->GetRepository(repository)->cd();
  Int_t nbds=gDataSetManager->GetNavailable();
  Int_t i=0;
  while(i<nbds)
   {
   cbDataSet->AddEntry(gDataSetManager->GetAvailableDataSet(i+1)->GetName(),i);
   i++;
   }

  cbDataSet->Layout();
  
  if(ds.Length())
   {
   SetDataSet(ds.Data());
   }
  else
   {
   SetTask();
   }

}

//__________________________________________
void KVDataAnalysisLauncher::SetTaskList(Char_t *dataset)
{
   // Sets the list of all possible tasks in the tasks combo box
   // Called when a new dataset is selected in the dropdown list
   
  SetResource("DataSet",dataset);
  
   //Info("SetTaskList", "Called with dataset = %s", dataset);

  TString ds=GetSavedResource("Task", "");

#ifdef __WITHOUT_TGCOMBOBOX_REMOVEALL
     RemoveAll(cbTask);
#else
  cbTask->RemoveAll();
#endif
  cbTask->Select(-1);
#ifdef KVDAL_DEBUG 
  cout << "DataSet : [" << dataset << "]" <<  endl;
#endif
  gDataSetManager->GetDataSet(dataset)->cd();
  Int_t nbt=gDataSet->GetNtasks();
  
  GetDataAnalyser()->SetDataSet(gDataSet);
  //no systems defined for dataset ?
  noSystems=(!gIndraDB->GetSystems()->GetSize());
  if(noSystems) cbSystem->SetEnabled(kFALSE);
  else cbSystem->SetEnabled(kTRUE);
  
  Int_t i=0;
  while(i<nbt)
   {
   cbTask->AddEntry(gDataSet->GetAnalysisTask(i+1)->GetType(),i);
   i++;
   }

  cbTask->Layout();

  if(ds.Length())
   {
   SetTask(ds.Data());
   }
  else
   {
   SetSystem();
   }

   //if no systems are defined for dataset, need to set list of triggers now
   //if(noSystems) SetTriggersList(0);
}

KVDataAnalyser* KVDataAnalysisLauncher::GetDataAnalyser(KVDataAnalysisTask* task)
{
   //Get analyser for task
   //If task = 0 we return the current analyser
   
   if(!task){
      if(!ia) ia = new KVDataAnalyser;
   }
   else
   {
      if(ia) delete ia;
      ia = KVDataAnalyser::GetAnalyser( task->GetDataAnalyser() );
      if(!ia) ia = new KVDataAnalyser;
      ia->SetAnalysisTask(task);
   }
   return ia;
}

//__________________________________________
void KVDataAnalysisLauncher::SetSystemList(Int_t itask)
{
// Sets the list of all possible systems in the system combo box
   //Info("SetSystemList", "Called with itask = %d", itask);
   // Called every time a task is selected
   
  KVDataAnalysisTask *task=gDataSet->GetAnalysisTask(itask+1);
  
  GetDataAnalyser(task);
  
  SetResource("Task", task->GetTitle());

  //if needed, set the user class and KVDataSelector fields and/or disable them
  fDataSelectorLabel->SetText("-");
  teDataSelector->SetEnabled(kFALSE);
  teDataSelector->SetToolTipText("");
  if( !task->WithUserClass() ){
     //no user class required
     //teSelector->SetEnabled(kFALSE);
     //teSelector->SetText(task->GetUserBaseClass());
     //teSelector->SetToolTipText("Default analysis class name");
     DisableUserClassList();
  }
  else
  {
     //user class required
     //teSelector->SetEnabled(kTRUE);
     //teSelector->SetToolTipText(Form("Enter name of user analysis class derived from %s", task->GetUserBaseClass()));
     EnableUserClassList();
     //is data selector an option ?
     if( GetDataAnalyser()->IsA()->GetMethodAllAny("SetKVDataSelector") ){
         fDataSelectorLabel->SetText("Data Selector");
         teDataSelector->SetEnabled(kTRUE);
         teDataSelector->SetToolTipText("Enter name of data selector class derived from KVDataSelector");
     }
  }
  //update display
  cfAnalysis->Layout();
  
  TString ds= GetSavedResource("System","");
  
#ifdef __WITHOUT_TGCOMBOBOX_REMOVEALL
     RemoveAll(cbSystem);
#else
  cbSystem->RemoveAll();
#endif
  cbSystem->Select(-1);

if(!noSystems){
   TList *sys_list = gDataSet->GetListOfAvailableSystems(task);
   TIter next(sys_list);
   TObject *o=0;
   Int_t i=1;
   // we put the 'All' option at the top of the list, with 'tag' = 0
   cbSystem->AddEntry("All",0);
   while((o=next())) cbSystem->AddEntry(o->GetName(),i++);
   //pad the list in case less than 6 systems are known
   while( i<6 ) cbSystem->AddEntry("",i++);
   cbSystem->Layout();
   delete sys_list;
}
if(ds.Length())
{
   SetSystem(ds.Data());
}
else
{
   SetRuns();
}
   
//if no systems are defined for dataset, need to set list of triggers now
  if(noSystems) SetTriggersList(0);
}

//__________________________________________
void KVDataAnalysisLauncher::SetTriggersList(Int_t isystem)
{
   // Sets the list of all possible triggers in the triggers combo box
   // Called every time a new system is selected.
   // isystem = index of selected system in drop-down list
   // BUT : because we added "All" as first entry in this list (with index = 0),
   // we have to subtract 1 from isystem in order to get the index of the system
   // in the database list of systems, gDataSet->GetListOfAvailableSystems(task)

   isystem-=1;
      
   //Info("SetTriggersList", "Called with isystem = %d", isystem);
  
#ifdef KVDAL_DEBUG 
  cout << "Task : " <<  cbTask->GetSelected() << endl;
  cout << "System : " <<  isystem << endl;
#endif
  KVDataAnalysisTask *task=gDataSet->GetAnalysisTask(cbTask->GetSelected()+1);
#ifdef KVDAL_DEBUG 
  cout << task->GetName() << endl;
#endif
  KVDBSystem *system=0;
  if(!noSystems && isystem>-1) {
     // case where systems are defined for dataset and user has
     // selected a specific system (not "All" i.e. -1)
     TList* sys_list = gDataSet->GetListOfAvailableSystems(task);
     system = (KVDBSystem *)(sys_list->At(isystem));
     delete sys_list;
  }
  GetDataAnalyser()->SetSystem(system);
  
  KVNumberList run_list = gDataSet->GetRunList(task->GetPrereq(), system);
  KVINDRADBRun *dbrun=0;

  //first read list and find what triggers are available
  Int_t triggers[10], n_trigs = 0;
  run_list.Begin();
  while ( !run_list.End() ) {
      dbrun = (KVINDRADBRun *)gDataSet->GetDataBase()->GetTable("Runs")->GetRecord(run_list.Next());
     if (!KVBase::
  	 ArrContainsValue(n_trigs, triggers, dbrun->GetTrigger())) {
  	triggers[n_trigs++] = dbrun->GetTrigger();
     }
  }
  //sort triggers in ascending order
  Int_t ord_trig[10];
  TMath::Sort(n_trigs, triggers, ord_trig, kFALSE);

#ifdef KVDAL_DEBUG 
  cout << n_trigs << " triggers found " << endl;
#endif
  
  //Set the items of the combobox' pull-down menu
#ifdef __WITHOUT_TGCOMBOBOX_REMOVEALL
     RemoveAll(cbTrigger);
#else
  cbTrigger->RemoveAll();
#endif
  cbTrigger->Select(-1);
  cbTrigger->AddEntry("All",0);
  for(Int_t i=0;i<n_trigs;i++) 
   {
   Int_t trig=triggers[ord_trig[i]];
   TString trigName=Form("M > %d",trig);
#ifdef KVDAL_DEBUG 
   cout << "Adding \"" << trigName.Data() << "\" to the triggers list." << endl;
#endif
   cbTrigger->AddEntry(trigName.Data(),trig);
   }
    
  cbTrigger->Layout();

  // look for saved resource for current repository, dataset & system
  TString trig=GetSavedResource("Trigger","All");
  
#ifdef KVDAL_DEBUG 
  cout << "Ressource trigger for " << 
   Form("KVDataAnalysisLauncher.Trigger.%s",SystemBatchName()) 
   << " : " << trig.Data() << endl;
#endif
  if(cbTrigger->FindEntry(trig.Data()))
   {
   SetTrigger(trig.Data());
   }
  else
   {
   SetTrigger("All");
   }
  
}

//__________________________________________
void KVDataAnalysisLauncher::SetTriggerRunsList(Int_t itrig)
{
// Sets the list of all possible runs in the runs list box for a specific
// trigger
   // Called when a trigger is selected in dropdown list.
   // Also called by SetTrigger

   //Info("SetTriggerRunsList", "Called with itrig = %d", itrig);
   
  Int_t isystem=-1;
  if(!noSystems) isystem = cbSystem->GetSelected() - 1;
  SetResource("Trigger", GetTrigger());
  SetRunsList(isystem,itrig);
}

//__________________________________________
void KVDataAnalysisLauncher::SetRunsList(Int_t isystem, Int_t itrig)
{
// Sets the list of all possible runs in the runs list box
  
   //Info("SetRunsList", "Called with isystem = %d, itrig = %d", isystem, itrig);
   
#ifdef KVDAL_DEBUG 
  cout << "Remove all runs" << endl;
#endif
  listOfRuns.Clear();
  listOfSystemRuns.Clear();
  if(entryMax>-1) 
   {
		lvRuns->RemoveAll();
   	entryMax=-1;
   }
#ifdef KVDAL_DEBUG 
  cout << "Task    : " <<  cbTask->GetSelected() << endl;
  cout << "System  : " <<  isystem << endl;
  cout << "Trigger : " <<  itrig << endl;
#endif
  KVDataAnalysisTask *task=gDataSet->GetAnalysisTask(cbTask->GetSelected()+1);
#ifdef KVDAL_DEBUG 
  cout << task->GetName() << endl;
#endif
  KVDBSystem *system=0;
  if( !noSystems && isystem>-1) {
     // case where systems are defined for dataset and user has
     // selected a specific system (not "All" i.e. -1)
     TList* sys_list = gDataSet->GetListOfAvailableSystems(task);
     system = (KVDBSystem *)(sys_list->At(isystem));
     delete sys_list;
}  
#ifdef KVDAL_DEBUG 
  cout << "System : " << SystemBatchName() << " -> " << system << endl;
#endif
	
  if(IsBatchNameAuto())
   {
   SetAutoBatchName();
   }	      

   //Setting name of system in ressources file
   if(!noSystems){
      // dataset with defined systems
      if(system){
         // user has chosen a system
         SetResource("System",system->GetName());
      }
      else{
         // user chose "All" for system
         SetResource("System","All");
      }
   }
   else
   {
      // no systems defined for dataset
      SetResource("System", "Unknown");
   }

  KVNumberList run_list = gDataSet->GetRunList(task->GetPrereq(), system);
  KVDBRun *dbrun=0;
  Int_t i=0;
  
  run_list.Begin();
  TList list_of_runs;//list of runs to display
  while ( !run_list.End() ) {
      dbrun = (KVDBRun *)gDataSet->GetDataBase()->GetTable("Runs")->GetRecord(run_list.Next());
   Bool_t addRun=kTRUE;

	if(dbrun->InheritsFrom("KVINDRADBRun"))
    {
    if(itrig && ((KVINDRADBRun *)dbrun)->GetTrigger() != itrig)
     {
     addRun=kFALSE;
     }
    }
    // if systems are defined for dataset & user chose "All" for the system,
    // we write the name of the system next to each run
//    if( !noSystems && !system ){
//       if(dbrun->GetSystem()) srun += Form(" {%s} ", dbrun->GetSystem()->GetName());
//    }
   if(addRun)
    list_of_runs.Add(dbrun);
   entryMax=i;
   i++;
   }
  listOfSystemRuns=run_list;
  lvRuns->Display(&list_of_runs);

  //TString ds=GetRuns();
   TString ds=GetSavedResource("RunsList","");
   SetRuns(ds.Data());

   // Set saved user class, data selector, number of events for current
   // repository, dataset, task, system, trigger & runs
 ds = GetSavedResource("UserClass", "");
 SetUserClass( ds.Data() );
 ds=GetSavedResource("KVDataSelector","");
  teDataSelector->SetText(ds.Data());
 ds=GetSavedResource("NbEventsToRead","");
  teNbToRead->SetIntNumber(ds.Atoi());
        
}

//__________________________________________
void KVDataAnalysisLauncher::SetListOfRuns(Int_t irun)
{
   // Sets the list of all possible repositories in the repository combo box
 
   //Info("SetListOfRuns", "Called with irun = %d", irun);
   
 KVDBRun *dbrun=(KVDBRun *)gDataSet->GetDataBase()->GetTable("Runs")->GetRecord(listOfSystemRuns[irun]);
//  if(lbRuns->GetSelection(irun))
//   {
//   listOfRuns.Add(dbrun->GetNumber());
//   }
//  else
  {
  Int_t nv=0;
  Int_t *lor=listOfRuns.GetArray(nv);
  listOfRuns.SetList("");
  for(Int_t i=0;i<nv;i++)
   {
   if(lor[i] != dbrun->GetNumber())
    {
    listOfRuns.Add(lor[i]);
    }
   }
  delete [] lor;
  }
 if(listOfRuns.GetNValues())
  {
  SetResource("RunsList",listOfRuns.AsString());
  }
 selectedRuns->SetText(Form(" Selected Runs : %s",listOfRuns.AsString(MAX_LENGTH_SELECTED_RUNS)));
 selectedRuns->Layout();
  
}

//__________________________________________
void KVDataAnalysisLauncher::SelectAll(void)
{
	// Select all runs currently in the displayed list of runs
	lvRuns->SelectAll(); 
}

//__________________________________________
void KVDataAnalysisLauncher::DeselectAll(void)
{
	// Deselect all runs currently in the displayed list of runs
	lvRuns->UnSelectAll(); 
}

//__________________________________________
void KVDataAnalysisLauncher::Process(void)
{
// Run the analysis task
 
 TString oriIncludePath=gSystem->GetIncludePath();
 
 if(gDataRepository->IsRemote())
  {
  cout << "Checking connection to remote repository." << endl;
  if(!gDataRepository->IsConnected())
   {
   cout << "Connection to server refused" << endl;
   cout << "Process aborted." << endl;
   WarningBox("Connection refused","Connection to server refused\nProcess aborted.");
   return;
   }
  }
  
  KVDataAnalysisTask *task = gDataSet->GetAnalysisTask(cbTask->GetSelected()+1);
  KVDataAnalyser *datan = GetDataAnalyser(task);
  
  //set global pointer to analyser
  gDataAnalyser = datan;
  
 datan->SetDataSet(gDataSet);
 datan->SetAnalysisTask(task);
 if(listOfRuns.GetNValues())
  {
  datan->SetRuns(listOfRuns,kFALSE);
  }
 else
  {
  WarningBox("Empty Run List","The list of runs to process is empty.");
  return;
  }
 
 if(fUserIncludes.Length())
  {
  datan->SetUserIncludes(fUserIncludes.Data());
  }
 if(fUserLibraries.Length())
  {
  datan->SetUserLibraries(fUserLibraries.Data());
  }
 
 //Need a user class for the analysis ?
TString kvs( GetUserClass() );
 if(task->WithUserClass()){
    //read user's class name from input box
    if(kvs.Length())
   {
      datan->SetUserClass(kvs.Data());
      if(checkCompilation) checkCompilation=kFALSE;
   }
   else
   {
      WarningBox("No User Class","Please enter the user analysis class name.");
      return;
   }
}
else if(strcmp(task->GetUserBaseClass(), "")){
   //task with default "user" class (i.e. UserClass=no but UserClass.BaseClass!="")
      datan->SetUserClass(task->GetUserBaseClass(),kFALSE);   
}
 if(datan->InheritsFrom("KVINDRAReconDataAnalyser"))
    ((KVINDRAReconDataAnalyser*)ia)->SetKVDataSelector(teDataSelector->GetText());
 datan->SetNbEventToRead((Long64_t)teNbToRead->GetIntNumber());
 SetResource("RunsList", listOfRuns.AsString());
 //SetResource("UserClass", GetUserClass() );
 SetResource("KVDataSelector",teDataSelector->GetText());
 SetResource("NbEventsToRead", Form("%.0f", teNbToRead->GetNumber()));
 GUIenv->SetValue("KVDataAnalysisLauncher.BatchName",teBatchName->GetText());
 if(withBatchParams){
   GUIenv->SetValue("KVDataAnalysisLauncher.BatchMemory",teBatchMemory->GetText());
   GUIenv->SetValue("KVDataAnalysisLauncher.BatchDisk",teBatchDisk->GetText());
   GUIenv->SetValue("KVDataAnalysisLauncher.BatchTime",(Int_t)teBatchTime->GetIntNumber());
}
 GUIenv->SaveLevel(kEnvUser);
  if(IsBatch()){
      gBatchSystemManager->GetDefaultBatchSystem()->cd();
      gBatchSystem->Clear();
      if(IsBatchNameAuto()) gBatchSystem->SetJobName(teBatchNameFormat->GetText());
      else gBatchSystem->SetJobName(teBatchName->GetText());
      if(gBatchSystem->InheritsFrom("KV_CCIN2P3_BQS")){
         ((KV_CCIN2P3_BQS*)gBatchSystem)->SetJobMemory(teBatchMemory->GetText());
         ((KV_CCIN2P3_BQS*)gBatchSystem)->SetJobDisk(teBatchDisk->GetText());
         ((KV_CCIN2P3_BQS*)gBatchSystem)->SetJobTime((Int_t)teBatchTime->GetIntNumber());
      }
      gBatchSystem->SetMultiJobsMode(rbParallel->IsOn());
      datan->SetBatchSystem(gBatchSystem);
  } 
  else
  {
     datan->SetBatchSystem(0);
  }
 datan->Run();
 
 gSystem->SetIncludePath(oriIncludePath.Data());
}

//__________________________________________
const Char_t *KVDataAnalysisLauncher::GetRepository(void)
 {
 if(TGTextLBEntry *e=(TGTextLBEntry *)cbRepository->GetSelectedEntry())
  {
  return e->GetText()->GetString();
  }
 else
  {
  return "";
  }
 }

//__________________________________________
const Char_t *KVDataAnalysisLauncher::GetDataSet(void)
 {
 if(TGTextLBEntry *e=(TGTextLBEntry *)cbDataSet->GetSelectedEntry())
  {
  return e->GetText()->GetString();
  }
 else
  {
  return "";
  }
 }

//__________________________________________
const Char_t *KVDataAnalysisLauncher::GetTask(void)
 {
 if(TGTextLBEntry *e=(TGTextLBEntry *)cbTask->GetSelectedEntry())
  {
  return e->GetText()->GetString();
  }
 else
  {
  return "";
  }
 }

//__________________________________________
const Char_t *KVDataAnalysisLauncher::GetSystem(void)
 {
    if(noSystems) return "";
 if(TGTextLBEntry *e=(TGTextLBEntry *)cbSystem->GetSelectedEntry())
  {
  return e->GetText()->GetString();
  }
 else
  {
  return "";
  }
 }

//__________________________________________
const Char_t *KVDataAnalysisLauncher::GetTrigger(void)
 {
 if(TGTextLBEntry *e=(TGTextLBEntry *)cbTrigger->GetSelectedEntry())
  {
  return e->GetText()->GetString();
  }
 else
  {
  return "";
  }
 }

//__________________________________________
const Char_t *KVDataAnalysisLauncher::GetRuns(void)
 {
 return listOfRuns.AsString();
 }

//__________________________________________
void KVDataAnalysisLauncher::SetRepository(const Char_t *r)
 {
 TGLBEntry *e=0;
 if((e=cbRepository->FindEntry(r)))
  {
  Int_t i=0;
#ifdef __WITHOUT_TGCOMBOBOX_SELECT_BOOL_T
  cbRepository->Select(i);
#else
  cbRepository->Select(i,kFALSE);
#endif
  while(e!=cbRepository->GetSelectedEntry())
   {
   i++;
#ifdef __WITHOUT_TGCOMBOBOX_SELECT_BOOL_T
    cbRepository->Select(i);
#else
   cbRepository->Select(i,kFALSE);
#endif
   }
  SetDataSetList((Char_t *)r);
  }
 else
  {
  SetDataSet();
  }
 }

//__________________________________________
void KVDataAnalysisLauncher::SetDataSet(const Char_t *r)
 {
 if(!r)
  {
     //remove all datasets because no repository has been chosen yet
#ifdef __WITHOUT_TGCOMBOBOX_REMOVEALL
     RemoveAll(cbDataSet);
#else
  cbDataSet->RemoveAll();
#endif
  cbDataSet->Select(-1);
    SetResource("Repository","");
  SetTask();
  }
 else
  {
  TGLBEntry *e=0;
  if((e=cbDataSet->FindEntry(r)))
   {
   Int_t i=0;
#ifdef __WITHOUT_TGCOMBOBOX_SELECT_BOOL_T
   cbDataSet->Select(i);
#else
   cbDataSet->Select(i,kFALSE);
#endif
   while(e!=cbDataSet->GetSelectedEntry())
    {
    i++;
#ifdef __WITHOUT_TGCOMBOBOX_SELECT_BOOL_T
    cbDataSet->Select(i);
#else
    cbDataSet->Select(i,kFALSE);
#endif
    }
   SetTaskList((Char_t *)r);
   }
  else
   {
   SetTask();
   }
  }
 }

//__________________________________________
void KVDataAnalysisLauncher::SetTask(const Char_t *r)
 {
 if(!r)
  {
     //remove all tasks from list because no dataset chosen yet
#ifdef __WITHOUT_TGCOMBOBOX_REMOVEALL
     RemoveAll(cbTask);
#else
  cbTask->RemoveAll();
#endif
  cbTask->Select(-1);
    SetResource("DataSet","");
  SetSystem();
  }
 else
  {
  TGLBEntry *e=0;
  if((e=cbTask->FindEntry(r)))
   {
   Int_t i=0;
#ifdef __WITHOUT_TGCOMBOBOX_SELECT_BOOL_T
    cbTask->Select(i);
#else
   cbTask->Select(i,kFALSE);
#endif
   while(e!=cbTask->GetSelectedEntry())
    {
    i++;
#ifdef __WITHOUT_TGCOMBOBOX_SELECT_BOOL_T
    cbTask->Select(i);
#else
    cbTask->Select(i,kFALSE);
#endif
    }
   SetSystemList(i);
   }
  else
   {
   SetSystem();
   } 
  }
 }
 
//__________________________________________
void KVDataAnalysisLauncher::SetSystem(const Char_t *r)
 {
   //Info("SetSystem", "Called with system = %s", r);
   
// if((!r) || noSystems)
 if( !r )
  {
     //remove all systems from list because no task chosen yet
#ifdef __WITHOUT_TGCOMBOBOX_REMOVEALL
     RemoveAll(cbSystem);
#else
    cbSystem->RemoveAll();
#endif
    cbSystem->Select(-1);
  //if( !noSystems ) SetTrigger();
    SetResource("Task","");
    //empty list of analysis classes and disable it
    DisableUserClassList();
     SetTrigger();
  }
 else
  {
  TGLBEntry *e=0;
  if((e=cbSystem->FindEntry(r)))
   {
   Int_t i=0;
#ifdef __WITHOUT_TGCOMBOBOX_SELECT_BOOL_T
    cbSystem->Select(i);
#else
   cbSystem->Select(i,kFALSE);
#endif
   while(e!=cbSystem->GetSelectedEntry())
    {
    i++;
#ifdef __WITHOUT_TGCOMBOBOX_SELECT_BOOL_T
    cbSystem->Select(i);
#else
    cbSystem->Select(i,kFALSE);
#endif
    }
   SetTriggersList(i);
   }
  else
   {
   SetTrigger();
   }
  }
 }

//__________________________________________
void KVDataAnalysisLauncher::SetTrigger(const Char_t *r)
 {
   //Info("SetTrigger", "Called with trigger = %s", r);
 if(!r)
  {
     //remove all triggers from list because no systems chosen yet
#ifdef __WITHOUT_TGCOMBOBOX_REMOVEALL
     RemoveAll(cbTrigger);
#else
  cbTrigger->RemoveAll();
#endif
  cbTrigger->Select(-1);
  SetResource("System","");
  SetRuns();
  }
 else
  {
#ifdef KVDAL_DEBUG 
  cout << "Setting trigger " << r << endl;
#endif
  TGLBEntry *e=0;
  if((e=cbTrigger->FindEntry(r)))
   {
   Int_t i=0;
#ifdef __WITHOUT_TGCOMBOBOX_SELECT_BOOL_T
    cbTrigger->Select(i);
#else
   cbTrigger->Select(i,kFALSE);
#endif
   while(e!=cbTrigger->GetSelectedEntry())
    {
    i++;
#ifdef __WITHOUT_TGCOMBOBOX_SELECT_BOOL_T
    cbTrigger->Select(i);
#else
    cbTrigger->Select(i,kFALSE);
#endif
    }
   SetTriggerRunsList(i);
   }
  else
   {
   SetRuns();
   }
  }
 }

//__________________________________________
void KVDataAnalysisLauncher::SetRuns(const Char_t *r)
 {
   //Info("SetRuns", "Called with runs = %s", r);
 if(!r)
  {
      //remove all runs from list because no trigger chosen yet
	   lvRuns->RemoveAll();
	   ClearListOfSelectedRuns();
   	SetResource("Trigger","");
   	SetResource("RunsList","");
  }
 else
  {
  listOfRuns.SetList(r);
  Int_t nbr=0;
  Int_t *lor=listOfRuns.GetArray(nbr);
  ClearListOfSelectedRuns();
  for(Int_t i=0;i<nbr;i++)
   {
   if( listOfSystemRuns.Contains( lor[i] ) )
    {
    Int_t index=0;
    while( lor[i] != listOfSystemRuns[index] ) index++;
	 
	 lvRuns->ActivateItemWithColumnData("Run", (Long_t)lor[i]);
//    SetListOfRuns(index);
    }
   }
 if(listOfRuns.GetNValues())
  {
  SetResource("RunsList",listOfRuns.AsString());
  }
  delete [] lor;
  }
 }
 
//__________________________________________
void KVDataAnalysisLauncher::SetBatch(void)
{
// Set the resource value according to the check box chIsBatch
 GUIenv->SetValue("KVDataAnalysisLauncher.Batch",IsBatch());
 GUIenv->SetValue("KVDataAnalysisLauncher.SequentialBatch",rbSequential->IsDown());
 GUIenv->SaveLevel(kEnvUser);
}

//__________________________________________
void KVDataAnalysisLauncher::SetBatchNameAuto(void)
{
// Set the resource value according to the check box chIsBatch
 GUIenv->SetValue("KVDataAnalysisLauncher.AutoBatchName",IsBatchNameAuto());
 GUIenv->SaveLevel(kEnvUser);
 SetAutoBatchName();
}

//__________________________________________
void KVDataAnalysisLauncher::SetAutoBatchName(void)
{
// Set the automatic batch name
   if(!IsBatchNameAuto())
    {
    return;
    }
   if(strcmp(teBatchNameFormat->GetText(),
             GUIenv->GetValue("KVDataAnalysisLauncher.BatchNameFormat","")))
    {
    GUIenv->SetValue("KVDataAnalysisLauncher.BatchNameFormat",
                    teBatchNameFormat->GetText());
    GUIenv->SaveLevel(kEnvUser);
    }
   
   // check if the pointers are valid before doing anything...
   if(GetDataAnalyser()){
     if(GetDataAnalyser()->GetAnalysisTask()){
       if(GetDataAnalyser()->GetAnalysisTask()->WithUserClass()){
         GetDataAnalyser()->SetUserClass( GetUserClass() ,kFALSE);
         if(GetDataAnalyser()->InheritsFrom("KVINDRAReconDataAnalyser"))
           ((KVINDRAReconDataAnalyser*)GetDataAnalyser())->SetKVDataSelector(teDataSelector->GetText());
       }
     }
     teBatchName->SetText( GetDataAnalyser()->ExpandAutoBatchName(teBatchNameFormat->GetText()) );
   }
}

//__________________________________________
void KVDataAnalysisLauncher::SetUserLibraries(void)
{
// Set the User's libraries
TString ori=fUserLibraries.Data();
new KVGFileList(fUserLibraries,"User's Libraries",
                gClient->GetRoot(),this);
GUIenv->SetValue("KVDataAnalysisLauncher.UserLibraries",fUserLibraries.Data());
GUIenv->SaveLevel(kEnvUser);
checkCompilation=ori.CompareTo(fUserLibraries);
}

//__________________________________________
void KVDataAnalysisLauncher::SetUserIncludes(void)
{
// Set the User's includes
TString ori=fUserIncludes.Data();
new KVGDirectoryList(fUserIncludes,"User's Includes",
                     gClient->GetRoot(),this);
GUIenv->SetValue("KVDataAnalysisLauncher.UserIncludes",fUserIncludes.Data());
GUIenv->SaveLevel(kEnvUser);
checkCompilation=ori.CompareTo(fUserIncludes);
}


//__________________________________________
void KVDataAnalysisLauncher::WarningBox(const char *title, const char *msg)
{
// Warning box in case of problems
#if ROOT_VERSION_CODE < ROOT_VERSION(5,0,0)
new TGMsgBox(0,this,title,msg,kMBIconExclamation);
#else
new TGMsgBox(gClient->GetRoot(),this,title,msg,kMBIconExclamation);
#endif
}

//__________________________________________

#ifdef __WITHOUT_TGCOMBOBOX_REMOVEALL
void KVDataAnalysisLauncher::RemoveAll(TGComboBox*box)
{
   //replaces functionality of TGComboBox::RemoveAll for ROOT versions < 5.11/02
   
   Int_t n = box->GetListBox()->GetNumberOfEntries();
   if (n) box->RemoveEntries(0, n-1);
   if (box->GetSelectedEntry()) {
      ((TGTextLBEntry*)box->GetSelectedEntry())->SetTitle("");
      fClient->NeedRedraw(box->GetSelectedEntry());
   } else {
      box->GetTextEntry()->SetTitle("");
      fClient->NeedRedraw(box->GetTextEntry());
   }
}
void KVDataAnalysisLauncher::RemoveAll(TGListBox*box)
{
   //replaces functionality of TGListBox::RemoveAll for ROOT versions < 5.11/02
   
   Int_t n = box->GetNumberOfEntries();
   if (n) box->RemoveEntries(0, n-1);
   if (box->GetSelectedEntry()) {
      ((TGTextLBEntry*)box->GetSelectedEntry())->SetTitle("");
      fClient->NeedRedraw(box->GetSelectedEntry());
   } 
}
#endif

//_________________________________________________________________________________________________

void KVDataAnalysisLauncher::SetResource(const Char_t* name, const Char_t* value)
{
   // Handles resource file ".KVDataAnalysisGUIrc"
   // We store the current state of the interface using the following resource names:
   //
   // Repository              KVDataAnalysisLauncher.Repository
   // DataSet                   KVDataAnalysisLauncher.DataSet
   // Task                         KVDataAnalysisLauncher.Task
   // System                    KVDataAnalysisLauncher.System
   // Trigger                    KVDataAnalysisLauncher.Trigger
   // RunsList                  KVDataAnalysisLauncher.RunsList
   // UserClass               KVDataAnalysisLauncher.UserClass
   // KVDataSelector      KVDataAnalysisLauncher.KVDataSelector
   // NbEventsToRead    KVDataAnalysisLauncher.NbEventsToRead
   //
   // We also keep a "memory" of all selected configurations using the following
   // resource names:
   //
   // DataSet                   KVDataAnalysisLauncher.DataSet.[repository]
   // Task                         KVDataAnalysisLauncher.Task.[repository].[dataset]
   // System                    KVDataAnalysisLauncher.System.[repository].[dataset].[task]
   // Trigger                    KVDataAnalysisLauncher.Trigger.[repository].[dataset].[task].[system]
   // RunsList                  KVDataAnalysisLauncher.RunsList.[repository].[dataset].[task].[system].[trigger]
   // UserClass               KVDataAnalysisLauncher.UserClass.[repository].[dataset].[task].[system].[trigger]
   // KVDataSelector      KVDataAnalysisLauncher.KVDataSelector.[repository].[dataset].[task].[system].[trigger]
   // NbEventsToRead    KVDataAnalysisLauncher.NbEventsToRead.[repository].[dataset].[task].[system].[trigger]
   //
   // N.B. [task]:  it is the NAME of the task which is used in the resource name.
   // N.B.2. [system]:  it is the name given by SystemBatchName() which is used in the resource name.
   // N.B.3. [trigger]:  for the resource name we take "M > 4", "M>=8" "All" etc. and replace all ' ', '>' or '=' by ''
   //                          to give "M4", "M8", "All", etc.
   
   TString res, ful_res;
   
   BuildResourceName(name, res, ful_res);
  
   // save current value of resource 
   GUIenv->SetValue( res.Data(), value );
   
   // save resource for future use if possible
   if( ful_res.Length() ) GUIenv->SetValue( ful_res.Data(), value );
   
   GUIenv->SaveLevel(kEnvUser);
}

//_________________________________________________________________________________________________

const Char_t* KVDataAnalysisLauncher::GetResource(const Char_t* name, const Char_t* defaultvalue)
{
   // Handles resource file ".KVDataAnalysisGUIrc"
   //
   // We return the current value of the resource "name"
   
   TString res = name;
   res.Prepend("KVDataAnalysisLauncher.");
   return GUIenv->GetValue( res.Data(), defaultvalue );
}

//_________________________________________________________________________________________________

const Char_t* KVDataAnalysisLauncher::GetSavedResource(const Char_t* name, const Char_t* defaultvalue)
{
   // Handles resource file ".KVDataAnalysisGUIrc"
   //
   // We look for a stored value of the resource "name" corresponding to the current
   // values of all the resources which come before "name" in the list :
   //    Repository              
   //    DataSet                
   //    Task                       
   //    System             
   //    Trigger               
   // These values are stored in resources with names like:
   //    KVDataAnalysisLauncher.[name].[repository].[dataset]...
   //
   // If no stored value is found, the defaultvalue is returned
   
   TString res, ful_res;
   
   BuildResourceName(name, res, ful_res);
   
   if( !ful_res.Length() ){    
      //cout << "GetSavedResource: current resource : " << res.Data() << endl;
      return GUIenv->GetValue( res.Data(), defaultvalue );
   }
   
   //cout << "GetSavedResource: saved resource : " << ful_res.Data() << endl;
   return GUIenv->GetValue( ful_res.Data(), defaultvalue );
}

//____________________________________________________________________________________________________

const Char_t *KVDataAnalysisLauncher::SystemBatchName(void)
{
   // Get the system name for the batch name
   
   TString tmp = GetSystem();
   if( tmp == "All" ) return GetSystem();
   return GetDataAnalyser()->SystemBatchName();
}

//____________________________________________________________________________________________________

void KVDataAnalysisLauncher::BuildResourceName(const Char_t* name, TString &cur_res, TString &saved_res)
{
   // Build the full resource names for storing DataSet, Task, etc.
   //
   // We store the current state of the interface using the following resource names:
   //
   //  "name"                       "cur_res"
   // Repository              KVDataAnalysisLauncher.Repository
   // DataSet                   KVDataAnalysisLauncher.DataSet
   // Task                         KVDataAnalysisLauncher.Task
   // System                    KVDataAnalysisLauncher.System
   // Trigger                    KVDataAnalysisLauncher.Trigger
   // RunsList                  KVDataAnalysisLauncher.RunsList
   // UserClass               KVDataAnalysisLauncher.UserClass
   // KVDataSelector      KVDataAnalysisLauncher.KVDataSelector
   // NbEventsToRead    KVDataAnalysisLauncher.NbEventsToRead
   //
   // We also keep a "memory" of all selected configurations using the following
   // resource names:
   //
   //  "name"                       "saved_res"
   // DataSet                   KVDataAnalysisLauncher.DataSet.[repository]
   // Task                         KVDataAnalysisLauncher.Task.[repository].[dataset]
   // System                    KVDataAnalysisLauncher.System.[repository].[dataset].[task]
   // Trigger                    KVDataAnalysisLauncher.Trigger.[repository].[dataset].[task].[system]
   // RunsList                  KVDataAnalysisLauncher.RunsList.[repository].[dataset].[task].[system].[trigger]
   // UserClass               KVDataAnalysisLauncher.UserClass.[repository].[dataset].[task].[system].[trigger]
   // KVDataSelector      KVDataAnalysisLauncher.KVDataSelector.[repository].[dataset].[task].[system].[trigger]
   // NbEventsToRead    KVDataAnalysisLauncher.NbEventsToRead.[repository].[dataset].[task].[system].[trigger]
   //
   // N.B. [task]:  it is the NAME of the task which is used in the resource name.
   // N.B.2. [system]:  it is the name given by SystemBatchName() which is used in the resource name.
   // N.B.3. [trigger]:  for the resource name we take "M > 4", "M>=8" "All" etc. and replace all ' ', '>' or '=' by ''
   //                          to give "M4", "M8", "All", etc.
   //
   // If name = "Repository", saved_res="" as we do not save it.
   // If the resource cannot be saved because one of the resources that is needed to form the
   // full resource name has not been set, saved_res="".
   
   //Resource name for current value
   cur_res = name;
   cur_res.Prepend("KVDataAnalysisLauncher.");
   
   //Build full name for save
   Int_t index=-1;   saved_res = "";
   // look for resource name in list ResourceNames
   TObject* resource=0;
   if( (resource = ResourceNames->FindObject( name )) ){
      //get index in resource table
      index = ResourceNames->IndexOf( resource );
   }
   
   if( index == 0 )  // resource name = "Repository"; nothing more to do
   {
      return;
   }
   
   if ( index == -1 )  index = NbResNames; // force loop to end of list
   
   // resource name is written in format KVDataAnalysisLauncher.[name].[repository]....
   // where the suffixed resource values are all those in the list before the named resource
   // i.e. for name = "Task" we write the resource KVDataAnalysisLauncher.Task.[repository].[dataset]
   
   saved_res = cur_res;
   TIter next_res(ResourceNames);
   Int_t i=0; Bool_t ok=kTRUE;
   
   while( (resource = next_res()) && (i++<index) ){
      
      TString tmp( GetResource(resource->GetName()) );
      TString res;
      if( tmp == "" ){
         // one of required resources is not set - none of following resources will be set either
         // we cannot save this resource
         ok = kFALSE;
         break;
      }
      if( !strcmp(resource->GetName(), "Task") && gDataSet ){
         // translate title to name for task
         KVDataAnalysisTask* tsk = gDataSet->GetAnalysisTask( tmp.Data() );
         if(tsk) res.Form( ".%s", tsk->GetName() );
      }
      else if( !strcmp(resource->GetName(), "System") ){
         // use SystemBatchName
         res.Form( ".%s", SystemBatchName() );
      }
      else if( !strcmp(resource->GetName(), "Trigger") ){
         // turn "M > 4" into "M4"
         tmp.ReplaceAll(" ", "");
         tmp.ReplaceAll(">", "");
         tmp.ReplaceAll("=", "");
         res.Form( ".%s", tmp.Data() );
      }
      else
      {
         res.Form( ".%s", GetResource(resource->GetName()) );
      }
      saved_res += res;
   }
   
   if( !ok ) saved_res="";
}
 
//____________________________________________________________________________________________________

void KVDataAnalysisLauncher::FillListOfUserClasses()
{
   // Look at files in working directory & deduce list of user analysis classes.
   // We look for any file ending in '.h'. If we can find a corresponding '.cpp' or '.C' or '.cxx',
   // we consider that it is a user analysis class. This list is used to fill the "User Class"
   // drop-down list.
   
   TSystemDirectory dir("LocDir", ".");
   TList *lf = dir.GetListOfFiles();
   if(!lf) return;
   UserClassNames->Clear();
   //loop over file names
   TIter next(lf);
   while( TObject* file = next() ){
      
      // fill list with all '.h' files
      TString tmp( file->GetName() );
      if( tmp.EndsWith(".h") ) {
         //strip '.h' from filename
         tmp.Remove( tmp.Index(".h") );
         UserClassNames->Add( new TNamed( tmp.Data(), tmp.Data()) );
      }
      
   }
   delete lf;
   // now check that implementation files exist for all '.h' we found
   TIter next_cl( UserClassNames ); KVString imp, dec;
   while( TNamed *clh = (TNamed*)next_cl() ){
      if( !KVBase::FindClassSourceFiles( clh->GetName(), imp, dec ) ) clh->SetName("_INVALID_");
   }
   // remove all invalid class names
   while( TObject *obj = UserClassNames->FindObject( "_INVALID_" ) ) {
      UserClassNames->Remove( obj );
      delete obj;
   }
  // Info("FillListOfUserClasses", "User classes found in working directory:");
  // UserClassNames->ls();
}

//__________________________________________

void KVDataAnalysisLauncher::SetUserClassList()
{
   // Sets the list of all available user classes in the drop down list
  
   //Info("SetUserClassList", "called");
   
#ifdef __WITHOUT_TGCOMBOBOX_REMOVEALL
     RemoveAll(cbUserClass);
#else
  cbUserClass->RemoveAll();
#endif
  cbUserClass->Select(-1);
  
  Int_t nbcl=UserClassNames->GetEntries();
  Int_t i=0;
  while(i<nbcl)
   {
      cbUserClass->AddEntry( UserClassNames->At(i)->GetName() , i );
      i++;
   }
   cbUserClass->Layout();
}

//__________________________________________

void KVDataAnalysisLauncher::UserClassSelected(char *class_name)
{
   // Called when a user class is selected in the combo box.
   // Updates batch name if 'auto batch name' is selected.
   
   //Info("UserClassSelected", "User class selected : %s", class_name);
   
   if(IsBatchNameAuto()) SetAutoBatchName();
   
   // save resource
   SetResource( "UserClass", class_name );
}

//__________________________________________

void KVDataAnalysisLauncher::SetUserClass(const Char_t *class_name)
{
   // Sets selected user class in combo box according to e.g. a previously stored resource value.
   // Updates batch name if 'auto batch name' is selected.
   // We update the resource corresponding to the current state of the interface.
   
   //Info("SetUserClass", "Set user class : %s", class_name);
   
   // look for user class in list
   TGLBEntry* e = cbUserClass->FindEntry( class_name );
  //cbUserClass->EnableTextInput(kFALSE);
   
   if( e ){
      //Info("SetUserClass", "Found class in list");
      
      Int_t i=0;
#ifdef __WITHOUT_TGCOMBOBOX_SELECT_BOOL_T
      cbUserClass->Select(i);
#else
      cbUserClass->Select(i,kFALSE);
#endif
      while(e!=cbUserClass->GetSelectedEntry()){
         i++;
#ifdef __WITHOUT_TGCOMBOBOX_SELECT_BOOL_T
         cbUserClass->Select(i);
#else
         cbUserClass->Select(i,kFALSE);
#endif
      }
      
      // save current user class
      SetResource( "UserClass", class_name );
   }
   
   else
    // unknown user class  
   {
      //Info("SetUserClass", "Class not found in list");
      cbUserClass->Select(-1);
      SetResource( "UserClass", "" );
   }
   
  //cbUserClass->EnableTextInput(kTRUE);
}

//__________________________________________

const Char_t* KVDataAnalysisLauncher::GetUserClass()
{
   // Returns currently selected user class name

 if(TGTextLBEntry *e=(TGTextLBEntry *)cbUserClass->GetSelectedEntry())
  {
  return e->GetText()->GetString();
  }
 else
  {
  return "";
  }
}

//__________________________________________

void KVDataAnalysisLauncher::DisableUserClassList()
{
   // Remove all entries from user class combo box & disable text entry
   
   //Info("DisableUserClassList", "called");
#ifdef __WITHOUT_TGCOMBOBOX_REMOVEALL
  RemoveAll(cbUserClass);
#else
  cbUserClass->RemoveAll();
#endif
  cbUserClass->Select(-1);
  cbUserClass->SetEnabled(kFALSE);
  //cbUserClass->EnableTextInput(kFALSE);
}

//__________________________________________

void KVDataAnalysisLauncher::EnableUserClassList()
{
   // Reenable user class combo box & text entry,
   // fill list with all known user classes & select the one corresponding
   // to the current environment
   
   //Info("EnableUserClassList", "called");
   
  SetUserClassList();
  SetUserClass( GetSavedResource( "UserClass", "" ) );
  //cbUserClass->EnableTextInput(kTRUE);
  cbUserClass->SetEnabled(kTRUE);
}

//__________________________________________

void KVDataAnalysisLauncher::EnterRunlist()
{
   // Called when user presses "Runlist" button.
   // Open dialogue box in which a runlist can be entered.
   // The runs in the runlist will be selected.
   
   TString runs = listOfRuns.AsString();
   Bool_t ok = kFALSE;
   new KVInputDialog(this, "Enter list of runs", &runs, &ok);
   if (ok) {
      DeselectAll();
      SetRuns(runs.Data());
   }
}

//__________________________________________

void KVDataAnalysisLauncher::UpdateListOfSelectedRuns()
{
	// Called when the selected runs in TGListView lvRuns change.
	// We update the KVNumberList listOfRuns according to the current selection
	listOfRuns.Clear();
	TList* novolist = lvRuns->GetSelectedItems();
	if( novolist->GetEntries() > 0 ){
		TIter nxt(novolist); TObjString* s=0;
		while( (s=(TObjString*)nxt()) ) listOfRuns.Add( s->String().Data() );
	}
	delete novolist;
   SetResource("RunsList",listOfRuns.AsString());
   selectedRuns->SetText(Form(" Selected Runs : %s",listOfRuns.AsString(MAX_LENGTH_SELECTED_RUNS)));
   fClient->NeedRedraw(selectedRuns);
}

//__________________________________________

void KVDataAnalysisLauncher::ClearListOfSelectedRuns()
{
	// Empty displayed list of selected runs
	listOfRuns.Clear();
   SetResource("RunsList","");
   selectedRuns->SetText(Form(" Selected Runs : %s",listOfRuns.AsString(MAX_LENGTH_SELECTED_RUNS)));
   fClient->NeedRedraw(selectedRuns);
}
