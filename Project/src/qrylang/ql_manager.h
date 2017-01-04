#ifndef WF_COMMAND_DATA
#define WF_COMMAND_DATA

#include "../rcdmana/rm_filehandle.h"

class QL_Manager{
    char path[1024];
    int filenum;
    
public:
    QL_Manager()
    {
        memset(path, 0, 1024);
        filenum = 0;
    }
    void setPath(const char* path)
    {
        strcpy(this->path, path);
    }
    int Insert(string db, string attrName[], string attrData[], int num)
    {
        DB_File* chart = new DB_File();
        chart->openFile(db.c_str());
        chart->resetKeys();
        unsigned int insert_data[chart->t_position[chart->t_number]] = {};
        for(int i=0;i<num;i++)
        {
            int j=0;
            for(j=0;j<chart->t_number;i++)
            {
                if(attrName[i] == chart->t_name[j])
                    break;
            }
            if( j == chart->t_number)
                return -1;
        }
        for(int i=0;i<chart->t_number;i++)
        {
            string name = chart->t_name[i];
            bool exist = 0;
            int j = 0;
            for(j=0;j<num;j++)
            {
                if(attrName[j] == name)
                {
                    exist = 1;
                    break;
                }
            }
            if(exist == 0)
            {
                if(chart->t_null[i] == 1)
                {
                    return 0;
                }
                else
                {
                    insert_data[chart->t_position[i]] = 0;
                }
            }
            else
            {
                int pos = chart->t_position[i];
                insert_data[pos] = 1;
                if(chart->t_type[i] == 0)
                {
                    int n = atoi(attrData[j].c_str());
                    insert_data[pos + 1] = (unsigned int)(n);
                }
                else if((chart->t_type[i] & 1) == 1)
                {
                    CodeTransfer* cf = new CodeTransfer();
                    cf->encode(attrData[j].c_str(), &(insert_data[pos + 1]), chart->t_length[i]);
                }
            }
        }
        chart->addData(insert_data);
        chart->closeFile();
        return 1;
    }
    
    void Delete(string db, int page[], int position[], int num)
    {
        DB_File* chart = new DB_File();
        chart->openFile(db.c_str());
        chart->resetKeys();
        for(int i=0;i<num;i++)
        {
            chart->deleteData(page[i], position[i]);
        }
        chart->closeFile();
    }
    
    bool Update(string db, int page[], int position[], string attrName[], string attrData[], int attrNull[], int num_attr, int num_data)
    {
        DB_File* chart = new DB_File();
        chart->openFile(db.c_str());
        chart->resetKeys();
        for(int i=0;i<num_data;i++)
        {
            unsigned int now[chart->t_position[chart->t_number]] = {};
            unsigned int* before = chart->getPiece(page[i], position[i]);
            for(int k=0;k<chart->t_position[chart->t_number];k++)
            {
                now[k] = before[k];
            }
            for(int k=0;k<chart->t_number;k++)
            {
                string name = chart->t_name[k];
                bool exist = 0;
                int j = 0;
                for(j=0;j<num_attr;j++)
                {
                    if(attrName[j] == name)
                    {
                        exist = 1;
                        break;
                    }
                }
                if(exist == 0)
                {
                }
                else
                {
                    int pos = chart->t_position[k];
                    now[pos] = 1;
                    if(attrNull[j] == 1)
                    {
                        if(chart->t_null[k] == 1)
                            return false;
                        else
                        {
                            now[pos] = 0;
                        }
                    }
                    else
                    {
                        if(chart->t_type[k] == 0)
                        {
                            int n = atoi(attrData[j].c_str());
                            now[pos + 1] = (unsigned int)(n);
                        }
                        else if((chart->t_type[k] & 1) == 1)
                        {
                            CodeTransfer* cf = new CodeTransfer();
                            cf->encode(attrData[j].c_str(), &(now[pos + 1]), chart->t_length[k]);
                        }
                    }
                }
            }
            chart->insertData(now, page[i], position[i]);
        }
        chart->closeFile();
        return true;
    }
    
