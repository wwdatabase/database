#ifndef RM_RID
#define RM_RID
#include "../utils/pagedef.h"

class RID {

public:
    int pageNum, slotNum;
    
public:
    RID(){}
    ~RID(){}
    RID(int pageNum, int slotNum){
    	this->pageNum = pageNum;
    	this->slotNum = slotNum;
    }

    RC getPageNum(int &pageNum) const {
    	pageNum = this->pageNum;
    	return 0;
    }
    
    RC getSlotNum(int &slotNum) const {
    	slotNum = this->slotNum;
    	return 0;
    }
};

#endif //RM_RID
