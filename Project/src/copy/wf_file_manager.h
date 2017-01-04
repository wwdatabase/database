#ifndef WF_FILE_MANAGER
#define WF_FILE_MANAGER

#include "bufmanager/BufPageManager.h"
#include "fileio/FileManager.h"
#include "utils/pagedef.h"
#include "transfer.h"
#include "bplus_tree.h"
#include <iostream>
#include <string>
#include <cstdio>
#include <sys/stat.h>

using namespace std;

/*
    type:
        0 int
        1 char
        3 varchar
*/

class DB_File{
private:
    FileManager* fm;
    BufPageManager* bpm;
    int fileID;
    string type[4];
public:
    DB_File* next;
    char filename[1024];
    char db_path[1024];
    int* t_type;
    string* t_name;
    int t_primary_one;
    int* t_length;
    bool* t_null;
    int t_number;
    int* t_position;
    int t_page;
    BPlusTree* trees[20];

    DB_File()
    {
        fm = new FileManager();
        bpm = new BufPageManager(fm);
        memset(db_path, 0, 1024);
        type[0] = "int";
        type[1] = "char";
        type[3] = "varchar";
        for(int i=0;i<20;i++)
            trees[i] = NULL;
        t_number = 0;
    }
    ~DB_File()
    {
        if(t_number != 0)
        {
            delete[] t_type;
            delete[] t_name;
            delete[] t_length;
            delete[] t_null;
            delete[] t_position;
            for(int i=0;i<t_number;i++)
                delete trees[i];
        }
        delete bpm;
        delete fm;
    }
    char* getFilename()
    {
        return filename;
    }
    void setDBpath(const char* path)
    {
        strcpy(this->db_path, path);
    }
    bool createFile(const char* filename)
    {
        string file = db_path;
        file.append(filename);
        return fm->createFile(file.c_str());
    }
    bool openFile(const char* filename)
    {
        strcpy(this->filename, filename);
        string file = db_path;
        file.append(filename);
        return fm->openFile(file.c_str(), fileID);
    }
    int closeFile()
    {
        bpm->close();
        for(int i=0;i<t_number;i++)
            if(trees[i] != NULL)
                trees[i]->closeFile();
        return fm->closeFile(fileID);
    }
    int deleteFile(const char* filename)
    {
        string fi = db_path;
        fi = fi + filename +"_btree/";
        fi = "rm -rf " + fi;
        system(fi.c_str());
        return remove(filename);
    }
    int deleteFile()
    {
        string file = db_path;
        file.append(filename);
        string fi = db_path;
        fi = fi + filename +"_btree/";
        fi = "rm -rf " + fi;
        system(fi.c_str());
        return remove(file.c_str());
    }
    bool initFile(int type[], string name[], bool type_not_null[], int primary_one, int type_length[], int number)
    {
        int index;
        BufType b = bpm->getPage(fileID, 0, index);
        b[0] = number;
        CodeTransfer* cf = new CodeTransfer();
        int len = 36;
        for(int i=0;i<number;i++)
        {
            b[i*len + 1] = type[i];
            b[i*len + 2] = type_length[i];
            b[i*len + 3] = type_not_null[i];
            b[i*len + 4] = (primary_one == i);
            cf->encode(name[i].c_str(), &b[i*len + 5], 128);
            trees[i] = new BPlusTree();
            string fi = db_path;
            fi = fi + filename +"_btree/";
            mkdir(fi.c_str(), 0777);
            fi = fi +  name[i];
            trees[i]->setFilename(fi.c_str());
            trees[i]->createFile();
            trees[i]->openFile();
            trees[i]->initBPlusTree();
            trees[i]->closeFile();
        }
        delete cf;
        b[len*number+1] = 0;
        bpm->markDirty(index);
    }
    bool resetKeys()
    {
        int index;
        BufType b = bpm->getPage(fileID, 0, index);
        t_number = b[0];
        t_type = new int[b[0]];
        t_length = new int[b[0]];
        t_null = new bool[b[0]];
        t_name = new string[b[0]];
        t_position = new int[b[0]+1];
        t_position[0] = 0;
        t_primary_one = -1;

        int number = b[0];
        CodeTransfer* cf = new CodeTransfer();
        int len = 36;
        for(int i=0;i<number;i++)
        {
            t_type[i] = b[i*len + 1];
            t_length[i] = b[i*len + 2];
            if(t_type[i] == 0)
                t_position[i+1] = t_position[i] + 2;
            if(t_type[i] == 1 || t_type[i] == 3)
            {
                int k = t_length[i]/4;
                if(t_length[i] % 4 != 0)
                    k++;
                t_position[i+1] = t_position[i] + k + 1;
            }
            if(t_type[i] == 2)
                t_position[i+1] = t_position[i] + 3;
            t_null[i] = (b[i*len + 3] & 1);
            if(b[i*len + 4] == 1)
                t_primary_one = i;
            char name[128]={};
            cf->decode(name, &b[i*len + 5], 128);
            t_name[i] = name;
        }
        
        for(int i=0;i<t_number;i++)
            if(trees[i] == NULL)
            {
                trees[i] = new BPlusTree();
                string fi = db_path;
                fi = fi + filename +"_btree/" + t_name[i];
                trees[i]->setFilename(fi.c_str());
                trees[i]->openFile();
            }
            else
            {
                trees[i]->openFile();
            }

        t_page = b[t_number*len+1];
        delete cf;
    }
    void showStores()
    {
        cout << "The Number of Keys : " << t_number << endl;
        if(t_primary_one > -1)
        {
            cout << "The Primary Key : " << t_name[t_primary_one] << endl;
        }
        else
        {
            cout << "No Primary Key" << endl;
        }
        cout << endl;
        for(int i=0;i<t_number;i++)
        {
            cout << "Key : " << t_name[i] << endl;
            cout << "Type : " << type[t_type[i]] << endl;
            cout << "Length : " << t_length[i] << endl;
            if(t_null[i] == 1)
                cout << "Not NULL" << endl;
            cout << endl;
        }
    }
    void testFile()
    {
        int index;
        BufType b = bpm->getPage(fileID, 0, index);
        int number = b[0];
        CodeTransfer* cf = new CodeTransfer();
        int len = 36;
        
        for(int ik=1;ik<=t_page;ik++)
        {
            BufType b_1 = bpm->getPage(fileID, ik, index);
            for(int i=1;i<b_1[0];i+=t_position[t_number])
            {
                for(int j=0;j<t_number;j++)
                {
                    cout << t_name[j];
                    if(b_1[i+t_position[j]] == 0)
                        cout << ":NULL" << endl;
                    else
                    {
                        if(t_type[j] == 0)
                            cout << ':' << int(b_1[i+t_position[j]+1]) << endl;
                        if((t_type[j] & 1)==1)
                        {
                            char out[t_length[j]+1] = {};
                            cf->decode(out, &b_1[i+t_position[j]+1], t_length[j]);
                            cout << ':' << out << endl;
                        }
                    }
                }
                cout << endl;
            }
        }
        delete cf;
    }
    void addData(unsigned int data[])
    {
        int index, ind;
        BufType b = bpm->getPage(fileID, 0, ind);
        int num = t_position[t_number];
        int page = 36*t_number+1;
        bpm->access(ind);
        int pageID = 1;
        while(1)
        {
            BufType buf = bpm->getPage(fileID, pageID, index);
            if(pageID > t_page)
            {
                buf[0] = 0;
            }
            if(buf[0] == 0 || buf[0] > 2048)
            {
                buf[0] = 1;
                bpm->markDirty(index);
            }
            if(buf[0] + num < 2048)
            {
                for(int i=0;i<num;i++)
                {
                    buf[ buf[0] + i ] = data[i];
                }
                addIndex(data, pageID, buf[0]);
                buf[0] = buf[0] + num;
                int _page = (int) b[page];
                if(_page < pageID)
                {
                    b[page] = pageID;
                    t_page = pageID;
                }
                bpm->markDirty(index);
                bpm->markDirty(ind);
                break;
            }
            pageID++;
        }
    }
    
