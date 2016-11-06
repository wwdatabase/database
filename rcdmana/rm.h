#ifndef RECORD_MANAGER
#define RECORD_MANAGER
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
    RM_FileHandle *fileHandle;

private:
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
    RC createFile(const char *fileName, int recordSize);

    /* destory file whose name is "fileName". 
     */
    RC destoryFile(const char *fileName);

    /* open the file called "fileName", if the method is 
     * sucessful, the "fileHandle" should become a "handle"
     * for the open file.
     */
    RC openFile(const char *fileName);

    /* close the open file instance referred to by "fileHandle".
     */
    RC closeFile();

private:
    std::map<std::string, std::string> parseTableStruct(const char *table);

};

using namespace std;

RecordManager * RecordManager::m_pInstance = 0;

RecordManager::RecordManager() {
    MyBitMap::initConst();
    this->pfm = new FileManager();
    this->bpm = new BufPageManager(this->pfm);
    this->fileHandle = new RM_FileHandle(this->pfm, this->bpm);
}

RecordManager::~RecordManager() {
    delete fileHandle;
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

RC RecordManager::createFile(const char *fileName, int recordSize) {
    if (!pfm->createFile(fileName)){
        cout << "create file error!" << endl;
    }
    cout << ">>> create file " << fileName << " sucessfully." << endl;

    /* init */
    openFile(fileName);
    BufType b = fileHandle->bufType;

    /* set record size, offset 0 */
    int offset = 0;
    //int tagSize = 1; // tag size is 1 byte;
    b[offset++] = recordSize;

    /* set record log number for each page, offset 1 */
    int par = recordSize * 32;
    int rcdTop = (PAGE_SIZE / (recordSize)) * (par - 1) / par;
    if (rcdTop == 0){
        cout << "record size is too large" << endl;
        return 105;
    }
    else
        b[offset++] = rcdTop;

    /* set record tag size with bit map in each page, offset 2 */
    int tagSize = rcdTop / 32 + 1;
    if (rcdTop * recordSize + tagSize + 3 > PAGE_SIZE) {
        cout << "error attribute tags map !" << endl;
    }
    b[offset++] = tagSize;

    /* set record log number for all pages, offset 3 */
    int rcdNum = 0;
    b[offset++] = rcdNum;

    /* parse tableStruct 
    b[3] = sizeof(table);
    cout << sizeof(table) << endl;
    char* tableStruct = (char*)(b + 4);
    strcpy(tableStruct, table);
     */
    // fileHandle->tableStruct = parseTableStruct(table); 

    /* set bitmap size (devided by 32 by default), offset 4 */
    b[offset++] = 1;

    /* set page bit map in file, init with 32 pages' map, offset 5 */
    unsigned int &bitmap = b[offset++];
    bitmap = 0;
    /*for (int i = 0; i < 32; ++i)
        bitmap &= ~(1 << i); // set i bit zero
     */

    bpm->markDirty(fileHandle->index);
    return closeFile();
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
        tableStruct.insert(map<string, string>::value_type(strList[idx], strList[++idx])); 
    }

    return tableStruct;
}

RC RecordManager::destoryFile(const char *fileName) {
    if((fileHandle->fileName != NULL) && 
        strcmp(fileHandle->fileName, fileName) == 0) {
        cout << "file is opening by fileHandle, close it before destory!" << endl;
        return 101;
    }
    
    //cout << "remove :" << fileName << endl;
    if (remove(fileName) == 0) {
        cout << ">>> destory file " << fileName << " sucessful." << endl;
        return 0;
    }
    else{
        cout << "error remove" << endl;
        return 102;
    }
}

RC RecordManager::openFile(const char *fileName) {
    fileHandle->fileName = fileName;
    if (!pfm->openFile(fileName, fileHandle->fileID)){
        cout << "error open file : " << fileName << endl;
        return 103;
    }
    cout << endl;
    cout << ">>> open file " << fileName << " sucessfully." << endl;
    fileHandle->bufType = bpm->getPage(fileHandle->fileID, 
                                        fileHandle->pageID,
                                        fileHandle->index);
    BufType b = fileHandle->bufType;
    fileHandle->init(b[0], b[1], b[2], b[3], b[4], b+5);
    /*fileHandle->recordSize = b[0];
    fileHandle->recordNumForEachPage = b[1];
    fileHandle->tagSize = b[2];
    fileHandle->recordNumForAllPages = b[3];
    fileHandle->bitmapSize = b[4];
    fileHandle->bitmap = b+5;
    */
    cout << "******file " << fileName << " info******" << endl;
    cout << "* record size : " << b[0] << "\n" 
         << "* record number in each page : " << b[1] << "\n" 
         << "* tags map size in each page : " << b[2] << "\n"
         << "* record number for total pages : " << b[3] << "\n"
         << "* bitmap size : " << b[4]*32 << "\n" 
         << "* bitmap : " ;
    unsigned int bitmap;
    for (int i = 0; i < b[4]; ++i) {
        bitmap = b[5+i];
        for (int j = 0; j < 32; ++j) {
            cout << (bitmap & (1 << j));
        }
    }
    cout << endl;
    cout << "******end file info************" << endl;
    
    return 0;
}

RC RecordManager::closeFile() {
    if (fileHandle->fileName == NULL){
        cout << "close file error." << endl;
        return 104;
    }

    bpm->close();
    pfm->closeFile(fileHandle->fileID);
    fileHandle->cleanFileHandle();
    cout << ">>> close file sucessfully." << endl;
    return 0;
}


#endif //RECORD_MANAGER
