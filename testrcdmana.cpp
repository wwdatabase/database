#include "rcdmana/rm.h"
#include "rcdmana/rm_rid.h"
#include "rcdmana/rm_record.h"
//#include "csinglon.h"
#include <iostream>
#include <string>
using namespace std;

int main() {
    RecordManager *prm = RecordManager::getInstance();
    const char *fileName = "database.txt";
    prm->createFile(fileName, 20);

    prm->openFile(fileName);
    RID rid;
    prm->fileHandle->insertRec("12345134551234512345", rid);
    cout << rid << endl;
    prm->fileHandle->insertRec("12345134551234512345", rid);
    cout << rid << endl;
    prm->fileHandle->insertRec("This is one case lal", rid);
    prm->fileHandle->insertRec("12345134551234512345", rid);
    prm->fileHandle->insertRec("12345134551234512345", rid);

    RM_Record rec;
    prm->fileHandle->getRec(rid, rec);
    cout << string(rec.data) << endl;

    rec.setData("this is another case");
    prm->fileHandle->updateRec(rec);

    prm->fileHandle->getRec(rid, rec);
    cout << string(rec.data) << endl;

    prm->fileHandle->deleteRec(rid);
    prm->fileHandle->insertRec("33333333333333333333", rid);

    prm->closeFile();

    prm->destoryFile(fileName);
    return 0;
}