    void addIndex(unsigned int data[], int pageNum, int posNum)
    {
        for(int i=0;i<t_number;i++)
        {
            long long ind = 1;
            if(data[t_position[i]] == 1)
            {
                if(t_type[i] == 0)
                {
                    ind = int(data[t_position[i]+1]);
                    ind = ind + 2147483647 + 3;
                }
                else
                {
                    CodeTransfer* cf = new CodeTransfer();
                    char ch[t_length[i] + 1]={};
                    cf->decode(ch, &(data[t_position[i]+1]), t_length[i]);
                    int num = strlen(ch);
                    if(num == 0)
                    {
                        ind = 0;
                    }
                    else if(num == 1)
                    {
                        ind = (long long)(ch[0])*256*256*256;
                    }
                    else if(num == 2)
                    {
                        ind = (long long)(ch[0])*256*256*256;
                        ind = ind + ch[1]*256*256;
                    }
                    else if(num == 3)
                    {
                        ind = (long long)(ch[0])*256*256*256;
                        ind = ind + ch[1]*256*256;
                        ind = ind + ch[2]*256;
                    }
                    else
                    {
                        ind = (long long)(ch[0])*256*256*256;
                        ind = ind + ch[1]*256*256;
                        ind = ind + ch[2]*256;
                        ind = ind + ch[3];
                    }
                    ind += 2;
                    delete cf;
                }
            }
            node_position pos;
            pos.pageNum = pageNum;
            pos.posNum = posNum;
            trees[i]->addData(ind, pos);
        }
    }
    
