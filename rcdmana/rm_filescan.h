#ifndef RM_FILESCAN
#define RM_FILESCAN
#include "rm_record.h"
#include <vector>

class RM_FileScan {
  public:
    const RM_FileHandle &fh;
    std::vector<Record> reclist;
    RID rid;
       RM_FileScan  ();                                // Constructor
       ~RM_FileScan ();                                // Destructor
    RC openScan     (const RM_FileHandle &fileHandle,  // Initialize file scan
                     AttrType      attrType,
                     int           attrLength,
                     int           attrOffset,
                     CompOp        compOp,
                     void          *value);
    RC getNextRec   (RM_Record &rec);                  // Get next matching record
    RC closeScan    ();                                // Terminate file scan
};

using namespace std;

RM_FileScan::RM_FileScan() {
  rid = RID(1, 0);
}

RM_FileScan::~RM_FileScan() {

}

RC RM_FileScan::openScan(const RM_FileHandle &fileHandle,
                         AttrType      attrType,
                         int           attrLength,
                         int           attrOffset,  
                         CompOp        compOp,
                         void          *value) {
  fh = fileHandle;
  if (fh->fileName == NULL) {
    cout << "scan file error with file not open." << endl;
    return 106;
  }
  if (value == NULL) {
    
  }
}

RC RM_FileScan::getNextRec(RM_Record &rec) {

}

RC RM_FileScan::closeScan() {
  vector <int>().swap(reclist);
}

#endif //RM_FILESCAN