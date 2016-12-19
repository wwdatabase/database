#include "rcdmana/rm.h"
#include "rcdmana/rm_rid.h"
#include "rcdmana/rm_record.h"
#include "rcdmana/rm_filescan.h"
#include <string.h>
//#include "csinglon.h"
#include <iostream>
#include <string>
using namespace std;

int main() {
    RecordManager *prm = RecordManager::getInstance();
    const char *fileName = "database.txt";
    prm->createFile(fileName, 12);

    prm->openFile(fileName);
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
    // int a = 3;
    // char a[4];
    int b = 40100200;
    // float c = 5.0;
    // char c[4] = {'a', 'b', 'c', 'd'};
    char data[12];
    strcpy(data, "abcd");
    for (int i = 0; i < 4; ++i) {
        char t = *((char*)(&b)+i);
        cout << t << "ddd" << endl;
        strcat(data, &t);
        cout << data << "ddd " << strlen(data) << endl;
    }
    strcat(data, "1234");
    cout << "size : " << strlen(data) << endl;
    prm->fileHandle->insertRec(data, rid);
    // a = {'a', 'b', 'c', 'd'}; 
    b = 20; 
    // c = {'a', 'b', 'c', 'd'};
    strcpy(data, "1111");
    for (int i = 0; i < 4; ++i) {
        strcat(data, (char*)(&b)+i);
    }
    strcat(data, "3333");
    prm->fileHandle->insertRec(data, rid);
    // a = {'a', 'b', 'c', 'd'}; 
    b = 345; 
    // c = {'a', 'b', 'c', 'd'};
    strcpy(data, "2222");
    for (int i = 0; i < 4; ++i) {
        strcat(data, (char*)(&b)+i);
    }
    strcat(data, "6666");
    prm->fileHandle->insertRec(data, rid);

    RM_FileScan fs;
    b = 20;
    void* value = (void*)(&b);
    fs.openScan(prm->fileHandle,
                ATTRINT,
                4,
                4,
                EQ_OP,
                value);
    fs.getNextRec(rec);
    cout << string(rec.data) << " , " << rec.rid << endl;

    prm->closeFile();

    prm->destoryFile(fileName);
    return 0;
}
