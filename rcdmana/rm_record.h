#ifndef RM_RECORD
#define RM_RECORD
#include "../utils/pagedef.h"
#include "rm_rid.h"
#include <map>
#include <cstring>
#include <string>

class RM_Record {

public:
    char *data;
    RID rid;

public:
    RM_Record(const char *data){
        data = NULL;
        setData(data);
    }
    RM_Record() {
        data = NULL;
    }
    ~RM_Record(){
        if (data != NULL)
            delete []data;
    }

    RC setData(const char *pData) {
        if (this->data != NULL) {
            delete [] this->data;
            this->data = NULL;
        }
        if (pData == NULL)
            return 0;
        int len = strlen(pData);
        this->data = new char[len];
        strcpy(this->data, pData);
        return 0;
    }
    RC getData(char *pData) const{
        if (data == NULL) {
            cout << "Error with record, get data func" << endl;
        }
    	strcpy(pData, data);
    	return 0;
    }
    RC getRid(RID &rid) const {
        this->rid.getPageNum(rid.pageNum );
        this->rid.getSlotNum(rid.slotNum );
    	return 0;
    }
    RC clean() {
        if (this->data != NULL) {
            delete [] this->data;
            this->data = NULL;
        }
        return 0;
    }
    bool empty() const {
        return (this->data == NULL);
    }

    RM_Record& operator= (RM_Record &rec) {
        this->setData(rec.data);
        this->rid = rec.rid;
        return *this;
    }
};

#endif //RM_RECORD