    void insertData(unsigned int data[], int pageID, int position)
    {
        int index;
        int num = t_position[t_number];
        int dataEnd = num + position;
        BufType buf = bpm->getPage(fileID, pageID, index);
        search_pos pos[t_number];
        search_index(&(buf[position]), pos, pageID, position);
        for(int i=0;i<t_number;i++)
            trees[i]->deleteKey(pos[i]);
        for(int i = position;i<dataEnd;i++)
            buf[i] = data[i-position];
        addIndex(data, pageID, position);
        bpm->markDirty(index);
    }
    
    void search_index(unsigned int data[], search_pos index_pos[], int pageNum, int posNum)
    {
        for(int i=0;i<t_number;i++)
        {
            long long ind = 1;
            if(data[t_position[i]] == 1)
            {
                if(t_type[i] == 0)
                {
                    ind = int(data[t_position[i]+1]);
                    ind = ind + 2147483647 + 3;
                }
                else
                {
                    CodeTransfer* cf = new CodeTransfer();
                    char ch[t_length[i]+1]={};
                    cf->decode(ch, &(data[t_position[i]+1]), t_length[i]);
                    int num = strlen(ch);
                    if(num == 0)
                    {
                        ind = 0;
                    }
                    else if(num == 1)
                    {
                        ind = (long long)(ch[0])*256*256*256;
                    }
                    else if(num == 2)
                    {
                        ind = (long long)(ch[0])*256*256*256;
                        ind += ch[1]*256*256;
                    }
                    else if(num == 3)
                    {
                        ind = (long long)(ch[0])*256*256*256;
                        ind += ch[1]*256*256;
                        ind += ch[2]*256;
                    }
                    else
                    {
                        ind = (long long)(ch[0])*256*256*256;
                        ind += ch[1]*256*256;
                        ind += ch[2]*256;
                        ind += ch[3];
                    }
                    ind += 2;
                    delete cf;
                }
            }
            search_pos first = trees[i]->keySearchMin(0, 1, ind);
            search_pos second = trees[i]->keySearchMax(0, 1, ind);
            search_pos aim;
            aim = trees[i]->search_one(first, second, pageNum, posNum);
            index_pos[i] = aim;
        }
    }
    
