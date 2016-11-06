#ifndef RM_FILESCAN
#define RM_FILESCAN
#include "rm_record.h"
#include <vector>

class RM_FileScan {
  public:
    std::vector<Record> reclist;
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

}

RM_FileScan::~RM_FileScan() {

}

RC RM_FileScan::openScan(const RM_FileHandle &fileHandle,
                         AttrType      attrType,
                         int           attrLength,
                         int           attrOffset,  
                         CompOp        compOp,
                         void          *value) {
  if (value == NULL) {

  }
}

RC RM_FileScan::getNextRec(RM_Record &rec) {

}

RC RM_FileScan::closeScan() {

}

#endif //RM_FILESCAN