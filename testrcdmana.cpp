#include "rcdmana/rm.h"
#include "rcdmana/rm_rid.h"
#include "rcdmana/rm_record.h"
#include "rcdmana/rm_filescan.h"
#include <string.h>
//#include "csinglon.h"
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
using namespace std;

int main() {
    RecordManager *prm = RecordManager::getInstance();
    const char *fileName = "database.txt";
    vector<int> attrType, attrLength, isNull;
    for (int i = 0; i < 3; ++i) {
        attrType.push_back(1);
        attrLength.push_back(4);
        isNull.push_back(1);
    }
    prm->createFile(fileName, 12, attrType, attrLength, isNull, 1);

    RM_FileHandle fileHandle(prm->pfm, prm->bpm);
    prm->openFile(fileName, fileHandle);
    RID rid;
    /*prm->fileHandle->insertRec("123451345512", rid);
    cout << rid << endl;
    prm->fileHandle->insertRec("123451345512", rid);
    cout << rid << endl;
    prm->fileHandle->insertRec("This is one ", rid);
    prm->fileHandle->insertRec("123451345512", rid);
    prm->fileHandle->insertRec("123451345512", rid);
*/
    RM_Record rec;
    /*prm->fileHandle->getRec(rid, rec);
    cout << string(rec.data) << endl;

    rec.setData("this is another case");
    prm->fileHandle->updateRec(rec);

    prm->fileHandle->getRec(rid, rec);
    cout << string(rec.data) << endl;

    prm->fileHandle->deleteRec(rid);
    prm->fileHandle->insertRec("33333333333333333333", rid);
*/
    int b = 40100200;
    char data[12];
    strcpy(data, "abcd");
    for (int i = 0; i < 4; ++i) {
        char t = *((char*)(&b)+i);
        cout << t << " ddd" << endl;
        strcat(data, &t);
        cout << data << " ddd " << strlen(data) << endl;
    }
    strcat(data, "1234");
    cout << "size : " << strlen(data) << endl;
    fileHandle.insertRec(data, rid);
    b = 20; 
    strcpy(data, "1111");
    for (int i = 0; i < 4; ++i) {
        strcat(data, (char*)(&b)+i);
    }
    strcat(data, "3333");
    fileHandle.insertRec(data, rid);
    // a = {'a', 'b', 'c', 'd'}; 
    b = 345; 
    // c = {'a', 'b', 'c', 'd'};
    strcpy(data, "2222");
    for (int i = 0; i < 4; ++i) {
        strcat(data, (char*)(&b)+i);
    }
    strcat(data, "6666");
    fileHandle.insertRec(data, rid);

    RM_FileScan fs;
    b = 40100200;
    void* value = (void*)(&b);
    fs.openScan(fileHandle,
                ATTRINT,
                4,
                4,
                GT_OP,
                value);
    fs.getNextRec(rec);
    if (!rec.empty()) {
        cout << string(rec.data) << " , " << rec.rid << endl;
    }
    fs.closeScan();

    prm->closeFile(fileHandle);

    prm->destroyFile(fileName);
    return 0;
}