    void deleteData(int pageID, int position)
    {
        int index;
        int num = t_position[t_number];
        int dataEnd = num + position;
        BufType buf = bpm->getPage(fileID, pageID, index);
        
        if(buf[0] <= position) return;
        
        search_pos pos[t_number];
        search_index(&(buf[position]), pos, pageID, position);
        for(int i=0;i<t_number;i++)
        {
            trees[i]->deleteKey(pos[i]);
        }
        
        for(int i = position;i<dataEnd;i++)
            buf[i] = 0;
        if(dataEnd == buf[0])
            buf[0] = buf[0] - num;
        else
            if(buf[0] == (num + 1))
            {
                buf[0] = 1;
            }
            else
            {
                int k1 = buf[0] - num;
                for(int i = 0; i < num; i++)
                {
                    buf[i + position] = buf[i + k1];
                    buf[i + k1] = 0;
                }
                buf[0] = k1;
                search_index(&(buf[position]), pos, pageID, k1);
                for(int i=0;i<t_number;i++)
                {
                    page node = trees[i]->getNode(pos[i].pos.pageNum, pos[i].pos.posNum);
                    node.child[pos[i].index].posNum = position;
                    trees[i]->writeBackNode(node);
                }
            }
        bpm->markDirty(index);
    }
    /*
        method
        0 =
        1 !=
        2 >
        3 <
        4 >=
        5 <=
    */
    int searchData(unsigned int data[], int pos, int type, int method, int pages[], int positions[], int length)
    {
        int po = 0;
        for(int i=0;i<t_number;i++)
            if(pos == t_position[i])
            {
                po = i;
                break;
            }

        page check = trees[po]->getNode(0, 1);
        if(check.data[0] == 0)
            return 0;

        long long ind;
        
        if(type == -1)
        {
            ind = 1;
        }
        if(type == 0)
        {
            ind = (long long)(int(data[0]));
            ind = ind + 2147483647 + 3;
        }
        if(type == 1 || type ==3)
        {
            CodeTransfer* cf = new CodeTransfer();
            char ch[t_length[po]+1]={};
            cf->decode(ch, &(data[0]), t_length[po]);
            delete cf;
            int num = strlen(ch);
            if(num == 0)
            {
                ind = 0;
            }
            else if(num == 1)
            {
                ind = (long long)(ch[0])*256*256*256;
            }
            else if(num == 2)
            {
                ind = (long long)(ch[0])*256*256*256;
                ind += ch[1]*256*256;
            }
            else if(num == 3)
            {
                ind = (long long)(ch[0])*256*256*256;
                ind += ch[1]*256*256;
                ind += ch[2]*256;
            }
            else
            {
                ind = (long long)(ch[0])*256*256*256;
                ind += ch[1]*256*256;
                ind += ch[2]*256;
                ind += ch[3];
            }
            ind += 2;
        }
        
        search_pos first = trees[po]->keySearchMin(0, 1, ind);
        search_pos second = trees[po]->keySearchMax(0, 1, ind);
        
        int l = 0;
        
        if(type == 0)
        {
            if(method == 0 || method == 4 || method == 5)
            {
                page now = trees[po]->getNode(first.pos.pageNum, first.pos.posNum);
                int ini = first.index;
                while(1)
                {
                    for(;ini<2*DU-1;ini++)
                    {
                        if(now.data[ini] == 0)
                            break;
                        if((now.self_pos.pageNum == second.pos.pageNum) && (now.self_pos.posNum == second.pos.posNum) && (ini == second.index))
                            break;
                        pages[l] = now.child[ini].pageNum;
                        positions[l] = now.child[ini].posNum;
                        l++;
                    }
                    if((now.sibling.pageNum == 0 && now.sibling.posNum == 0) || (now.self_pos.pageNum == second.pos.pageNum && now.self_pos.posNum == second.pos.posNum))
                        break;
                    else
                        now = trees[po]->getNode(now.sibling.pageNum, now.sibling.posNum);
                    ini = 0;
                }
            }
            if(method == 1 || method == 3 || method == 5)
            {
                int labb;
                if(method == 1)
                    labb = 0;
                else
                    labb = 1;
                search_pos zero = trees[po]->keySearchMax(0, 1, labb);
                if(zero.pos.pageNum == 0 && zero.pos.posNum == 0)
                {
                }
                else
                {
                    page now = trees[po]->getNode(zero.pos.pageNum, zero.pos.posNum);
                    int ini = zero.index;
                    while(1)
                    {
                        for(;ini<2*DU-1;ini++)
                        {
                            if(now.data[ini] == 0)
                                break;
                            if((now.self_pos.pageNum == first.pos.pageNum) && (now.self_pos.posNum == first.pos.posNum) && (ini == first.index))
                                break;
                            pages[l] = now.child[ini].pageNum;
                            positions[l] = now.child[ini].posNum;
                            l++;
                        }
                        if((now.sibling.pageNum == 0 && now.sibling.posNum == 0) || (now.self_pos.pageNum == first.pos.pageNum && now.self_pos.posNum == first.pos.posNum))
                            break;
                        else
                            now = trees[po]->getNode(now.sibling.pageNum, now.sibling.posNum);
                        ini = 0;
                    }
                }
            }
            if(method == 1 || method == 2 || method == 4)
            {
                if(!(second.pos.pageNum == 0 && second.pos.posNum == 0))
                {
                    page now = trees[po]->getNode(second.pos.pageNum, second.pos.posNum);
                    int ini = second.index;
                    while(1)
                    {
                        for(;ini<2*DU-1;ini++)
                        {
                            if(now.data[ini] == 0)
                                break;
                            pages[l] = now.child[ini].pageNum;
                            positions[l] = now.child[ini].posNum;
                            l++;
                        }
                        if((now.sibling.pageNum == 0 && now.sibling.posNum == 0))
                            break;
                        else
                            now = trees[po]->getNode(now.sibling.pageNum, now.sibling.posNum);
                        ini = 0;
                    }
                }
            }
            return l;
        }
        else if(type == 1 || type == 3)
        {
            CodeTransfer* cf = new CodeTransfer();
            if(method == 0)
            {
                page now = trees[po]->getNode(first.pos.pageNum, first.pos.posNum);
                int ini = first.index;
                while(1)
                {
                    for(;ini<2*DU-1;ini++)
                    {
                        if(now.data[ini] == 0)
                            break;
                        if((now.self_pos.pageNum == second.pos.pageNum) && (now.self_pos.posNum == second.pos.posNum) && (ini == second.index))
                            break;
                        unsigned int* bu;
                        bu = getPiece(now.child[ini].pageNum, now.child[ini].posNum);
                        int la;
                        for(la = pos + 1;la < t_position[po + 1];la++)
                        {
                            if(data[la - pos - 1] != bu[la])
                                break;
                        }
                        if(la < t_position[po+1])
                            continue;
                        pages[l] = now.child[ini].pageNum;
                        positions[l] = now.child[ini].posNum;
                        l++;
                    }
                    if((now.sibling.pageNum == 0 && now.sibling.posNum == 0) || (now.self_pos.pageNum == second.pos.pageNum && now.self_pos.posNum == second.pos.posNum))
                        break;
                    else
                        now = trees[po]->getNode(now.sibling.pageNum, now.sibling.posNum);
                    ini = 0;
                }
            }
            if(method == 1)
            {
                page zero = trees[po]->getNode(0, 1);
                while(zero.isLeaf == 0)
                {
                    zero = trees[po]->getNode(zero.child[0].pageNum, zero.child[0].posNum);
                }
                page now = zero;
                int ini = 0;
                while(1)
                {
                    for(;ini<2*DU-1;ini++)
                    {
                        if(now.data[ini] == 0)
                            break;
                        if((now.self_pos.pageNum == first.pos.pageNum) && (now.self_pos.posNum == first.pos.posNum) && (ini == first.index))
                            break;
                        pages[l] = now.child[ini].pageNum;
                        positions[l] = now.child[ini].posNum;
                        l++;
                    }
                    if((now.sibling.pageNum == 0 && now.sibling.posNum == 0) || (now.self_pos.pageNum == first.pos.pageNum && now.self_pos.posNum == first.pos.posNum))
                        break;
                    else
                        now = trees[po]->getNode(now.sibling.pageNum, now.sibling.posNum);
                    ini = 0;
                }
                now = trees[po]->getNode(first.pos.pageNum, first.pos.posNum);
                ini = first.index;
                while(1)
                {
                    for(;ini<2*DU-1;ini++)
                    {
                        if(now.data[ini] == 0)
                            break;
                        if((now.self_pos.pageNum == second.pos.pageNum) && (now.self_pos.posNum == second.pos.posNum) && (ini == second.index))
                            break;
                        unsigned int* bu;
                        bu = getPiece(now.child[ini].pageNum, now.child[ini].posNum);
                        int la;
                        for(la = pos + 1;la < t_position[po + 1];la++)
                        {
                            if(data[la - pos - 1] != bu[la])
                                break;
                        }
                        if(la >= t_position[po+1])
                            continue;
                        pages[l] = now.child[ini].pageNum;
                        positions[l] = now.child[ini].posNum;
                        l++;
                    }
                    if((now.sibling.pageNum == 0 && now.sibling.posNum == 0) || (now.self_pos.pageNum == second.pos.pageNum && now.self_pos.posNum == second.pos.posNum))
                        break;
                    else
                        now = trees[po]->getNode(now.sibling.pageNum, now.sibling.posNum);
                    ini = 0;
                }
                if(!(second.pos.pageNum == 0 && second.pos.posNum == 0))
                {
                    now = trees[po]->getNode(second.pos.pageNum, second.pos.posNum);
                    ini = second.index;
                    while(1)
                    {
                        for(;ini<2*DU-1;ini++)
                        {
                            if(now.data[ini] == 0)
                                break;
                            pages[l] = now.child[ini].pageNum;
                            positions[l] = now.child[ini].posNum;
                            l++;
                        }
                        if((now.sibling.pageNum == 0 && now.sibling.posNum == 0))
                            break;
                        else
                            now = trees[po]->getNode(now.sibling.pageNum, now.sibling.posNum);
                        ini = 0;
                    }
                }
            }
            if(method == 2)
            {
                if(!(second.pos.pageNum == 0 && second.pos.posNum == 0))
                {
                    page now = trees[po]->getNode(second.pos.pageNum, second.pos.posNum);
                    int ini = second.index;
                    while(1)
                    {
                        for(;ini<2*DU-1;ini++)
                        {
                            if(now.data[ini] == 0)
                                break;
                            pages[l] = now.child[ini].pageNum;
                            positions[l] = now.child[ini].posNum;
                            l++;
                        }
                        if((now.sibling.pageNum == 0 && now.sibling.posNum == 0))
                            break;
                        else
                            now = trees[po]->getNode(now.sibling.pageNum, now.sibling.posNum);
                        ini = 0;
                    }
                }
                page now = trees[po]->getNode(first.pos.pageNum, first.pos.posNum);
                int ini = first.index;
                while(1)
                {
                    for(;ini<2*DU-1;ini++)
                    {
                        if(now.data[ini] == 0)
                            break;
                        if((now.self_pos.pageNum == second.pos.pageNum) && (now.self_pos.posNum == second.pos.posNum) && (ini == second.index))
                            break;
                        unsigned int* bu;
                        bu = getPiece(now.child[ini].pageNum, now.child[ini].posNum);
                        char _aim[8001] = {};
                        char _sou[8001] = {};
                        cf->decode(_aim, data, length);
                        cf->decode(_sou, &bu[pos+1], length);
                        if(strcmp(_sou, _aim) >0)
                        {
                            pages[l] = now.child[ini].pageNum;
                            positions[l] = now.child[ini].posNum;
                            l++;
                        }
                    }
                    if((now.sibling.pageNum == 0 && now.sibling.posNum == 0) || (now.self_pos.pageNum == second.pos.pageNum && now.self_pos.posNum == second.pos.posNum))
                        break;
                    else
                        now = trees[po]->getNode(now.sibling.pageNum, now.sibling.posNum);
                    ini = 0;
                }
            }
            if(method == 3)
            {
                search_pos z_po = trees[po]->keySearchMax(0, 1, 1);
                if(!(z_po.pos.pageNum == 0 && z_po.pos.posNum ==0 ))
                {
                    page now = trees[po]->getNode(z_po.pos.pageNum, z_po.pos.posNum);
                    int ini = z_po.index;
                    while(1)
                    {
                        for(;ini<2*DU-1;ini++)
                        {
                            if(now.data[ini] == 0) break;
                            if(now.self_pos.pageNum == first.pos.pageNum && now.self_pos.posNum == first.pos.posNum && ini == first.index)
                                break;
                            pages[l] = now.child[ini].pageNum;
                            positions[l] = now.child[ini].posNum;
                            l++;
                        }
                        if((now.sibling.pageNum == 0 && now.sibling.posNum == 0) || (now.self_pos.pageNum == first.pos.pageNum && now.self_pos.posNum == first.pos.posNum))
                            break;
                        else
                            now = trees[po]->getNode(now.sibling.pageNum, now.sibling.posNum);
                        ini = 0;
                    }
                }

                page now = trees[po]->getNode(first.pos.pageNum, first.pos.posNum);
                int ini = first.index;
                while(1)
                {
                    for(;ini<2*DU-1;ini++)
                    {
                        if(now.data[ini] == 0)
                            break;
                        if((now.self_pos.pageNum == second.pos.pageNum) && (now.self_pos.posNum == second.pos.posNum) && (ini == second.index))
                            break;
                        unsigned int* bu;
                        bu = getPiece(now.child[ini].pageNum, now.child[ini].posNum);
                        char _aim[8001] = {};
                        char _sou[8001] = {};
                        cf->decode(_aim, data, length);
                        cf->decode(_sou, &bu[pos+1], length);
                        if(strcmp(_sou, _aim) < 0)
                        {
                            pages[l] = now.child[ini].pageNum;
                            positions[l] = now.child[ini].posNum;
                            l++;
                        }
                    }
                    if((now.sibling.pageNum == 0 && now.sibling.posNum == 0) || (now.self_pos.pageNum == second.pos.pageNum && now.self_pos.posNum == second.pos.posNum))
                        break;
                    else
                        now = trees[po]->getNode(now.sibling.pageNum, now.sibling.posNum);
                    ini = 0;
                }
                
            }
            
            if(method == 4)
            {
                if(!(second.pos.pageNum == 0 && second.pos.posNum == 0))
                {
                    page now = trees[po]->getNode(second.pos.pageNum, second.pos.posNum);
                    int ini = second.index;
                    while(1)
                    {
                        for(;ini<2*DU-1;ini++)
                        {
                            if(now.data[ini] == 0)
                                break;
                            pages[l] = now.child[ini].pageNum;
                            positions[l] = now.child[ini].posNum;
                            l++;
                        }
                        if((now.sibling.pageNum == 0 && now.sibling.posNum == 0))
                            break;
                        else
                            now = trees[po]->getNode(now.sibling.pageNum, now.sibling.posNum);
                        ini = 0;
                    }
                }
                page now = trees[po]->getNode(first.pos.pageNum, first.pos.posNum);
                int ini = first.index;
                while(1)
                {
                    for(;ini<2*DU-1;ini++)
                    {
                        if(now.data[ini] == 0)
                            break;
                        if((now.self_pos.pageNum == second.pos.pageNum) && (now.self_pos.posNum == second.pos.posNum) && (ini == second.index))
                            break;
                        unsigned int* bu;
                        bu = getPiece(now.child[ini].pageNum, now.child[ini].posNum);
                        char _aim[8001] = {};
                        char _sou[8001] = {};
                        cf->decode(_aim, data, length);
                        cf->decode(_sou, &bu[pos+1], length);
                        if(strcmp(_sou, _aim) >= 0)
                        {
                            pages[l] = now.child[ini].pageNum;
                            positions[l] = now.child[ini].posNum;
                            l++;
                        }
                    }
                    if((now.sibling.pageNum == 0 && now.sibling.posNum == 0) || (now.self_pos.pageNum == second.pos.pageNum && now.self_pos.posNum == second.pos.posNum))
                        break;
                    else
                        now = trees[po]->getNode(now.sibling.pageNum, now.sibling.posNum);
                    ini = 0;
                }
            }
            
            if(method == 5)
            {
                search_pos z_po = trees[po]->keySearchMax(0, 1, 1);
                if(!(z_po.pos.pageNum == 0 && z_po.pos.posNum ==0 ))
                {
                    page now = trees[po]->getNode(z_po.pos.pageNum, z_po.pos.posNum);
                    int ini = z_po.index;
                    while(1)
                    {
                        for(;ini<2*DU-1;ini++)
                        {
                            if(now.data[ini] == 0) break;
                            if(now.self_pos.pageNum == first.pos.pageNum && now.self_pos.posNum == first.pos.posNum && ini == first.index)
                                break;
                            pages[l] = now.child[ini].pageNum;
                            positions[l] = now.child[ini].posNum;
                            l++;
                        }
                        if((now.sibling.pageNum == 0 && now.sibling.posNum == 0) || (now.self_pos.pageNum == first.pos.pageNum && now.self_pos.posNum == first.pos.posNum))
                            break;
                        else
                            now = trees[po]->getNode(now.sibling.pageNum, now.sibling.posNum);
                        ini = 0;
                    }
                }
                
                page now = trees[po]->getNode(first.pos.pageNum, first.pos.posNum);
                int ini = first.index;
                while(1)
                {
                    for(;ini<2*DU-1;ini++)
                    {
                        if(now.data[ini] == 0)
                            break;
                        if((now.self_pos.pageNum == second.pos.pageNum) && (now.self_pos.posNum == second.pos.posNum) && (ini == second.index))
                            break;
                        unsigned int* bu;
                        bu = getPiece(now.child[ini].pageNum, now.child[ini].posNum);
                        char _aim[8001] = {};
                        char _sou[8001] = {};
                        cf->decode(_aim, data, length);
                        cf->decode(_sou, &bu[pos+1], length);
                        if(strcmp(_sou, _aim) <= 0)
                        {
                            pages[l] = now.child[ini].pageNum;
                            positions[l] = now.child[ini].posNum;
                            l++;
                        }
                    }
                    if((now.sibling.pageNum == 0 && now.sibling.posNum == 0) || (now.self_pos.pageNum == second.pos.pageNum && now.self_pos.posNum == second.pos.posNum))
                        break;
                    else
                        now = trees[po]->getNode(now.sibling.pageNum, now.sibling.posNum);
                    ini = 0;
                }
            }
            return l;
        }
        else if(type == -1)
        {
            if(method == 0)
            {
                page now = trees[po]->getNode(first.pos.pageNum, first.pos.posNum);
                int ini = first.index;
                while(1)
                {
                    for(;ini<2*DU-1;ini++)
                    {
                        if(now.data[ini] == 0)
                            break;
                        if((now.self_pos.pageNum == second.pos.pageNum) && (now.self_pos.posNum == second.pos.posNum) && (ini == second.index))
                            break;
                        pages[l] = now.child[ini].pageNum;
                        positions[l] = now.child[ini].posNum;
                        l++;
                    }
                    if((now.sibling.pageNum == 0 && now.sibling.posNum == 0) || (now.self_pos.pageNum == second.pos.pageNum && now.self_pos.posNum == second.pos.posNum))
                        break;
                    else
                        now = trees[po]->getNode(now.sibling.pageNum, now.sibling.posNum);
                    ini = 0;
                }
            }
            
            if(method == 1)
            {
                if(!(second.pos.pageNum == 0 && second.pos.posNum == 0))
                {
                    page now = trees[po]->getNode(second.pos.pageNum, second.pos.posNum);
                    int ini = second.index;
                    while(1)
                    {
                        for(;ini<2*DU-1;ini++)
                        {
                            if(now.data[ini] == 0)
                                break;
                            pages[l] = now.child[ini].pageNum;
                            positions[l] = now.child[ini].posNum;
                            l++;
                        }
                        if((now.sibling.pageNum == 0 && now.sibling.posNum == 0))
                            break;
                        else
                            now = trees[po]->getNode(now.sibling.pageNum, now.sibling.posNum);
                        ini = 0;
                    }
                }
            }
            
            return l;
        }
        /*int index, ind;
        BufType b = bpm->getPage(fileID, 0, ind);
        int step = t_position[t_number];
        int max_page = t_page;
        int l=0;
        CodeTransfer* cf = new CodeTransfer();
        for(int i=1;i<=max_page;i++)
        {
            BufType buf = bpm->getPage(fileID, i, index);
            for(int j=1;j<buf[0];j+=step)
            {
                if(type == -1)//null
                {
                    if((buf[j+pos] == 0 && method == 0) || (buf[j+pos] == 1 && method == 1))
                    {
                        pages[l] = i;
                        positions[l++] = j;
                    }
                }
                if(type == 0)//int
                {
                    int _source = int (data[0]);
                    int nul = buf[j+pos];
                    if(nul == 0)
                    {
                        if(method != 1)
                            continue;
                        else
                        {
                            pages[l] = i;
                            positions[l++] = j;
                        }
                    }
                    int _buf = int (buf[j+pos+1]);
                    if(method == 0 && _buf == _source)
                    {
                        pages[l] = i;
                        positions[l++] = j;
                    }
                    if(method == 1 && _buf != _source)
                    {
                        pages[l] = i;
                        positions[l++] = j;
                    }
                    if(method == 2 && _buf > _source)
                    {
                        pages[l] = i;
                        positions[l++] = j;
                    }
                    if(method == 3 && _buf < _source)
                    {
                        pages[l] = i;
                        positions[l++] = j;
                    }
                    if(method == 4 && _buf >= _source)
                    {
                        pages[l] = i;
                        positions[l++] = j;
                    }
                    if(method == 5 && _buf <= _source)
                    {
                        pages[l] = i;
                        positions[l++] = j;
                    }
                }
                if(type == 1 || type == 3)//string
                {
                    char _source[8000]={};
                    char _buf[8000]={};
                    int nul = buf[j+pos];
                    if(nul == 0)
                    {
                        if(method != 1)
                            continue;
                        else
                        {
                            pages[l] = i;
                            positions[l++] = j;
                        }
                    }
                    cf->decode(_source, data, length);
                    cf->decode(_buf,&(buf[j + pos + 1]), length);
                    if( method == 0 && strcmp(_buf, _source) == 0)
                    {
                        pages[l] = i;
                        positions[l++] = j;
                    }
                    if( method == 1 && strcmp(_buf, _source) != 0)
                    {
                        pages[l] = i;
                        positions[l++] = j;
                    }
                    if( method == 2 && strcmp(_buf, _source) > 0)
                    {
                        pages[l] = i;
                        positions[l++] = j;
                    }
                    if( method == 3 && strcmp(_buf, _source) < 0)
                    {
                        pages[l] = i;
                        positions[l++] = j;
                    }
                    if( method == 4 && strcmp(_buf, _source) >= 0)
                    {
                        pages[l] = i;
                        positions[l++] = j;
                    }
                    if( method == 5 && strcmp(_buf, _source) <= 0)
                    {
                        pages[l] = i;
                        positions[l++] = j;
                    }
                }
            }
        }
        return l;*/
    }
    
    int getAllPiece(int page[], int position[])
    {
        int index, ind;
        BufType b = bpm->getPage(fileID, 0, ind);
        int step = t_position[t_number];
        int max_page = t_page;
        int l = 0;
        for(int i=1;i<=max_page;i++)
        {
            BufType buf = bpm->getPage(fileID, i, index);
            for(int j=1;j<buf[0];j+=step)
            {
                page[l] = i;
                position[l++] = j;
            }
        }
        return l;
    }
    
    BufType getPiece(int pageID, int position)
    {
        int index;
        BufType b = bpm->getPage(fileID, pageID, index);
        return &(b[position]);
    }
};

#endif
