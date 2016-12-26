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
  cout << "Open scan" << endl;
  return 0;
}

RC RM_FileScan::getNextRec(RM_Record &rec) {
    cout << "Get record" << endl;
  char data[this->fh->recordSize];
  bool isSelected = false;
  bool rid_error = false;
  do {
    if (this->recordScanNum >= this->fh->recordNumForAllPages) {
        cout << "File EOF" << endl;
        this->record.clean();
        break;
    }

    while (this->fh->getRec(rid, this->record) == 108) {
        cout << "Unused record, ignore it." << endl;
        incRID();
    }

    char *td = data;
    this->record.getData(td);
    char comValue[this->attrLength];
    strncpy(comValue, data+this->attrOffset, this->attrLength);
    if (attrType == ATTRINT) {
      int *intValue = (int*)(comValue);
      // int c = 40100200;
      // int *intValue = &c;
      int *cmpValue = (int*)(value);
      cout << *intValue << " : " << *cmpValue << endl;
      switch (this->compOp) {
          case EQ_OP:
              if ((*intValue) == (*cmpValue))
                  isSelected = true;
              cout << isSelected << endl;
              break;
          case LT_OP:
              if (*intValue < *cmpValue)
                  isSelected = true;
              break;
          case GT_OP:
              if (*intValue > *cmpValue)
                  isSelected = true;
              break;
          case LE_OP:
              if (*intValue <= *cmpValue)
                  isSelected = true;
              break;
          case GE_OP:
              if (*intValue >= *cmpValue)
                  isSelected = true;
              break;
          case NE_OP:
              if (*intValue != *cmpValue)
                  isSelected = true;
              break;
          case NO_OP:
              /* No comparision (when value is a null pointer) */
              isSelected = true;
              break;
          default:
              cout << "compOp Error at compare INT" << endl;
              break;
      }
    }
    else if (attrType == ATTRFLOAT) {
      float *floatValue = (float*)(comValue);
      float *cmpValue = (float*)(value);
      switch (this->compOp) {
          case EQ_OP:
              if (*floatValue == *cmpValue)
                  isSelected = true;
              break;
          case LT_OP:
              if (*floatValue < *cmpValue)
                  isSelected = true;
              break;
          case GT_OP:
              if (*floatValue > *cmpValue)
                  isSelected = true;
              break;
          case LE_OP:
              if (*floatValue <= *cmpValue)
                  isSelected = true;
              break;
          case GE_OP:
              if (*floatValue >= *cmpValue)
                  isSelected = true;
              break;
          case NE_OP:
              if (*floatValue != *cmpValue)
                  isSelected = true;
              break;
          case NO_OP:
              /* No comparision (when value is a null pointer) */
              isSelected = true;
              break;
          default:
              cout << "Compop error at compare FLOAT" << endl;
              break;
      }
    }
    else if (attrType == ATTRSTRING) {
      isSelected = true;
      char *strValue = (char*)(comValue);
      char* cmpValue = (char*)(value);
      switch (this->compOp) {
          case EQ_OP:
              if (strncmp(strValue, cmpValue, this->attrLength) == 0)
                  isSelected = true;
              break;
          case LT_OP:
              if (strncmp(strValue, cmpValue, this->attrLength) < 0)
                  isSelected = true;
              break;
          case GT_OP:
              if (strncmp(strValue, cmpValue, this->attrLength) > 0)
                  isSelected = true;
              break;
          case LE_OP:
              if (strncmp(strValue, cmpValue, this->attrLength) <= 0)
                  isSelected = true;
              break;
          case GE_OP:
              if (strncmp(strValue, cmpValue, this->attrLength) >= 0)
                  isSelected = true;
              break;
          case NE_OP:
              if (strncmp(strValue, cmpValue, this->attrLength) != 0)
                  isSelected = true;
              break;
          case NO_OP:
              /* No comparision (when value is a null pointer) */
              isSelected = true;
              break;
          default:
              cout << "Compop error at compare FLOAT" << endl;
              break;
      }
    }

    incRID();
    ++recordScanNum;
  } while ((value != NULL) && !isSelected);

  rec = this->record;
  // rec.setData(this->record.data);
  // rec.rid = this->record.rid;
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