    string Select(string db, string attrName[], int attrNum, int page[], int position[], int dataNum)
    {
        char nn[20] = {};
        sprintf(nn,"%d",filenum++);
        string new_db(nn);
        new_db += "ins.db";
        DB_File* chart_before = new DB_File();
        chart_before->openFile(db.c_str());
        chart_before->resetKeys();
        int new_type[chart_before->t_number];
        string new_name[chart_before->t_number];
        bool new_null[chart_before->t_number];
        int new_primary = -1;
        int new_length[chart_before->t_number];
        int new_number = 0;
        int initial_pos[chart_before->t_number];
        for(int i=0;i<chart_before->t_number;i++)
        {
            bool flag = 0;
            for(int j=0;j<attrNum;j++)
            {
                if(attrName[j] == chart_before->t_name[i])
                {
                    flag = 1;
                }
            }
            if(flag == 1)
            {
                new_type[new_number] = chart_before->t_type[i];
                new_name[new_number] = chart_before->t_name[i];
                new_null[new_number] = chart_before->t_null[i];
                new_length[new_number] = chart_before->t_length[i];
                if(i == chart_before->t_primary_one)
                    new_primary = new_number;
                initial_pos[new_number] = i;
                new_number++;
            }
        }
        DB_File* chart_now = new DB_File();
        chart_now->createFile(new_db.c_str());
        chart_now->openFile(new_db.c_str());
        chart_now->initFile(new_type, new_name, new_null, new_primary, new_length, new_number);
        chart_now->resetKeys();
        for(int i=0;i<dataNum;i++)
        {
            unsigned int now[chart_before->t_position[chart_before->t_number]] = {};
            unsigned int* before = chart_before->getPiece(page[i], position[i]);
            for(int j=0;j<new_number;j++)
            {
                int c_begin = chart_before->t_position[initial_pos[j]];
                int c_end = chart_before->t_position[initial_pos[j]+1];
                int len = c_end - c_begin;
                for(int k=0;k<len;k++)
                {
                    now[k+chart_now->t_position[j]] = before[k+c_begin];
                }
            }
            chart_now->addData(now);
        }
        chart_now->testFile();
        chart_now->closeFile();
        chart_before->closeFile();
        return new_db;
    }
    
    int whereSearch(string db, string attrName[], string attrData[], int attrNull[], int attrCmp[], int attrNum, int page[], int position[])
    {
        int num_all = 0;
        DB_File* chart = new DB_File();
        chart->openFile(db.c_str());
        chart->resetKeys();
        bool flag = 0;
        int page_get[100000];
        int position_get[100000];
        int num_get;
        for(int i=0;i<chart->t_number;i++)
        {
            if(chart->t_name[i] == attrName[0])
            {
                if(attrNull[0] == 0)
                {
                    unsigned int b[1] = {0};
                    num_get = chart->searchData(b, chart->t_position[i], -1, attrCmp[0], page_get, position_get, 1);
                }
                else
                {
                    if(chart->t_type[i] == 0)
                    {
                        unsigned int b[1];
                        b[0] = (unsigned int)atoi(attrData[0].c_str());
                        num_get = chart->searchData(b, chart->t_position[i], 0, attrCmp[0], page_get, position_get, 1);
                    }
                    if((chart->t_type[i] & 1) == 1)
                    {
                        unsigned int bb[chart->t_length[i]];
                        CodeTransfer* cf = new CodeTransfer();
                        cf->encode(attrData[0].c_str(), bb, chart->t_length[i]);
                        num_get = chart->searchData(bb, chart->t_position[i], chart->t_type[i], attrCmp[0], page_get, position_get, chart->t_length[i]);
                    }
                }
                flag = 1;
            }
        }
        if(flag == 0) return -1;

        for(int i=0;i<num_get;i++)
        {
            bool flag = 1;
            for(int j=1;j<attrNum;j++)
            {
                bool init_flag = 0;
                for(int k=0;k<chart->t_number;k++)
                {
                    if(chart->t_name[k] == attrName[j])
                    {
                        unsigned int* b;
                        b = chart->getPiece(page[i],position[i]);
                        if(attrNull[j] == 0)
                        {
                            unsigned int bb[1] = {0};
                            init_flag = checkPiece(b, bb, chart->t_position[k], -1, attrCmp[j], 0);
                        }
                        else
                        {
                            if(chart->t_type[k] == 0)
                            {
                                unsigned int bb[1];
                                bb[0] = (unsigned int)atoi(attrData[j].c_str());
                                init_flag = checkPiece(b, bb, chart->t_position[k], chart->t_type[k], attrCmp[j], chart->t_length[k]);
                            }
                            if((chart->t_type[k] & 1) == 1)
                            {
                                unsigned int bb[chart->t_length[k]];
                                CodeTransfer* cf = new CodeTransfer();
                                cf->encode(attrData[j].c_str(), bb, chart->t_length[k]);
                                init_flag = checkPiece(b, bb, chart->t_position[k], chart->t_type[k], attrCmp[j], chart->t_length[k]);
                            }
                        }
                    }
                }
                if(init_flag == 0)
                {
                    flag = 0;
                    break;
                }
            }
            if(flag == 1)
            {
                page[num_all] = page_get[i];
                position[num_all++] = position_get[i];
            }
        }
        chart->closeFile();
        return num_all;
    }
    
