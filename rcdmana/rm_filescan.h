#ifndef RM_FILESCAN
#define RM_FILESCAN
#include "rm_record.h"
#include "rm_rid.h"
#include "../utils/pagedef.h"
#include <vector>

class RM_FileScan {
  public:

    RM_FileHandle *fh;
    AttrType      attrType;
    int           attrLength;
    int           attrOffset;  
    CompOp        compOp;
    void          *value;

    RM_Record record;
    int recordScanNum;
    RID rid;
       RM_FileScan  ();                                // Constructor
       ~RM_FileScan ();                                // Destructor
    RC openScan     (RM_FileHandle *fileHandle,  // Initialize file scan
                     AttrType      attrType,
                     int           attrLength,
                     int           attrOffset,
                     CompOp        compOp,
                     void          *value);
    RC getNextRec   (RM_Record &rec);                  // Get next matching record
    RC closeScan    ();                                // Terminate file scan

  private:

    RC incRID();
};

using namespace std;

RM_FileScan::RM_FileScan() {
}

RM_FileScan::~RM_FileScan() {

}

RC RM_FileScan::openScan(RM_FileHandle *fileHandle,
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
  if ((attrOffset+attrLength) >= fh->recordSize) {
    cout << "scan record error with size invalid." << endl;
    return 107;
  }

  this->rid = RID(1, 0);
  this->recordScanNum = 0;
  this->attrType = attrType;
  this->attrLength = attrLength;
  this->attrOffset = attrOffset;
  this->compOp = compOp;
  this->value = value;
  return 0;
}

RC RM_FileScan::getNextRec() {
  char data[this->fh->recordSize];
  bool isSelected = false;
  cout << "ready scan" << endl;
  do {
    if (this->recordScanNum == this->fh->recordNumForAllPages) {
      return RM_EOF;
    }
    cout << "no RM_EOF" << endl;

    // this->fh->getRec(rid, this->record);
    while (this->fh->getRec(rid, this->record) == 108) {
      incRID();
    }

    cout << "no unuse record" << endl;
    char *td = data;
    this->record.getData(td);
    char* comValue = data+this->attrOffset;
    if (attrType == ATTRINT) {
      cout << "compare int" << endl;
      int intValue = *(int*)(comValue);
      cout << intValue << endl;
      if (intValue == *(int*)(value)) {
        isSelected = true;
      }
    }
    else if (attrType == ATTRFLOAT) {
      float floatValue = *(float*)(comValue);
      if (floatValue == *(float*)(value)) {
        isSelected = true;
      }
    }
    else if (attrType == ATTRSTRING) {
      isSelected = true;
      char* stringValue = (char*)(value);
      for (int i = 0; i < attrLength; ++i) {
        if (comValue[i] != stringValue[i]) {
          isSelected = false;
          break;
        }
      }
    }

    incRID();
    ++recordScanNum;
  } while ((value != NULL) && !isSelected);

  rec = this->record;

  return 0;
}

RC RM_FileScan::closeScan() {
  // vector <int>().swap(reclist);
  fh = NULL;
  value = NULL;
}

RC RM_FileScan::incRID() {
  if ((this->rid.slotNum+1) < this->fh->recordNumForEachPage) {
    this->rid.slotNum ++;
  }
  else{
    this->rid.pageNum ++;
    this->rid.slotNum = 0;
  }

  return 0;
}

#endif //RM_FILESCAN
