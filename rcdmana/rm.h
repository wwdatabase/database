#ifndef RECORD_MANAGER
#define RECORD_MANAGER
#include <string>
#include <cstring>
#include <vector>
#include <cstdio>
#include <map>
#include "../bufmanager/BufPageManager.h"
#include "../fileio/FileManager.h"
#include "../utils/pagedef.h"
#include "rm_filehandle.h"

/*
 * The RecordManager class handles the creation, deletion,
 * opening, and closing of files of records in the RM
 * component. 
 * class RecordManager only maintains exactly one instance 
 * file for easy operation and higher security. 
 * More details is here:
 * http://web.stanford.edn/class/cs346/2015/redbase-rm.html
 */
class RecordManager {

public:
    // RM_FileHandle *fileHandle;

    RecordManager ();
    ~RecordManager();

    FileManager *pfm;
    BufPageManager *bpm;

    static RecordManager *m_pInstance;
    class Garbo {
    public:
        ~Garbo() {
            if (RecordManager::m_pInstance)
                delete RecordManager::m_pInstance;
        }
    };
    
public:
    /* The program should create exactly one instance of this
     * class, and all requests for RcdMana component file 
     * management should be directly to this instance.
     */
    static RecordManager * getInstance() {
        if (m_pInstance == NULL)
            m_pInstance = new RecordManager();
        return m_pInstance;
    }

    /* create a paged file with records will have const size
     * "recordSize".
     */
    RC createFile(const char *fileName,
                    int recordSize,
                    vector<int> attrType,
                    vector<int> attrLength,
                    vector<int> isNull,
                    int priKeyNum);

    /* destroy file whose name is "fileName". 
     */
    RC destroyFile(const char *fileName);

    /* open the file called "fileName", if the method is 
     * sucessful, the "fileHandle" should become a "handle"
     * for the open file.
     */
    RC openFile(const char *fileName, RM_FileHandle &fileHandle);

    /* close the open file instance referred to by "fileHandle".
     */
    RC closeFile(RM_FileHandle &fileHandle);

private:
    std::map<std::string, std::string> parseTableStruct(const char *table);

};

using namespace std;

RecordManager * RecordManager::m_pInstance = 0;

RecordManager::RecordManager() {
    MyBitMap::initConst();
    this->pfm = new FileManager();
    this->bpm = new BufPageManager(this->pfm);
}

RecordManager::~RecordManager() {
    // delete fileHandle;
    delete pfm;
    delete bpm;
}

/*
static RecordManager * RecordManager::getInstance() {
    if (m_pInstance == NULL)
        m_pInstance = new RecordManager();
    return m_pInstance;
}
 */

RC RecordManager::createFile(const char *fileName,
                            int recordSize,
                            vector<int> attrType,
                            vector<int> attrLength,
                            vector<int> isNull,
                            int priKeyNum) {
    if (!pfm->createFile(fileName)){
        cout << "create file error!" << endl;
    }
    cout << ">>> create file " << fileName << " sucessfully." << endl;

    /* init */
    RM_FileHandle fileHandle(this->pfm, this->bpm);
    openFile(fileName, fileHandle);
    BufType b = fileHandle.bufType;

    /* set record size. */
    int offset = 0;
    b[offset ++] = recordSize;

    /* set attr num. */
    int attrNum = attrType.size();
    b[offset ++] = attrNum;

    /* set primary key num. */
    b[offset ++] = priKeyNum;

    for (int i = 0; i < attrNum; ++i) {
        b[offset ++] = attrType[i];
        b[offset ++] = attrLength[i];
        b[offset ++] = isNull[i];
    }

    /* set record log number for each page. */
    double par = recordSize + 1.0 / 8; 
    int rcdTop = (PAGE_SIZE - 1) / par;
    if (rcdTop == 0){
        cout << "record size is too large" << endl;
        return 105;
    }
    else
        b[offset++] = rcdTop;

    /* set record tag size with bit map in each page. */
    int tagSize = rcdTop / 8 + 1;
    if (rcdTop * recordSize + tagSize + 3 > PAGE_SIZE) {
        cout << "error attribute tags map !" << endl;
    }
    b[offset++] = tagSize;

    /* set record log number for all pages. */
    int rcdNum = 0;
    b[offset++] = rcdNum;

    /* set bitmap size (devided by 8 by default). */
    b[offset++] = 4;

    /* set page bit map in file, init with 32 pages' map. */
    unsigned int &bitmap = b[offset++];
    bitmap = 0;
    /*for (int i = 0; i < 32; ++i)
        bitmap &= ~(1 << i); // set i bit zero
     */

    bpm->markDirty(fileHandle.index);
    return closeFile(fileHandle);
}