    int getAllPiece(string db, int page[], int position[])
    {
        DB_File* chart = new DB_File();
        chart->openFile(db.c_str());
        chart->resetKeys();
        int k = chart->getAllPiece(page, position);
        chart->closeFile();
        return k;
    }
    
    bool checkPiece(unsigned int buf[], unsigned int data[], int pos, int type, int method, int length)
    {
        if(type == -1)
        {
            if((buf[pos] == 0 && method == 0) || (buf[pos] == 1 && method == 1))
            {
                return 1;
            }
        }
        if(type == 0)
        {
            int _source = int (data[0]);
            int nul = buf[pos];
            if(nul == 0)
            {
                if(method != 1)
                    return 0;
                else
                {
                    return 1;
                }
            }
            int _buf = int (buf[pos+1]);
            if(method == 0 && _buf == _source)
            {
                return 1;
            }
            if(method == 1 && _buf != _source)
            {
                return 1;
            }
            if(method == 2 && _buf > _source)
            {
                return 1;
            }
            if(method == 3 && _buf < _source)
            {
                return 1;
            }
            if(method == 4 && _buf >= _source)
            {
                return 1;
            }
            if(method == 5 && _buf <= _source)
            {
                return 1;
            }
        }
        if(type == 1 || type == 3)
        {
            char _source[8000]={};
            char _buf[8000]={};
            int nul = buf[pos];
            if(nul == 0)
            {
                if(method != 1)
                    return 0;
                else
                {
                    return 1;
                }
            }
            CodeTransfer* cf = new CodeTransfer();
            cf->decode(_source, data, length);
            cf->decode(_buf,&(buf[pos + 1]), length);
            if( method == 0 && strcmp(_buf, _source) == 0)
            {
                return 1;
            }
            if( method == 1 && strcmp(_buf, _source) != 0)
            {
                return 1;
            }
            if( method == 2 && strcmp(_buf, _source) > 0)
            {
                return 1;
            }
            if( method == 3 && strcmp(_buf, _source) < 0)
            {
                return 1;
            }
            if( method == 4 && strcmp(_buf, _source) >= 0)
            {
                return 1;
            }
            if( method == 5 && strcmp(_buf, _source) <= 0)
            {
                return 1;
            }
        }
        return 0;
    }

    void deleteInsDB()
    {
        DB_File* chart = new DB_File();
        for(int i=0;i<filenum;i++)
        {
            char nn[20] = {};
            sprintf(nn,"%d",i);
            string new_db(nn);
            new_db += "ins.db";
            chart->deleteFile(new_db.c_str());
        }
        filenum = 0;
    }
};

#endif
