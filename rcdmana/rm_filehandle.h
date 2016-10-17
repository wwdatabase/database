#ifndef RM_FILEHANDLE
#define RM_FILEHANDLE
#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <map>
#include "../utils/pagedef.h"
#include "rm_rid.h"
#include "rm_record.h"

class RM_FileHandle {

public:
	const char *fileName;
    int fileID;
    int recordSize;
    int tagSize;
    int recordNumForEachPage, recordNumForAllPages;
    int bitmapSize;
    unsigned int *bitmap;

    int pageID;
    BufType bufType;
    int index;

private:
    FileManager *pfm;
    BufPageManager *bpm;

public:
    RM_FileHandle(FileManager *pfm, BufPageManager *bpm);
    ~RM_FileHandle();

    RC getRec(const RID &rid, RM_Record &rec);
    RC insertRec(const char *data, RID &rid);
    RC deleteRec(const RID &rid);
    RC updateRec(const RM_Record &rec); 
    RC cleanFileHandle();
};

using namespace std;

RM_FileHandle::RM_FileHandle(FileManager *pfm, BufPageManager *bpm){
    cleanFileHandle();
    this->pfm = pfm;
    this->bpm = bpm;
}

RM_FileHandle::~RM_FileHandle(){

}

RC RM_FileHandle::getRec(const RID &rid, RM_Record &rec){
    rid.getPageNum(this->pageID);
    this->bufType = bpm->getPage(this->fileID,
                                 this->pageID,
                                 this->index);
    BufType b = this->bufType;
    int tagID ;
    rid.getSlotNum(tagID);
    //cout << "get record : " << this->pageID << ", " << tagID << endl;
    //cout << "size of buttype : " << b[0] << endl;
    char *rcd = (char *)(b + this->recordSize * tagID);
    //cout << string(rcd) << endl;
    char data[20];
    strcpy(data, rcd);
    rec.setData(data);
    rec.rid = rid;

    bpm->access(this->index);

    return 0;
}

RC RM_FileHandle::insertRec(const char* data, RID &rid) {
    BufType b = this->bufType;

    /* select page id for insert record */
    int pageID = 0;
    for (int i = 0; i < bitmapSize; ++i) {
        unsigned int &bitBlock = bitmap[i];
        for (int j = 0; j < 32; ++j) {
            if ((bitBlock & (1 << j)) == 0){
                pageID = i * 32 + j + 1;
                break;
            }
        }
        if (pageID != 0)
            break;
    }
    this->pageID = pageID;
    this->bufType = bpm->getPage(this->fileID, 
                                 this->pageID,
                                 this->index);
    b = this->bufType;
    //cout << "bufType : " << b[1] << endl;

    /* select tags for insert record */
    int tagID = -1;
    for (int i = 0; i < this->recordNumForEachPage; ++i){
        int tagInt = i / 32;
        unsigned int &tagBlock = b[PAGE_INT_NUM-1-tagInt];
        int tagBit = i % 32;
        if ((tagBlock & (1 << tagBit)) == 0){
            tagID = i;
            tagBlock |= (1 << tagBit);
            break;
        }
    }
    
    char *rcd = (char *)(b + recordSize * tagID);
    strcpy(rcd, data);
    cout << "insert record : (" << fileID << ", " << pageID << ", " << tagID << ") "
         << string(rcd) << endl;
    //cout << "insert record : " << string(rcd) << endl;
    //cout << "insert addr : " << rcd << endl;

    bpm->markDirty(this->index);

    rid.pageNum = pageID;
    rid.slotNum = tagID;

    return 0;
}

RC RM_FileHandle::deleteRec(const RID &rid) {
    rid.getPageNum(this->pageID);
    this->bufType = bpm->getPage(this->fileID,
                                 this->pageID,
                                 this->index);
    BufType b = this->bufType;
    int tagID ;
    rid.getSlotNum(tagID);
    int tagInt = tagID / 32;
    int tagBit = tagID % 32;
    unsigned int &tagBlock = b[PAGE_INT_NUM-1-tagInt];

    // check bit correct
    if ((tagBlock & (1 << tagBit)) == 0) {
        cout << "error delete" << endl;
    }

    cout << "delete record : (" << fileID << ", " << pageID << ", " << tagID << ")" << endl;
    tagBlock &= ~(1 << tagBit);

    bpm->markDirty(index);
    return 0;
}

RC RM_FileHandle::updateRec(const RM_Record &rec) {
    rec.rid.getPageNum(this->pageID);
    this->bufType = bpm->getPage(this->fileID,
                                 this->pageID,
                                 this->index);
    BufType b = this->bufType;
    int tagID ;
    rec.rid.getSlotNum(tagID);

    char *rcd = (char *)(b + recordSize * tagID);
    rec.getData(rcd);
    cout << "update record : (" << fileID << ", " << pageID << ", " << tagID << ") " 
         << string(rcd) << endl;

    bpm->markDirty(index);

    return 0;
}

RC RM_FileHandle::cleanFileHandle(){
    this->fileName = NULL;
    this->bufType = NULL;
    this->bitmap = NULL;
    this->fileID = 0;
    this->pageID = 0;
    this->index = 0;
    this->recordSize = 0;
    this->tagSize = 0;
    this->recordNumForEachPage = 0;
    this->recordNumForAllPages = 0;
    this->bitmapSize = 0;

    return 0;
}

#endif //RM_FILEHANDLE