/* no use function */
map<string, string> RecordManager::parseTableStruct(const char *table) {
    string identifier = "";
    vector<string> strList;
    int idx, len = sizeof(table);
    while (idx < len) {
        if ((table[idx] == ' ') || (table[idx] == ',')) {
            if (identifier != "")
                strList.push_back(identifier);
            identifier = "";
        }
        else {
            identifier += table[idx];
        }
        idx ++;
    }
    map<string, string> tableStruct;
    for (idx = 0; idx < strList.size(); ++idx) {
      //  tableStruct.insert(map<string, string>::value_type(strList[idx], strList[++idx])); 
    }

    return tableStruct;
}

RC RecordManager::destoryFile(const char *fileName) {
    //cout << "remove :" << fileName << endl;
    if (remove(fileName) == 0) {
        cout << ">>> destroy file " << fileName << " sucessful." << endl;
        return 0;
    }
    else{
        cout << "error remove" << endl;
        return 102;
    }
}

RC RecordManager::openFile(const char *fileName, RM_FileHandle &fileHandle) {
    fileHandle.fileName = fileName;
    if (!pfm->openFile(fileName, fileHandle.fileID)){
        cout << "error open file : " << fileName << endl;
        return 103;
    }
    cout << endl;
    cout << ">>> open file " << fileName << " sucessfully." << endl;
    fileHandle.bufType = bpm->getPage(fileHandle.fileID, 
                                        fileHandle.pageID,
                                        fileHandle.index);
    BufType b = fileHandle.bufType;
    fileHandle.init();
    cout << "******file " << fileName << " info******" << endl;
    int offset = 0;
    cout << "* record size : " << b[offset] << "\n" ;
    fileHandle.recordSize = b[offset ++ ];
    fileHandle.attrNum = b[offset ++];
    fileHandle.priKeyNum = b[offset ++];
    for (int i = 0; i < fileHandle.attrNum; ++i) {
        fileHandle.attrType.push_back(b[offset ++]);
        fileHandle.attrLength.push_back(b[offset ++]);
        fileHandle.isNull.push_back(b[offset ++]);
    }
    cout << "* record number in each page : " << b[offset] << "\n" ;
    fileHandle.recordNumForEachPage = b[offset ++];
    cout << "* tags map size in each page : " << b[offset] << "\n" ;
    fileHandle.tagSize = b[offset ++];
    cout << "* record number for total pages : " << b[offset] << "\n";
    fileHandle.recordNumForAllPages = b[offset ++];
    cout << "* bitmap size : " << b[offset]*8 << "\n" ;
    fileHandle.bitmapSize = b[offset ++];
    fileHandle.bitmap = b + offset;
    
    cout << "* bitmap : " ;
    unsigned int bitmap;
    for (int i = 0; i < fileHandle.bitmapSize; ++i) {
        bitmap = b[offset+i];
        for (int j = 0; j < 8; ++j) {
            cout << (bitmap & (1 << j));
        }
    }
    cout << endl;
    cout << "******end file info************" << endl;
    
    return 0;
}

RC RecordManager::closeFile(RM_FileHandle &fileHandle) {
    if (fileHandle.fileName == NULL){
        cout << "close file error." << endl;
        return 104;
    }

    bpm->close();
    pfm->closeFile(fileHandle.fileID);
    fileHandle.cleanFileHandle();
    cout << ">>> close file sucessfully." << endl;
    return 0;
}


#endif //RECORD_MANAGER
