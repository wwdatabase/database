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
    /* file relative variable, fixed when opening file
     */
	const char *fileName;
    int fileID;

    /* info loaded from table file in page index 0
     */
        int recordSize;
        /* a map size represents weather records in page
         * is available.
         */
        int tagSize;
        /* number of one page can store record log, and 
         * total number record have been insert into database.
         */
        int recordNumForEachPage, recordNumForAllPages;
        /* stored in the first page representing relative page
         * is available.
         */
        int bitmapSize;
        unsigned int *bitmap;


    /* page relative variable, may change after opening file
     */
    int pageID;
    BufType bufType;
    int index;

private:
    FileManager *pfm;
    BufPageManager *bpm;

public:
    RM_FileHandle(FileManager *pfm, BufPageManager *bpm);
    ~RM_FileHandle();

    /* get record by RID, 
     */
    RC getRec(const RID &rid, RM_Record &rec);

    /* insert a record, return RID to indentify.
     */
    RC insertRec(const char *data, RID &rid);

    /* delete specfic record with RID.
     */
    RC deleteRec(const RID &rid);

    /* update record with RID.
     */
    RC updateRec(const RM_Record &rec); 

    RC init(const int &recordSize,
            const int &recordNumForEachPage,
            const int &tagSize,
            const int &recordForAllPages,
            const int &bitmapSize,
            unsigned int *bitmap);

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

RC RM_FileHandle::init(const int &recordSize,
                       const int &recordNumForEachPage,
                       const int &tagSize,
                       const int &recordForAllPages,
                       const int &bitmapSize,
                       unsigned int *bitmap) {
    this->recordSize = recordSize;
    this->recordNumForEachPage = recordNumForEachPage;
    this->tagSize = tagSize;
    this->recordNumForAllPages = recordNumForAllPages;
    this->bitmapSize = bitmapSize;
    this->bitmap = bitmap;
    return 0;
}

RC RM_FileHandle::getRec(const RID &rid, RM_Record &rec){
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
        // cout << "error delete" << endl;
        return 108;
    }
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

/*
 * Here is a bug, while insert a record into page, nerver check
 * out weather there is full record to change bitmap status and 
 * the last pageNum variable.
 * Same as the delete record func. Solved at 6/Nov/2016
 * May occur overflow problem, as first page to be limited length.
 */
RC RM_FileHandle::insertRec(const char* data, RID &rid) {

  /* select page id for insert record */
  searchPageID:
    this->bufType = this->bpm->getPage(this->fileID,
                                       0,
                                       this->index);
    BufType b = this->bufType;

    int page_id = 0;

    int pageIndex = 0;
    for (int i = 0; i < bitmapSize; ++i) {
        unsigned int &bitBlock = this->bitmap[i];
        for (int j = 0; j < 32; ++j) {
            if ((bitBlock & (1 << j)) == 0){
                page_id = i * 32 + j + 1;
                break;
            }
        }
        if (page_id != 0)
            break;

        pageIndex = i+1;
    }

    /* current bitmap pages have been full, 
     * ask for more 32 pages.
     */
    if (page_id == 0) {
        ++this->bitmapSize;
        b[4] = this->bitmapSize;
        unsigned int &bitmap = b[5+bitmapSize];
        bitmap = 0;
        this->bpm->markDirty(this->index);

        page_id = this->bitmapSize*32;
    }

    this->pageID = page_id;
    this->bufType = this->bpm->getPage(this->fileID, 
                                 this->pageID,
                                 this->index);
    b = this->bufType;

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

    /* the selected page is full exactly,
     * but not labeled full,
     */
    if (tagID == -1) {
        int bitmapInt = pageID / 32;
        unsigned int &bitBlock = this->bitmap[bitmapInt];
        int bitmapBit = pageID % 32;
        bitBlock |= (1 << bitmapBit);

        goto searchPageID;
    }
    
    char *rcd = (char *)(b + recordSize * tagID);
    strcpy(rcd, data);
    cout << ">>> insert record : (" << fileID << ", " << pageID << ", " << tagID << ") "
         << string(rcd) << endl;
    // cout << "insert record : " << string(rcd) << endl;

    bpm->markDirty(this->index);

    rid.pageNum = pageID;
    rid.slotNum = tagID;

    /* fresh table info with page index 0
     */
    this->bufType = this->bpm->getPage(this->fileID,
                                 0,
                                 this->index);
    b = this->bufType;
    ++recordNumForAllPages;
    b[3] = recordNumForAllPages;
    this->bpm->markDirty(this->index);

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
        return 108;
    }

    cout << ">>> delete record : (" << fileID << ", " << pageID << ", " << tagID << ")" << endl;
    tagBlock &= ~(1 << tagBit);

    bpm->markDirty(index);

    /* fresh table info with page index 0
     */
    this->bufType = this->bpm->getPage(this->fileID,
                                 0,
                                 this->index);
    b = this->bufType;
    --recordNumForAllPages;
    b[3] = recordNumForAllPages;
    this->bpm->markDirty(this->index);

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
    cout << ">>> update record : (" << fileID << ", " << pageID << ", " << tagID << ") " 
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
