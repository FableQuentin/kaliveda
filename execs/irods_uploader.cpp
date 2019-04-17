// upload files to irods server at cc.in2p3 and (optionally)
// delete them after successful transfer
//
// Usage:
//
//  irods_uploader [-D] [directory to scan] [dataset]
//
//    options: -D  - if given, delete each file which has been successfully transferred
//
//    for each run R, there can be a series of files:
//        run_R.dat.[date/time]
//        run_R.dat.[date/time].1
//        run_R.dat.[date/time].2
//        run_R.dat.[date/time].3
//    etc.
//
//    - a file run_R.*.I is only transferrable if file run_R.*.I+1 or run_R+1.* exists
//    in the directory, this means that writing file run_R.*.I has finished.
//    - if file run_R.*.I is found on the irods server AND has the same file size as the local file,
//    then the local file can be deleted (if option -D is given)

#include "KVString.h"
#include <IRODS.h>
#include <KVDataRepositoryManager.h>
#include <KVDataSetManager.h>
#include <KVDataRepository.h>
#include <KVSystemDirectory.h>
#include <KVSystemFile.h>
#include <iostream>
#include <map>
#include <KVMFMDataFileReader.h>
#include "KVAvailableRunsFile.h"
using namespace std;

struct runfile_t {
   KVString name;
   KVString date;
   Long64_t size;
};

struct run_t {
   map<int, runfile_t> files;
};

class file_helper {
public:
   KVString scan_dir, dataset;
   KVString file_format;
   int index_multiplier;

   bool file_has_index_number(const KVString& name, int& index)
   {
      // return true if filename has an index number
      index = 0;
      if (name.GetNValues(".") == 3) return false;
      name.Begin(".");
      for (int i = 0; i < 3; i++) name.Next();
      index = name.Next().Atoi();
      return true;
   }

   bool find_next_sequential_file(int run0, int index0, int& run, int& index, runfile_t& runfile)
   {
      // look for next file after (run0,index0) in directory
      // return false if no file found

      KVSystemDirectory dir("data", scan_dir);
      TList* files = dir.GetListOfFiles();
      if (run0) {
         run = run0;
         index = index0 + 1;
      }
      else {
         run = 1;
         index = 0;
      }
      // sort files into ordered map run[index]=filename
      map<int, run_t> runs;
      TIter nxt(files);
      KVSystemFile* f;
      while ((f = (KVSystemFile*)nxt())) {
         if (!f->IsDirectory()) {
            int ir;
            if ((ir = KVAvailableRunsFile::IsRunFileName(file_format, f->GetName(), index_multiplier))) {
               int idx = ir % index_multiplier;
               ir /= index_multiplier;
               runfile_t rf;
               rf.name = f->GetName();
               rf.size = f->GetSize();
               rf.date = f->GetDate();
               runs[ir].files[idx] = rf;
            }
         }
      }
      // now look for (run,index)
      // if not found, look for (run+1,0)
      map<int, run_t>::iterator find_run = runs.find(run);
      if (find_run != runs.end()) {
         map<int, runfile_t>::iterator find_index = find_run->second.files.find(index);
         if (find_index != find_run->second.files.end()) {
            runfile = find_index->second;
            return true;
         }
      }
      ++run;
      index = 0;
      find_run = runs.find(run);
      if (find_run != runs.end()) {
         map<int, runfile_t>::iterator find_index = find_run->second.files.find(index);
         if (find_index != find_run->second.files.end()) {
            runfile = find_index->second;
            return true;
         }
      }
      return false;
   }
   void read_and_store_infos_on_file(const runfile_t& runfile)
   {
      KVString path_to_file;
      path_to_file.Form("%s/%s", scan_dir.Data(), runfile.name.Data());
      int run_num = KVAvailableRunsFile::IsRunFileName(file_format, runfile.name, index_multiplier);
      KVDatime when;
      KVAvailableRunsFile::ExtractDateFromFileName(file_format, runfile.name, when);
      KVNameValueList infos;
      infos.SetName(Form("run%010d", run_num));
      infos.SetValue("Run", run_num);
      infos.SetValue("Start", when.AsSQLString());
      infos.SetValue("End", runfile.date);
      infos.SetValue("Size", (int)runfile.size);

      KVMFMDataFileReader reader(path_to_file);
      ULong64_t events = 0;
      // correct start date/time from infos in file ?
      if (reader.GetRunInfos().HasStringParameter("FileCreationTime")) {
         when.SetGanacqNarvalDate(reader.GetRunInfos().GetStringValue("FileCreationTime"));
         infos.SetValue("Start", when.AsSQLString());
      }
      while (reader.GetNextEvent()) {
         cout << "\xd" << "Reading events " << ++events << flush;
      }
      cout << endl;
      infos.SetValue64bit("Events", events);
      infos.Print();

      TFile f(Form("%s/indrafazia/%s/runinfos.root", KVBase::GetKVSourceDir(), dataset.Data()), "update");
      infos.Write();

      // copy ACTIONS file to source directory
      gSystem->CopyFile(reader.GetPathToLastEbyedatActionsFile(), Form("%s/indrafazia/%s/ebyedat/%s", KVBase::GetKVSourceDir(), dataset.Data(), gSystem->BaseName(reader.GetPathToLastEbyedatActionsFile())));
   }
};

int main(int argc, char* argv[])
{
   if (argc < 3 || argc > 4) {
      cout << "Usage: irods_uploader [-D] [directory to scan] [dataset]" << endl << endl;
      cout << "  options: -D  - if given, delete each file which has been successfully transferred" << endl;
      exit(1);
   }
   file_helper FILE_H;
   bool delete_files = !strcmp(argv[1], "-D");
   if (delete_files) {
      FILE_H.scan_dir = argv[2];
      FILE_H.dataset = argv[3];
   }
   else {
      FILE_H.scan_dir = argv[1];
      FILE_H.dataset = argv[2];
   }
   cout << "Directory to scan: " << FILE_H.scan_dir << "  dataset: " << FILE_H.dataset << endl;
   if (delete_files) cout << "Any transferred files will be deleted from local disk" << endl;

   KVDataRepositoryManager drm;
   drm.Init();
   drm.GetRepository("ccali")->cd();
   KVDataSet* dataset = gDataSetManager->GetDataSet(FILE_H.dataset);
   FILE_H.file_format = dataset->GetDataSetEnv("DataSet.RunFileName.raw", "");
   FILE_H.index_multiplier = dataset->GetDataSetEnv("DataSet.RunFileIndexMultiplier.raw", -1.);
   cout << "format " << FILE_H.file_format << " mult " << FILE_H.index_multiplier << endl;

   int current_run(0), current_index(0);
   runfile_t current_file;
   int next_run(0), next_index(0);
   runfile_t next_file;
   while (1) {
      while (FILE_H.find_next_sequential_file(current_run, current_index, next_run, next_index, next_file)) {
         if (current_run > 0) {
            // check if current run has been uploaded
            FileStat_t fs;
            if (gDataRepository->GetFileInfo(dataset, "raw", current_file.name, fs)) {
               // check size of uploaded file
               if (fs.fSize == current_file.size) {
                  cout << "File " << current_file.name << " has been uploaded successfully & can be deleted" << endl;
               }
               else {
                  cout << "File " << current_file.name << " partially uploaded, size IRODS=" << fs.fSize << " size local disk=" << current_file.size << endl;
               }
            }
            else {
               cout << "File " << current_file.name << " ready for upload [next file: " << next_file.name << "]" << endl;
               FILE_H.read_and_store_infos_on_file(current_file);
               if (gDataRepository->CopyFileToRepository(
                        Form("%s/%s", FILE_H.scan_dir.Data(), current_file.name.Data()),
                        dataset, "raw", current_file.name.Data()
                     ) == 0) {
                  // successful transfer
                  cout << "File upload successful" << endl;
                  continue;
               }
               else {
                  cout << "            *************** ERROR uploading file *************** " << endl;
               }
            }
         }
         current_run = next_run;
         current_index = next_index;
         current_file = next_file;
      }
      cout << "Waiting for next file after " << current_file.name << " ... " << endl;
      sleep(5);
   }
}