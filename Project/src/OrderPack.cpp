#include <cstring>
#include <iostream>
#include <vector>
#include "qrylang/ql_manager.h"
#include "OrderPack.h"
#include "SchemaEntry.h"
#include "Condition.h"
#include "CondEntry.h"
#include "Attr.h"
#include "Expr.h"
#include "sysmana/SysMana.h"
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <algorithm>
#include <fstream>
#include <sys/timeb.h>

using namespace std;

static int enterDatabaseFlag = 0;
extern DBT* filemanager;
extern bool isTuo;

inline int ld(std::string s, std::string t)
{
    int n = s.size();
    int m = t.size();
    if (m == 0) return n;
    if (n == 0) return m;
    std::vector< std::vector<int> > matrix(n + 1);
    for (int i = 0; i <= n; i ++) matrix[i].resize(m + 1);
    for (int i = 1; i <= n; i++) matrix[i][0] = i;
    for (int i = 1; i <= m; i++) matrix[0][i] = i;
    for (int i = 1; i <= n; i++)
    {
        char c = s[i - 1];
        for (int j = 1; j <= m; j++)
        {
            char d = t[j - 1];
            int cost = 1;
            if (c == d) cost = 0;
            int p = matrix[i - 1][j] + 1;
            int q = matrix[i][j - 1] + 1;
            int r = matrix[i - 1][j - 1] + cost;
            if (q < p) p = q;
            if (r < p) p = r;
            matrix[i][j] = p;
        }
    }
    return matrix[n][m];
}

inline bool like(std::string s, std::string t)
{
    locale loc;
    for(int i=0;i<s.size();i++)
        s[i] = toupper(s[i], loc);
    for(int i=0;i<t.size();i++)
        t[i] = toupper(t[i], loc);
    if (s == t)
        return 1;
    int l = s.size();
    int j = t.size();
    if ((l > 2 && (double)j / l < 2 || (l > 3)) && t.find(s) != std::string::npos) return 1;
    if (l < j) l = j;
    int k = ld(s, t);
    if ((double)k / l < 0.25) return 1;
    if (l > 1 && k == 1) return 1;
    return 0;
}

Condition trans(Condition condition)
{
    Condition cond;

    for(int i=0;i<condition.operands.size();i++)
    {
        if(condition.operands[i].op != CondEntry::IN)
        {
            cond.operands.push_back(condition.operands[i]);
        }
        else
        {
            for(int j=0;j<condition.operands[i].values.size();j++)
            {
                if(condition.operands[i].values[j].type == 0)
                {
                    Expr exp_right(Expr::INTEGER);
                    exp_right.integer = condition.operands[i].values[j].integer;
                    cond.operands.push_back(CondEntry(CondEntry::EQUAL, condition.operands[i].left, exp_right));
                }
                if(condition.operands[i].values[j].type == 1)
                {
                    Expr exp_right(Expr::LITERAL);
                    exp_right.literal = condition.operands[i].values[j].literal;
                    cond.operands.push_back(CondEntry(CondEntry::EQUAL, condition.operands[i].left, exp_right));
                }
                if(j == 0)
                    continue;
                cond.opcodes.push_back(Condition::OR);
            }
        }
        if(i > 0)
        {
            cond.opcodes.push_back(condition.opcodes[i-1]);
        }
    }
    return cond;
}

void OrderPack::process()
{
    switch (type)
    {
    case CREATE_DB:
    {
        filemanager->create((char*)dbname.c_str());
        break;
    }
    case DROP_DB:
    {
        filemanager->drop((char*)dbname.c_str());
        break;
    }
    case USE:
    {
        filemanager->use((char*)dbname.c_str());
        break;
    }
    case CREATE_TB:
    {
        int num = schema.size();
        int types[num];
        string names[num];
        bool not_null[num];
        int primary_one = -1;
        int length[num];
        
        for(int i=0;i<schema.size();i++)
        {
            if(schema.entries[i].type == "int")
                types[i] = 0;
            else if(schema.entries[i].type == "char")
                types[i] = 1;
            else
                types[i] = 3;
            if (schema.entries[i].field.length() >= 128)
            {
                cout << "[ERROR] Name of column is too long" << endl;
                return;
            }
            names[i] = schema.entries[i].field;
            not_null[i] = schema.entries[i].notNull;
            if(schema.entries[i].isPrimary == 1)
            {
                if(primary_one != -1)
                {
                    cout << "[ERROR] The primary key is at most one" << endl;
                    return;
                }
                primary_one = i;
            }
            length[i] = schema.entries[i].length;
        }
        
        if (isTuo)
        {
            for(int i=0;i<schema.size();i++)
            {
                string table_path = filemanager->npath;
                table_path = table_path + "/" + tbname + "_foreign/";
                mkdir(table_path.c_str(), 0777);
                if(schema.entries[i].isForeign == 1)
                {
                    string filename (filemanager->npath);
                    filename = filename + '/' + schema.entries[i].foreignAttr.tbname;
                    DB_File * dbb = new DB_File();
                    if(dbb->openFile(filename.c_str())==0)
                    {
                        cout << "[ERROR] NO such foreign table!" << endl;
                        delete dbb;
                        return;
                    }
                    dbb->resetKeys();
                    int kk = 0;
                    for(kk = 0;kk < dbb->t_number;kk++)
                    {
                        if(dbb->t_name[kk] == schema.entries[i].foreignAttr.attrname)
                            break;
                    }
                    if(kk == dbb->t_number)
                    {
                        cout << "[ERROR] NO such foreign key!" << endl;
                        delete dbb;
                        return;
                    }
                    filename = table_path + schema.entries[i].field;
                    ofstream fout(filename.c_str());
                    fout << schema.entries[i].foreignAttr.tbname << endl;
                    fout << schema.entries[i].foreignAttr.attrname << endl;
                    fout.close();
                    delete dbb;
                }
            }

            if(schema.constrain.operands.size()!=0)
            {
                string table_path = filemanager->npath;
                table_path = table_path + "/" + tbname + "_res/";
                mkdir(table_path.c_str(), 0777);
                for(int i=0;i<schema.constrain.operands.size();i++)
                {
                    if(schema.constrain.operands[i].op != CondEntry::IN)
                    {
                        cout << "[ERROR] Can't check this key!" << endl;
                        return;
                    }
                    string res = table_path + schema.constrain.operands[i].left.attr.attrname;
                    ofstream fout(res.c_str());
                    for(int j=0;j<schema.constrain.operands[i].values.size();j++)
                    {
                        if(schema.constrain.operands[i].values[j].type == 0)
                        {
                            fout << schema.constrain.operands[i].values[j].integer << endl;
                        }
                        else if(schema.constrain.operands[i].values[j].type == 1)
                        {
                            fout << schema.constrain.operands[i].values[j].literal << endl;
                        }
                        else
                        {
                            cout << "[ERROR] Check error!" << endl;
                        }
                    }
                    fout.close();
                }
            }
        }
        
        filemanager->create_file((char *)tbname.c_str(), types, names, not_null, primary_one, length, num);
        break;
    }
    case DROP_TB:
    {
        filemanager->drop_file((char *)tbname.c_str());
        string fi = filemanager->npath;
        fi = fi + "/" + tbname +"_res/";
        fi = "rm -rf " + fi;
        system(fi.c_str());
        filemanager->npath;
        fi = fi + "/" + tbname +"_foreign/";
        fi = "rm -rf " + fi;
        system(fi.c_str());
        break;
    }
    case SHOWDBS:
    {
        filemanager->show_dbs();
        break;
    }
    case SHOWTBS:
    {
        filemanager->show();
        break;
    }
    case DESC:
    {
        filemanager->show_file((char*)tbname.c_str());
        break;
    }
    case INSERT:
    {
        DB_File* db = new DB_File();
        
        string table_path(filemanager->npath);
        table_path = table_path + '/' + tbname;
        
        if(db->openFile(table_path.c_str()) == 0)
        {
            cout << "[ERROR] There is no such table!" << endl;
            delete db;
            return;
        }
        
        db->resetKeys();
        
        for(int ik=0;ik<valuesList.size();ik++)
        {
            values = valuesList[ik];

            if(isTuo)
            {
                for(int i=0;i<db->t_number;i++)
                {
                    string filename = table_path + "_foreign/" + db->t_name[i];
                    ifstream fin(filename.c_str());
                    if(!fin)
                        continue;
                    string tablename;
                    string tablekey;
                    fin >> tablename;
                    fin >> tablekey;
                    DB_File* db_f = new DB_File();
                    string f_table_path(filemanager->npath);
                    f_table_path = f_table_path + '/' + tablename;
                    if(db_f->openFile(f_table_path.c_str())==0)
                    {
                        cout << "[ERROR] Foreign table disappeared!" << endl;
                        delete db;
                        delete db_f;
                        return;
                    }
                    db_f->resetKeys();
                    if(values[i].type == 2)
                        continue;
                    QL_Manager* dd = new QL_Manager();
                    int pages[10000];
                    int positions[10000];
                    int len;
                    string attrName[1] = tablekey;
                    string attrData[1] = {};
                    int null[1] = {1};
                    int cpm[1] = {0};

                    if(values[i].type == 0)
                    {
                        char numb[100] = {};
                        sprintf(numb, "%d", values[i].integer);
                        attrData[0] = numb;
                    }
                    else
                    {
                        attrData[0] = values[i].literal;
                    }

                    len = dd->whereSearch(f_table_path, attrName, attrData, null, cpm, 1, pages, positions);

                    if(len == 0)
                    {
                        cout << "[EEROR] Foreign key's data doesn't exist!" << endl;
                        delete db_f;
                        delete dd;
                        delete db;
                        return;
                    }
                }
            }

            if(isTuo)
            {
                for(int i=0;i<db->t_number;i++)
                {
                    string filename = table_path + "_res/" + db->t_name[i];
                    ifstream fin(filename.c_str());
                    if(!fin)
                        continue;
                    if(db->t_type[i] == 0)
                    {
                        bool flag = 0;
                        int k;
                        if(values[i].type != 0)
                            continue;
                        while(fin >> k)
                        {
                            if(k == values[i].integer)
                            {
                                flag = 1;
                                break;
                            }
                        }
                        if(flag == 0)
                        {
                            cout << "[ERROR] Key check failed" << endl;
                            delete db;
                            return;
                        }
                    }
                    else
                    {
                        bool flag = 0;
                        string k;
                        if(values[i].type != 1)
                            continue;
                        while(fin >> k)
                        {
                            if(k == values[i].literal)
                            {
                                flag = 1;
                                break;
                            }
                        }
                        if(flag == 0)
                        {
                            cout << "[ERROR] Key check failed" << endl;
                            delete db;
                            return;
                        }
                    }
                }
            }

            if(values.size() != db->t_number)
            {
                cout << "[ERROR] The Values' number isn't right!" << endl;
            }

            unsigned int insert_data[10000] = {};

            for(int i=0;i<db->t_number;i++)
            {
                if(values[i].type == 0)
                    if(db->t_type[i] != 0)
                    {
                        cout << "[ERROR] Value : " << (i+1) << " Type Error!" << endl;
                        delete db;
                        return;
                    }
                if(values[i].type == 1)
                    if(db->t_type[i] != 1 && db->t_type[i]!=3)
                    {
                        cout << "[ERROR] Value : " << (i+1) << " Type Error!" << endl;
                        delete db;
                        return;
                    }
                if(db->t_primary_one == i)
                {
                    if(values[i].type == 2)
                    {
                        cout << "[ERROR] Primary Key can't be NULL!" << endl;
                        delete db;
                        return;
                    }
                    if(db->t_type[i] == 0)
                    {
                        unsigned int b[2];
                        b[0] = values[i].integer;
                        int pages[1000];
                        int positions[1000];
                        int len = 0;
                        len = db->searchData(b, db->t_position[i], 0, 0, pages, positions, 1);
                        if(len > 0)
                        {
                            cout << "[ERROR] Primary Key has existed!" << endl;
                            delete db;
                            return;
                        }
                    }
                    if(db->t_type[i] == 1 || db->t_type[i] == 3)
                    {
                        string aim = values[i].literal;
                        if(aim.size() > db->t_length[i])
                        {
                            cout << "[ERROR] String is longer than init!" << endl;
                            delete db;
                            return;
                        }
                        CodeTransfer* cf = new CodeTransfer();
                        unsigned int dat[db->t_position[i+1] - db->t_position[i]];
                        aim = aim + '\0';
                        cf->encode(aim.c_str(), dat, db->t_length[i]);
                        int pages[1000];
                        int positions[1000];
                        int len = 0;
                        len = db->searchData(dat, db->t_position[i], 1, 0, pages, positions, db->t_length[i]);
                        if(len > 0)
                        {
                            cout << "[ERROR] Primary Key has existed!" << endl;
                            delete db;
                            return;
                        }
                    }
                }
                if(values[i].type == 0)
                {
                    insert_data[db->t_position[i]] = 1;
                    insert_data[db->t_position[i]+1] = (unsigned int)(values[i].integer);
                }
                if(values[i].type == 2)
                {
                    insert_data[db->t_position[i]] = 0;
                }
                if(values[i].type == 1)
                {
                    if((db->t_type[i] == 1) && (values[i].literal.size() != db->t_length[i]))
                    {
                        cout << "[ERROR] Type char should own the same length!" << endl;
                        delete db;
                        return;
                    }
                    CodeTransfer* cf = new CodeTransfer();
                    insert_data[db->t_position[i]] = 1;
                    cf->encode(values[i].literal.c_str(), &insert_data[db->t_position[i]+1], db->t_length[i]);
                    delete cf;
                }
            }

            db->addData(insert_data);
        }
        
        db->closeFile();
        delete db;
        break;
    }
    case DELETE:
    {
        DB_File* db = new DB_File();
        
        QL_Manager* dd = new QL_Manager();
        
        string table_path(filemanager->npath);
        table_path = table_path + '/' + tbname;
        
        if(db->openFile(table_path.c_str()) == 0)
        {
            cout << "[ERROR] There is no such table!" << endl;
            delete db;
            delete dd;
            return;
        }
        
        db->resetKeys();
        
        db->closeFile();
        
        condition = trans(condition);
        
        int last_pages[100000];
        int last_positions[100000];
        int last_len = 0;
        
        int pre_pages[100000];
        int pre_positions[100000];
        int pre_len = 0;
        
        if(condition.operands.size() == 0)
        {
            last_len = dd->getAllPiece(table_path, last_pages, last_positions);
        }
        
        for(int i=0;i<condition.operands.size();i++)
        {
            int now_pages[100000] = {};
            int now_pos[100000] = {};
            int now_len = 0;
            if(condition.operands[i].op == CondEntry::LIKE || condition.operands[i].op == CondEntry::IN)
                continue;
            if(condition.operands[i].left.type == Expr::ATTR)
            {
                if(condition.operands[i].right.type == Expr::ATTR)
                {
                    cout << "[ERROR] WHERE search failed!" << endl;
                    delete db;
                    delete dd;
                    return;
                }
                string attr_name[1] = {condition.operands[i].left.attr.attrname.c_str()};
                string attr_data[1] = {};
                int attrNull[1];
                int attrCmp[1];
                int type = -1;
                if(condition.operands[i].op == CondEntry::IS)
                {
                    attrNull[0] = 0;
                    attrCmp[0] = 0;
                }
                else
                {
                    attrNull[0] = 1;
                }
                if(condition.operands[i].right.type == Expr::INTEGER)
                {
                    char number[100] = {};
                    sprintf(number, "%d", condition.operands[i].right.integer);
                    attr_data[0] = number;
                    type = 0;
                }
                else if(condition.operands[i].right.type == Expr::LITERAL)
                {
                    attr_data[0] = condition.operands[i].right.literal;
                    type = 1;
                }
                
                if(condition.operands[i].op == CondEntry::EQUAL)
                    attrCmp[0] = 0;
                else if(condition.operands[i].op == CondEntry::NEQUAL)
                    attrCmp[0] = 1;
                else if(condition.operands[i].op == CondEntry::GREATER)
                    attrCmp[0] = 2;
                else if(condition.operands[i].op == CondEntry::LESS)
                    attrCmp[0] = 3;
                else if(condition.operands[i].op == CondEntry::GREATERE)
                    attrCmp[0] = 4;
                else if(condition.operands[i].op == CondEntry::LESSE)
                    attrCmp[0] = 5;

                
                int k;
                for(k=0;k<db->t_number;k++)
                {
                    if(attr_name[0] == db->t_name[k])
                    {
                        if(attrNull[0] == 0 && db->t_null[k] == 0)
                            break;
                        if((type == 0 && db->t_type[k]!=0) || (type == 1 && db->t_type[k]!=1 && db->t_type[k]!=3))
                        {
                            cout << "[ERROR] WHERE search failed!" << endl;
                            delete db;
                            delete dd;
                            return;
                        }
                        if(type == 1)
                        {
                            if(attr_data[0].size() > db->t_length[k])
                            {
                                cout << "[ERROR] String is more than limitation!" << endl;
                                delete db;
                                delete dd;
                                return;
                            }
                        }
                        break;
                    }
                }
                
                
                if(k == db->t_number)
                {
                    cout << "[ERROR] WHERE search failed!" << endl;
                    delete db;
                    delete dd;
                    return;
                }

                now_len = dd->whereSearch(table_path, attr_name, attr_data, attrNull, attrCmp, 1, now_pages, now_pos);
            }
            else
            {
                if(condition.operands[i].right.type == Expr::INTEGER || condition.operands[i].right.type == Expr::LITERAL)
                {
                    cout << "[ERROR] WHERE search failed!" << endl;
                    delete db;
                    delete dd;
                    return;
                }
                string attr_name[1] = {condition.operands[i].right.attr.attrname.c_str()};
                string attr_data[1] = {};
                int attrNull[1];
                int attrCmp[1];
                int type = -1;
                if(condition.operands[i].op == CondEntry::IS)
                {
                    attrNull[0] = 0;
                    attrCmp[0] = 0;
                }
                else
                {
                    attrNull[0] = 1;
                }
                if(condition.operands[i].left.type == Expr::INTEGER)
                {
                    char number[100] = {};
                    sprintf(number, "%d", condition.operands[i].left.integer);
                    attr_data[0] = number;
                    type = 0;
                }
                else if(condition.operands[i].left.type == Expr::LITERAL)
                {
                    attr_data[0] = condition.operands[i].left.literal;
                    type = 1;
                }
                
                if(condition.operands[i].op == CondEntry::EQUAL)
                    attrCmp[0] = 0;
                else if(condition.operands[i].op == CondEntry::NEQUAL)
                    attrCmp[0] = 1;
                else if(condition.operands[i].op == CondEntry::GREATER)
                    attrCmp[0] = 3;
                else if(condition.operands[i].op == CondEntry::LESS)
                    attrCmp[0] = 2;
                else if(condition.operands[i].op == CondEntry::GREATERE)
                    attrCmp[0] = 5;
                else if(condition.operands[i].op == CondEntry::LESSE)
                    attrCmp[0] = 4;

                int k;
                for(k=0;k<db->t_number;k++)
                {
                    if(attr_name[0] == db->t_name[k])
                    {
                        if((type == 0 && db->t_type[k]!=0) || (type == 1 && db->t_type[k]!=1 && db->t_type[k]!=3))
                        {
                            cout << "[ERROR] WHERE search failed!" << endl;
                            delete db;
                            delete dd;
                            return;
                        }
                        if(type == 1)
                        {
                            if(attr_data[0].size() > db->t_length[k])
                            {
                                cout << "[ERROR] String is more than limitation!" << endl;
                                delete db;
                                delete dd;
                                return;
                            }
                        }
                        break;
                    }
                }
                
                if(k == db->t_number)
                {
                    cout << "[ERROR] WHERE search failed!" << endl;
                    delete db;
                    delete dd;
                    return;
                }
                
                now_len = dd->whereSearch(table_path, attr_name, attr_data, attrNull, attrCmp, 1, now_pages, now_pos);
            }
            
            if(i == 0)
            {
                for(int j=0;j<now_len;j++)
                {
                    pre_pages[j] = now_pages[j];
                    pre_positions[j] = now_pos[j];
                    last_pages[j] = now_pages[j];
                    last_positions[j] = now_pos[j];
                }
                pre_len = now_len;
                last_len = now_len;
            }
            else
            {
                if(condition.opcodes[i-1] == Condition::AND)
                {
                    last_len = 0;
                    for(int i=0;i<now_len;i++)
                        for(int j=0;j<pre_len;j++)
                        {
                            if(pre_pages[j] == now_pages[i] && pre_positions[j] == now_pos[i])
                            {
                                last_pages[last_len] = now_pages[i];
                                last_positions[last_len] = now_pos[i];
                                last_len++;
                                break;
                            }
                        }
                    for(int i=0;i<last_len;i++)
                    {
                        pre_pages[i] = last_pages[i];
                        pre_positions[i] = last_positions[i];
                    }
                    pre_len = last_len;
                }
                else
                {
                    last_len = 0;
                    for(int i=0;i<now_len;i++)
                    {
                        int j = 0;
                        for(j=0;j<pre_len;j++)
                        {
                            if(pre_pages[j] == now_pages[i] && pre_positions[j] == now_pos[i])
                            {
                                break;
                            }
                        }
                        if(j == pre_len)
                        {
                            last_pages[last_len] = now_pages[i];
                            last_positions[last_len] = now_pos[i];
                            last_len++;
                        }
                    }
                    for(int i=0;i<pre_len;i++)
                    {
                        last_pages[last_len] = pre_pages[i];
                        last_positions[last_len] = pre_positions[i];
                        last_len++;
                    }
                    for(int i=0;i<last_len;i++)
                    {
                        pre_pages[i] = last_pages[i];
                        pre_positions[i] = last_positions[i];
                    }
                    pre_len = last_len;
                }
            }
        }
        
        for(int i=0;i<last_len;i++)
        {
            for(int j=i;j<last_len;j++)
                if(last_positions[i]<last_positions[j])
                {
                    int k;
                    k = last_pages[i];
                    last_pages[i] = last_pages[j];
                    last_pages[j] = k;
                    k = last_positions[i];
                    last_positions[i] = last_positions[j];
                    last_positions[j] = k;
                }
        }
        
        if(last_len == 0)
        {
            cout << "[ERROR] No such infomation!" << endl;
            delete db;
            delete dd;
            return;
        }
        
        dd->Delete(table_path, last_pages, last_positions, last_len);
        
        delete db;
        delete dd;
        break;
    }
    case UPDATE:
    {
        
        DB_File* db = new DB_File();
        
        QL_Manager* dd = new QL_Manager();
        
        string table_path(filemanager->npath);
        table_path = table_path + '/' + tbname;
        
        if(db->openFile(table_path.c_str()) == 0)
        {
            cout << "[ERROR] There is no such table!";
            delete db;
            delete dd;
            return;
        }
        
        db->resetKeys();
        
        db->closeFile();
        
        condition = trans(condition);
        
        int last_pages[100000];
        int last_positions[100000];
        int last_len = 0;
        
        int pre_pages[100000];
        int pre_positions[100000];
        int pre_len = 0;
        
        if(condition.operands.size() == 0)
        {
            last_len = dd->getAllPiece(table_path, last_pages, last_positions);
        }
        
        for(int i=0;i<condition.operands.size();i++)
        {
            int now_pages[100000] = {};
            int now_pos[100000] = {};
            int now_len = 0;
            if(condition.operands[i].op == CondEntry::LIKE || condition.operands[i].op == CondEntry::IN)
                continue;
            if(condition.operands[i].left.type == Expr::ATTR)
            {
                if(condition.operands[i].right.type == Expr::ATTR)
                {
                    cout << "[ERROR] WHERE search failed!" << endl;
                    delete db;
                    delete dd;
                    return;
                }
                string attr_name[1] = {condition.operands[i].left.attr.attrname.c_str()};
                string attr_data[1] = {};
                int attrNull[1];
                int attrCmp[1];
                int type = -1;
                if(condition.operands[i].op == CondEntry::IS)
                {
                    attrNull[0] = 0;
                    attrCmp[0] = 0;
                }
                else
                {
                    attrNull[0] = 1;
                }
                if(condition.operands[i].right.type == Expr::INTEGER)
                {
                    char number[100] = {};
                    sprintf(number, "%d", condition.operands[i].right.integer);
                    attr_data[0] = number;
                    type = 0;
                }
                else if(condition.operands[i].right.type == Expr::LITERAL)
                {
                    attr_data[0] = condition.operands[i].right.literal;
                    type = 1;
                }
                
                if(condition.operands[i].op == CondEntry::EQUAL)
                    attrCmp[0] = 0;
                else if(condition.operands[i].op == CondEntry::NEQUAL)
                    attrCmp[0] = 1;
                else if(condition.operands[i].op == CondEntry::GREATER)
                    attrCmp[0] = 2;
                else if(condition.operands[i].op == CondEntry::LESS)
                    attrCmp[0] = 3;
                else if(condition.operands[i].op == CondEntry::GREATERE)
                    attrCmp[0] = 4;
                else if(condition.operands[i].op == CondEntry::LESSE)
                    attrCmp[0] = 5;

                int k;
                for(k=0;k<db->t_number;k++)
                {
                    if(attr_name[0] == db->t_name[k])
                    {
                        if(attrNull[0] == 0 && db->t_null[k] == 0)
                            break;
                        if((type == 0 && db->t_type[k]!=0) || (type == 1 && db->t_type[k]!=1 && db->t_type[k]!=3))
                        {
                            cout << "[ERROR] WHERE search failed!" << endl;
                            delete db;
                            delete dd;
                            return;
                        }
                        if(type == 1)
                        {
                            if(attr_data[0].size() > db->t_length[k])
                            {
                                cout << "[ERROR] String is more than limitation!" << endl;
                                delete db;
                                delete dd;
                                return;
                            }
                        }
                        break;
                    }
                }

                if(k == db->t_number)
                {
                    cout << "[ERROR] WHERE search failed!" << endl;
                    delete db;
                    delete dd;
                    return;
                }

                now_len = dd->whereSearch(table_path, attr_name, attr_data, attrNull, attrCmp, 1, now_pages, now_pos);
            }
            else
            {
                if(condition.operands[i].right.type == Expr::INTEGER || condition.operands[i].right.type == Expr::LITERAL)
                {
                    cout << "[ERROR] WHERE search failed!" << endl;
                    delete db;
                    delete dd;
                    return;
                }
                string attr_name[1] = {condition.operands[i].right.attr.attrname.c_str()};
                string attr_data[1] = {};
                int attrNull[1];
                int attrCmp[1];
                int type = -1;
                if(condition.operands[i].op == CondEntry::IS)
                {
                    attrNull[0] = 0;
                    attrCmp[0] = 0;
                }
                else
                {
                    attrNull[0] = 1;
                }
                if(condition.operands[i].left.type == Expr::INTEGER)
                {
                    char number[100] = {};
                    sprintf(number, "%d", condition.operands[i].left.integer);
                    attr_data[0] = number;
                    type = 0;
                }
                else if(condition.operands[i].left.type == Expr::LITERAL)
                {
                    attr_data[0] = condition.operands[i].left.literal;
                    type = 1;
                }
                
                if(condition.operands[i].op == CondEntry::EQUAL)
                    attrCmp[0] = 0;
                else if(condition.operands[i].op == CondEntry::NEQUAL)
                    attrCmp[0] = 1;
                else if(condition.operands[i].op == CondEntry::GREATER)
                    attrCmp[0] = 3;
                else if(condition.operands[i].op == CondEntry::LESS)
                    attrCmp[0] = 2;
                else if(condition.operands[i].op == CondEntry::GREATERE)
                    attrCmp[0] = 5;
                else if(condition.operands[i].op == CondEntry::LESSE)
                    attrCmp[0] = 4;

                int k;
                for(k=0;k<db->t_number;k++)
                {
                    if(attr_name[0] == db->t_name[k])
                    {
                        if((type == 0 && db->t_type[k]!=0) || (type == 1 && db->t_type[k]!=1 && db->t_type[k]!=3))
                        {
                            cout << "[ERROR] WHERE search failed!" << endl;
                            delete db;
                            delete dd;
                            return;
                        }
                        if(type == 1)
                        {
                            if(attr_data[0].size() > db->t_length[k])
                            {
                                cout << "[ERROR] String is more than limitation!" << endl;
                                delete db;
                                delete dd;
                                return;
                            }
                        }
                        break;
                    }
                }
                
                if(k == db->t_number)
                {
                    cout << "[ERROR] WHERE search failed!" << endl;
                    delete db;
                    delete dd;
                    return;
                }
                
                now_len = dd->whereSearch(table_path, attr_name, attr_data, attrNull, attrCmp, 1, now_pages, now_pos);
            }
            
            if(i == 0)
            {
                for(int j=0;j<now_len;j++)
                {
                    pre_pages[j] = now_pages[j];
                    pre_positions[j] = now_pos[j];
                    last_pages[j] = now_pages[j];
                    last_positions[j] = now_pos[j];
                }
                pre_len = now_len;
                last_len = now_len;
            }
            else
            {
                if(condition.opcodes[i-1] == Condition::AND)
                {
                    last_len = 0;
                    for(int i=0;i<now_len;i++)
                        for(int j=0;j<pre_len;j++)
                        {
                            if(pre_pages[j] == now_pages[i] && pre_positions[j] == now_pos[i])
                            {
                                last_pages[last_len] = now_pages[i];
                                last_positions[last_len] = now_pos[i];
                                last_len++;
                                break;
                            }
                        }
                    for(int i=0;i<last_len;i++)
                    {
                        pre_pages[i] = last_pages[i];
                        pre_positions[i] = last_positions[i];
                    }
                    pre_len = last_len;
                }
                else
                {
                    last_len = 0;
                    for(int i=0;i<now_len;i++)
                    {
                        int j = 0;
                        for(j=0;j<pre_len;j++)
                        {
                            if(pre_pages[j] == now_pages[i] && pre_positions[j] == now_pos[i])
                            {
                                break;
                            }
                        }
                        if(j == pre_len)
                        {
                            last_pages[last_len] = now_pages[i];
                            last_positions[last_len] = now_pos[i];
                            last_len++;
                        }
                    }
                    for(int i=0;i<pre_len;i++)
                    {
                        last_pages[last_len] = pre_pages[i];
                        last_positions[last_len] = pre_positions[i];
                        last_len++;
                    }
                    for(int i=0;i<last_len;i++)
                    {
                        pre_pages[i] = last_pages[i];
                        pre_positions[i] = last_positions[i];
                    }
                    pre_len = last_len;
                }
            }
        }
        
        for(int i=0;i<updateAttr.size();i++)
        {
            int j;
            for(j=0;j<db->t_number;j++)
            {
                if(updateAttr[i] == db->t_name[j])
                    break;
            }
            if(j == db->t_number)
            {
                cout << "Key is not exist" << endl;
                delete db;
                delete dd;
                return;
            }
            if(db->t_primary_one == j)
            {
                if(last_len > 2)
                {
                    cout << "[ERROR] The Primary Key should not be the same!" << endl;
                    delete db;
                    delete dd;
                    return;
                }
                if(updateValue[i].type == 2)
                {
                    cout << "[ERROR] The Primary Key should not be NULL!" << endl;
                    delete db;
                    delete dd;
                    return;
                }
            }
            if(db->t_type[j] == 0 && updateValue[i].type !=0 )
            {
                cout << "[ERROR] The Type is wrong!" << endl;
                delete db;
                delete dd;
                return;
            }
            if((db->t_type[j] == 1 || db->t_type[j] == 3) && updateValue[i].type != 1)
            {
                cout << "[ERROR] The Type is wrong!" << endl;
                delete db;
                delete dd;
                return;
            }
            if(db->t_type[j] == 1 && updateValue[i].type == 1)
            {
                if(db->t_length[j] != updateValue[i].literal.size())
                {
                    cout << "[ERROR] The char's size is fixed!" << endl;
                    delete db;
                    delete dd;
                    return;
                }
            }
        }
        
        if(isTuo)
        {
            for(int i=0;i<updateAttr.size();i++)
            {
                string filename = table_path + "_foreign/" + updateAttr[i];
                ifstream fin(filename.c_str());
                if(!fin)
                    continue;
                string tablename;
                string tablekey;
                fin >> tablename;
                fin >> tablekey;
                DB_File* db_f = new DB_File();
                string f_table_path(filemanager->npath);
                f_table_path = f_table_path + '/' + tablename;
                if(db_f->openFile(f_table_path.c_str())==0)
                {
                    cout << "[ERROR] Foreign table disappeared!" << endl;
                    delete db;
                    delete db_f;
                    return;
                }
                db_f->resetKeys();
                if(updateValue[i].type == 2)
                    continue;
                QL_Manager* dd_2 = new QL_Manager();
                int pages[10000];
                int positions[10000];
                int len;
                string attrName[1] = tablekey;
                string attrData[1] = {};
                int null[1] = {1};
                int cpm[1] = {0};
                
                if(updateValue[i].type == 0)
                {
                    char numb[100] = {};
                    sprintf(numb, "%d", updateValue[i].integer);
                    attrData[0] = numb;
                }
                else
                {
                    attrData[0] = updateValue[i].literal;
                }
                
                len = dd->whereSearch(f_table_path, attrName, attrData, null, cpm, 1, pages, positions);
                
                if(len == 0)
                {
                    cout << "[EEROR] Foreign key's data doesn't exist!" << endl;
                    delete db_f;
                    delete dd;
                    delete dd_2;
                    delete db;
                    return;
                }
            }
        }
        
        if(isTuo)
        {
            for(int i=0;i<updateAttr.size();i++)
            {
                string filename = table_path + "_res/" + updateAttr[i];
                ifstream fin(filename.c_str());
                if(!fin)
                    continue;
                if(updateValue[i].type == 0)
                {
                    bool flag = 0;
                    int k;
                    while(fin >> k)
                    {
                        if(k == updateValue[i].integer)
                        {
                            flag = 1;
                            break;
                        }
                    }
                    if(flag == 0)
                    {
                        cout << "[ERROR] Key check failed" << endl;
                        delete dd;
                        delete db;
                        return;
                    }
                }
                else
                {
                    bool flag = 0;
                    string k;
                    while(fin >> k)
                    {
                        if(k == updateValue[i].literal)
                        {
                            flag = 1;
                            break;
                        }
                    }
                    if(flag == 0)
                    {
                        cout << "[ERROR] Key check failed" << endl;
                        delete dd;
                        delete db;
                        return;
                    }
                }
            }
        }
        
        int numm = updateAttr.size();;
        string attrName[numm] = {};
        string attrData[numm] = {};
        int attrNull[numm] = {};
        
        for(int i=0;i<numm;i++)
        {
            attrName[i] = updateAttr[i];
            if(updateValue[i].type == 0)
            {
                char number[20] = {};
                sprintf(number, "%d", updateValue[i].integer);
                attrData[i] = number;
            }
            if(updateValue[i].type == 1)
            {
                attrData[i] = updateValue[i].literal;
            }
            if(updateValue[i].type == 2)
            {
                attrNull[i] = 1;
            }
        }
        
        if(last_len == 0)
        {
            cout << "[ERROR] No such infomation!" << endl;
            delete db;
            delete dd;
            return;
        }
        
        bool rong = dd->Update(table_path, last_pages, last_positions, attrName, attrData, attrNull, numm, last_len);
        
        if(rong == false)
        {
            cout << "[ERROR] Not NULL for NOT NULL Keys!" << endl;
            delete db;
            delete dd;
            return;
        }
        
        delete db;
        delete dd;
        
        break;
    }
    case SELECT:
    {
        
        condition = trans(condition);
        
        if(tables.size() == 2)
        {
            DB_File* db_1 = new DB_File();
            DB_File* db_2 = new DB_File();
            QL_Manager* dd = new QL_Manager();
            
            string table_path(filemanager->npath);
            string table_path_1 = table_path + '/' + tables[0].c_str();
            string table_path_2 = table_path + '/' + tables[1].c_str();
            
            if(db_1->openFile(table_path_1.c_str()) == 0)
            {
                cout << "[ERROR] There is no such table!";
            }
            db_1->resetKeys();
            
            if(db_2->openFile(table_path_2.c_str()) == 0)
            {
                cout << "[ERROR] There is no such table!";
            }
            db_2->resetKeys();
            
            int number = db_1->t_number + db_2->t_number;
            string names[number];
            bool null[number];
            int length[number];
            int types[number];
            
            int l = 0;
            
            for(int i=0;i<db_1->t_number;i++)
            {
                names[l] = tables[0] + '.' + db_1->t_name[i];
                null[l] = db_1->t_null[i];
                length[l] = db_1->t_length[i];
                types[l] = db_1->t_type[i];
                l++;
            }
            
            for(int i=0;i<db_2->t_number;i++)
            {
                names[l] = tables[1] + '.' + db_2->t_name[i];
                null[l] = db_2->t_null[i];
                length[l] = db_2->t_length[i];
                types[l] = db_2->t_type[i];
                l++;
            }
            
            DB_File* db_ins = new DB_File();
            
            string table_path_ins = table_path + '/' + "link_two";
            db_ins->createFile(table_path_ins.c_str());
            db_ins->openFile(table_path_ins.c_str());
            db_ins->initFile(types, names, null, 0, length, number);
            db_ins->resetKeys();
            
            int page_1[100000];
            int page_2[100000];
            int pos_1[100000];
            int pos_2[100000];
            int num_1;
            int num_2;
            
            num_1 = db_1->getAllPiece(page_1, pos_1);
            
            num_2 = db_2->getAllPiece(page_2, pos_2);
            
            bool flag =0;
            
            if(condition.operands[0].left.type == Expr::ATTR && condition.operands[0].right.type == Expr::ATTR)

            {
                flag = 1;
                string key_1 = condition.operands[0].left.attr.tbname + '.' + condition.operands[0].left.attr.attrname;
                int pos_key_1 = -1;
                for(int jj=0;jj<db_ins->t_number;jj++)
                {
                    if(db_ins->t_name[jj] == key_1)
                    {
                        pos_key_1 = jj;
                        break;
                    }
                }

                string key_2 = condition.operands[0].right.attr.tbname + '.' + condition.operands[0].right.attr.attrname;
                int pos_key_2 = -1;

                for(int jj=0;jj<db_ins->t_number;jj++)
                {
                    if(db_ins->t_name[jj] == key_2)
                    {
                        pos_key_2 = jj;
                        break;
                    }
                }

                if(pos_key_1 == -1 || pos_key_2 == -1)
                {
                    cout << "[ERROR] Key is not exist!" << endl;
                    db_ins->deleteFile();
                    delete db_ins;
                    delete dd;
                    delete db_1;
                    delete db_2;
                    return;
                }

                if(db_ins->t_type[pos_key_1] != db_ins->t_type[pos_key_2])
                {
                    cout << "[ERROR] Type is wrong!" << endl;
                    db_ins->deleteFile();
                    delete db_ins;
                    delete dd;
                    delete db_1;
                    delete db_2;
                    return;
                }

                for(int i=0;i<num_1;i++)
                {
                    unsigned int b_in[1000];
                    unsigned int* b_1 = db_1->getPiece(page_1[i], pos_1[i]);
                    for(int j=0;j<db_1->t_position[db_1->t_number];j++)
                        b_in[j] = b_1[j];
                    for(int j=0;j<num_2;j++)
                    {
                        unsigned int* b_2 = db_2->getPiece(page_2[j], pos_2[j]);
                        for(int k=0;k<db_1->t_position[db_1->t_number];k++)
                            b_in[db_1->t_position[db_1->t_number]+k] = b_2[k];

                        int n_type = db_ins->t_type[pos_key_1];

                        unsigned int* n_piece = b_in;
                        int pos_1_begin = db_ins->t_position[pos_key_1];
                        int pos_2_begin = db_ins->t_position[pos_key_2];
                        int pos_1_end = db_ins->t_position[pos_key_1 + 1];
                        int pos_2_end = db_ins->t_position[pos_key_2 + 1];
                        if(condition.operands[0].op == CondEntry::EQUAL && n_piece[pos_1_begin] == 0 && n_piece[pos_2_begin] == 0)
                        {
                            db_ins->addData(b_in);
                            continue;
                        }
                        if(condition.operands[0].op == CondEntry::NEQUAL && n_piece[pos_1_begin] != n_piece[pos_2_begin])
                        {
                            db_ins->addData(b_in);
                            continue;
                        }
                        if(n_piece[pos_1_begin] == 0 || n_piece[pos_2_begin] == 0)
                        {
                            continue;
                        }
                        if(n_type == 0)
                        {
                            int n1 = int(n_piece[pos_1_begin + 1]);
                            int n2 = int(n_piece[pos_2_begin + 1]);
                            if(condition.operands[0].op == CondEntry::EQUAL && n1 == n2)
                            {
                                db_ins->addData(b_in);
                                continue;
                            }
                            if(condition.operands[0].op == CondEntry::NEQUAL && n1 != n2)
                            {
                                db_ins->addData(b_in);
                                continue;
                            }
                            if(condition.operands[0].op == CondEntry::GREATER && n1 > n2)
                            {
                                db_ins->addData(b_in);
                                continue;
                            }
                            if(condition.operands[0].op == CondEntry::GREATERE && n1 >= n2)
                            {
                                db_ins->addData(b_in);
                                continue;
                            }
                            if(condition.operands[0].op == CondEntry::LESS && n1 < n2)
                            {
                                db_ins->addData(b_in);
                                continue;
                            }
                            if(condition.operands[0].op == CondEntry::LESSE && n1 <= n2)
                            {
                                db_ins->addData(b_in);
                                continue;
                            }
                        }
                        else if (n_type == 1 || n_type == 3)
                        {
                            CodeTransfer *cf = new CodeTransfer();
                            char* n1 = new char[8001];
                            char* n2 = new char[8001];
                            cf->decode(n1, &n_piece[pos_1_begin + 1], db_ins->t_length[pos_key_1]);
                            cf->decode(n2, &n_piece[pos_2_begin + 1], db_ins->t_length[pos_key_2]);
                            delete cf;
                            if(condition.operands[0].op == CondEntry::EQUAL && strcmp(n1, n2) == 0)
                            {
                                db_ins->addData(b_in);
                                continue;
                            }
                            if(condition.operands[0].op == CondEntry::NEQUAL && strcmp(n1, n2) != 0)
                            {
                                db_ins->addData(b_in);
                                continue;
                            }
                            if(condition.operands[0].op == CondEntry::GREATER && strcmp(n1, n2) > 0)
                            {
                                db_ins->addData(b_in);
                                continue;
                            }
                            if(condition.operands[0].op == CondEntry::GREATERE && strcmp(n1, n2) >= 0)
                            {
                                db_ins->addData(b_in);
                                continue;
                            }
                            if(condition.operands[0].op == CondEntry::LESS && strcmp(n1, n2) < 0)
                            {
                                db_ins->addData(b_in);
                                continue;
                            }
                            if(condition.operands[0].op == CondEntry::LESSE && strcmp(n1, n2) <= 0)
                            {
                                db_ins->addData(b_in);
                                continue;
                            }
                        }
                    }
                }
            }
            else
            {
                for(int i=0;i<num_1;i++)
                {
                    unsigned int b_in[1000];
                    unsigned int* b_1 = db_1->getPiece(page_1[i], pos_1[i]);
                    for(int j=0;j<db_1->t_position[db_1->t_number];j++)
                        b_in[j] = b_1[j];
                    for(int j=0;j<num_2;j++)
                    {
                        unsigned int* b_2 = db_2->getPiece(page_2[j], pos_2[j]);
                        for(int k=0;k<db_1->t_position[db_1->t_number];k++)
                            b_in[db_1->t_position[db_1->t_number]+k] = b_2[k];
                        db_ins->addData(b_in);
                    }
                }
            }
            
            db_ins->closeFile();

            int* last_pages = new int[100000];
            int* last_positions = new int[100000];
            int last_len = 0;
            int* pre_pages = new int[100000];
            int* pre_positions = new int[100000];
            int pre_len = 0;

            if(condition.operands.size() == 0)
            {
                last_len = dd->getAllPiece(table_path, last_pages, last_positions);
            }

            for(int i=0;i<condition.operands.size();i++)
            {
                int now_pages[100000] = {};
                int now_pos[100000] = {};
                int now_len = 0;
                if(condition.operands[i].op == CondEntry::LIKE || condition.operands[i].op == CondEntry::IN)
                    continue;
                if(condition.operands[i].left.type == Expr::ATTR)
                {
                    if(condition.operands[i].right.type == Expr::ATTR)
                    {
                        string key_1 = condition.operands[i].left.attr.tbname + '.' + condition.operands[i].left.attr.attrname;
                        int pos_key_1 = -1;
                        for(int j=0;j<db_ins->t_number;j++)
                        {
                            if(db_ins->t_name[j] == key_1)
                            {
                                pos_key_1 = j;
                                break;
                            }
                        }

                        string key_2 = condition.operands[i].right.attr.tbname + '.' + condition.operands[i].right.attr.attrname;
                        int pos_key_2 = -1;

                        for(int j=0;j<db_ins->t_number;j++)
                        {
                            if(db_ins->t_name[j] == key_2)
                            {
                                pos_key_2 = j;
                                break;
                            }
                        }

                        if(pos_key_1 == -1 || pos_key_2 == -1)
                        {
                            cout << "[ERROR] Key is not exist!" << endl;
                            db_ins->deleteFile();
                            delete last_pages;
                            delete last_positions;
                            delete pre_pages;
                            delete pre_positions;
                            delete db_ins;
                            delete dd;
                            delete db_1;
                            delete db_2;
                            return;
                        }

                        if(db_ins->t_type[pos_key_1] != db_ins->t_type[pos_key_2])
                        {
                            cout << "[ERROR] Type is wrong!" << endl;
                            db_ins->deleteFile();
                            delete last_pages;
                            delete last_positions;
                            delete pre_pages;
                            delete pre_positions;
                            delete db_ins;
                            delete dd;
                            delete db_1;
                            delete db_2;
                            return;
                        }

                        int n_type = db_ins->t_type[pos_key_1];
                        int n_pages[100000];
                        int n_pos[100000];
                        int n_num;
                        n_num = dd->getAllPiece(table_path_ins, n_pages, n_pos);
                        db_ins->openFile(table_path_ins.c_str());
                        db_ins->resetKeys();


                        for(int ii=0;ii<n_num;ii++)
                        {
                            unsigned int* n_piece = db_ins->getPiece(n_pages[ii], n_pos[ii]);
                            int pos_1_begin = db_ins->t_position[pos_key_1];
                            int pos_2_begin = db_ins->t_position[pos_key_2];
                            int pos_1_end = db_ins->t_position[pos_key_1 + 1];
                            int pos_2_end = db_ins->t_position[pos_key_2 + 1];
                            if(condition.operands[i].op == CondEntry::EQUAL && n_piece[pos_1_begin] == 0 && n_piece[pos_2_begin] == 0)
                            {
                                now_pages[now_len] = n_pages[i];
                                now_pos[now_len] = n_pos[i];
                                now_len++;
                                continue;
                            }
                            if(condition.operands[i].op == CondEntry::NEQUAL && n_piece[pos_1_begin] != n_piece[pos_2_begin])
                            {
                                now_pages[now_len] = n_pages[i];
                                now_pos[now_len] = n_pos[i];
                                now_len++;
                                continue;
                            }
                            if(n_piece[pos_1_begin] == 0 || n_piece[pos_2_begin] == 0)
                            {
                                continue;
                            }
                            if(n_type == 0)
                            {
                                int n1 = int(n_piece[pos_1_begin + 1]);
                                int n2 = int(n_piece[pos_2_begin + 1]);
                                if(condition.operands[i].op == CondEntry::EQUAL && n1 == n2)
                                {
                                    now_pages[now_len] = n_pages[ii];
                                    now_pos[now_len] = n_pos[ii];
                                    now_len++;
                                    continue;
                                }
                                if(condition.operands[i].op == CondEntry::NEQUAL && n1 != n2)
                                {
                                    now_pages[now_len] = n_pages[ii];
                                    now_pos[now_len] = n_pos[ii];
                                    now_len++;
                                    continue;
                                }
                                if(condition.operands[i].op == CondEntry::GREATER && n1 > n2)
                                {
                                    now_pages[now_len] = n_pages[ii];
                                    now_pos[now_len] = n_pos[ii];
                                    now_len++;
                                    continue;
                                }
                                if(condition.operands[i].op == CondEntry::GREATERE && n1 >= n2)
                                {
                                    now_pages[now_len] = n_pages[ii];
                                    now_pos[now_len] = n_pos[ii];
                                    now_len++;
                                    continue;
                                }
                                if(condition.operands[i].op == CondEntry::LESS && n1 < n2)
                                {
                                    now_pages[now_len] = n_pages[ii];
                                    now_pos[now_len] = n_pos[ii];
                                    now_len++;
                                    continue;
                                }
                                if(condition.operands[i].op == CondEntry::LESSE && n1 <= n2)
                                {
                                    now_pages[now_len] = n_pages[ii];
                                    now_pos[now_len] = n_pos[ii];
                                    now_len++;
                                    continue;
                                }
                            }
                            else if (n_type == 1 || n_type == 3)
                            {
                                CodeTransfer *cf = new CodeTransfer();
                                char* n1 = new char[8001];
                                char* n2 = new char[8001];
                                cf->decode(n1, &n_piece[pos_1_begin + 1], db_ins->t_length[pos_key_1]);
                                cf->decode(n2, &n_piece[pos_2_begin + 1], db_ins->t_length[pos_key_2]);
                                if(condition.operands[i].op == CondEntry::EQUAL && strcmp(n1, n2) == 0)
                                {
                                    now_pages[now_len] = n_pages[ii];
                                    now_pos[now_len] = n_pos[ii];
                                    now_len++;
                                    continue;
                                }
                                if(condition.operands[i].op == CondEntry::NEQUAL && strcmp(n1, n2) != 0)
                                {
                                    now_pages[now_len] = n_pages[ii];
                                    now_pos[now_len] = n_pos[ii];
                                    now_len++;
                                    continue;
                                }
                                if(condition.operands[i].op == CondEntry::GREATER && strcmp(n1, n2) > 0)
                                {
                                    now_pages[now_len] = n_pages[ii];
                                    now_pos[now_len] = n_pos[ii];
                                    now_len++;
                                    continue;
                                }
                                if(condition.operands[i].op == CondEntry::GREATERE && strcmp(n1, n2) >= 0)
                                {
                                    now_pages[now_len] = n_pages[ii];
                                    now_pos[now_len] = n_pos[ii];
                                    now_len++;
                                    continue;
                                }
                                if(condition.operands[i].op == CondEntry::LESS && strcmp(n1, n2) < 0)
                                {
                                    now_pages[now_len] = n_pages[ii];
                                    now_pos[now_len] = n_pos[ii];
                                    now_len++;
                                    continue;
                                }
                                if(condition.operands[i].op == CondEntry::LESSE && strcmp(n1, n2) <= 0)
                                {
                                    now_pages[now_len] = n_pages[ii];
                                    now_pos[now_len] = n_pos[ii];
                                    now_len++;
                                    continue;
                                }
                                delete n1;
                                delete n2;
                            }
                        }

                    }
                    else
                    {
                        string attr_name[1] = {condition.operands[i].left.attr.attrname.c_str()};
                        attr_name[0] = condition.operands[i].left.attr.tbname + '.' + attr_name[0] ;
                        string attr_data[1] = {};
                        int attrNull[1];
                        int attrCmp[1];
                        int type = -1;
                        if(condition.operands[i].op == CondEntry::IS)
                        {
                            attrNull[0] = 0;
                            attrCmp[0] = 0;
                        }
                        else
                        {
                            attrNull[0] = 1;
                        }
                        if(condition.operands[i].right.type == Expr::INTEGER)
                        {
                            char number[100] = {};
                            sprintf(number, "%d", condition.operands[i].right.integer);
                            attr_data[0] = number;
                            type = 0;
                        }
                        else if(condition.operands[i].right.type == Expr::LITERAL)
                        {
                            attr_data[0] = condition.operands[i].right.literal;
                            type = 1;
                        }

                        if(condition.operands[i].op == CondEntry::EQUAL)
                            attrCmp[0] = 0;
                        else if(condition.operands[i].op == CondEntry::NEQUAL)
                            attrCmp[0] = 1;
                        else if(condition.operands[i].op == CondEntry::GREATER)
                            attrCmp[0] = 2;
                        else if(condition.operands[i].op == CondEntry::LESS)
                            attrCmp[0] = 3;
                        else if(condition.operands[i].op == CondEntry::GREATERE)
                            attrCmp[0] = 4;
                        else if(condition.operands[i].op == CondEntry::LESSE)
                            attrCmp[0] = 5;

                        int k;
                        for(k=0;k<db_ins->t_number;k++)
                        {
                            if(attr_name[0] == db_ins->t_name[k])
                            {
                                if(attrNull[0] == 0 && db_ins->t_null[k] == 0)
                                    break;
                                if((type == 0 && db_ins->t_type[k]!=0) || (type == 1 && db_ins->t_type[k]!=1 && db_ins->t_type[k]!=3))
                                {
                                    cout << "[ERROR] WHERE search failed!" << endl;
                                    delete db_ins;
                                    delete dd;
                                    delete last_pages;
                                    delete last_positions;
                                    delete pre_pages;
                                    delete pre_positions;
                                    return;
                                }
                                if(type == 1)
                                {
                                    if(attr_data[0].size() > db_ins->t_length[k])
                                    {
                                        cout << "[ERROR] String is more than limitation!" << endl;
                                        delete db_ins;
                                        delete dd;
                                        delete last_pages;
                                        delete last_positions;
                                        delete pre_pages;
                                        delete pre_positions;
                                        return;
                                    }
                                }
                                break;
                            }
                        }

                        if(k == db_ins->t_number)
                        {
                            cout << "[ERROR] WHERE search failed!" << endl;
                            delete db_ins;
                            delete dd;
                            delete last_pages;
                            delete last_positions;
                            delete pre_pages;
                            delete pre_positions;
                            return;
                        }

                        now_len = dd->whereSearch(table_path_ins, attr_name, attr_data, attrNull, attrCmp, 1, now_pages, now_pos);
                    }
                }
                else
                {
                    if(condition.operands[i].right.type == Expr::INTEGER || condition.operands[i].right.type == Expr::LITERAL)
                    {
                        cout << "[ERROR] WHERE search failed!" << endl;
                        delete db_ins;
                        delete dd;
                        delete last_pages;
                        delete last_positions;
                        delete pre_pages;
                        delete pre_positions;
                        return;
                    }
                    string attr_name[1] = {condition.operands[i].right.attr.attrname.c_str()};
                    attr_name[0] = condition.operands[i].right.attr.tbname + '.' + attr_name[0];
                    string attr_data[1] = {};
                    int attrNull[1];
                    int attrCmp[1];
                    int type = -1;
                    if(condition.operands[i].op == CondEntry::IS)
                    {
                        attrNull[0] = 0;
                        attrCmp[0] = 0;
                    }
                    else
                    {
                        attrNull[0] = 1;
                    }
                    if(condition.operands[i].left.type == Expr::INTEGER)
                    {
                        char number[100] = {};
                        sprintf(number, "%d", condition.operands[i].left.integer);
                        attr_data[0] = number;
                        type = 0;
                    }
                    else if(condition.operands[i].left.type == Expr::LITERAL)
                    {
                        attr_data[0] = condition.operands[i].left.literal;
                        type = 1;
                    }

                    if(condition.operands[i].op == CondEntry::EQUAL)
                        attrCmp[0] = 0;
                    else if(condition.operands[i].op == CondEntry::NEQUAL)
                        attrCmp[0] = 1;
                    else if(condition.operands[i].op == CondEntry::GREATER)
                        attrCmp[0] = 3;
                    else if(condition.operands[i].op == CondEntry::LESS)
                        attrCmp[0] = 2;
                    else if(condition.operands[i].op == CondEntry::GREATERE)
                        attrCmp[0] = 5;
                    else if(condition.operands[i].op == CondEntry::LESSE)
                        attrCmp[0] = 4;
                    
                    int k;
                    for(k=0;k<db_ins->t_number;k++)
                    {
                        if(attr_name[0] == db_ins->t_name[k])
                        {
                            if((type == 0 && db_ins->t_type[k]!=0) || (type == 1 && db_ins->t_type[k]!=1 && db_ins->t_type[k]!=3))
                            {
                                cout << "[ERROR] WHERE search failed!" << endl;
                                delete db_ins;
                                delete dd;
                                delete last_pages;
                                delete last_positions;
                                delete pre_pages;
                                delete pre_positions;
                                return;
                            }
                            if(type == 1)
                            {
                                if(attr_data[0].size() > db_ins->t_length[k])
                                {
                                    cout << "[ERROR] String is more than limitation!" << endl;
                                    delete db_ins;
                                    delete dd;
                                    delete last_pages;
                                    delete last_positions;
                                    delete pre_pages;
                                    delete pre_positions;
                                    return;
                                }
                            }
                            break;
                        }
                    }

                    if(k == db_ins->t_number)
                    {
                        cout << "[ERROR] WHERE search failed!" << endl;
                        delete db_ins;
                        delete dd;
                        delete last_pages;
                        delete last_positions;
                        delete pre_pages;
                        delete pre_positions;
                        return;
                    }

                    now_len = dd->whereSearch(table_path_ins, attr_name, attr_data, attrNull, attrCmp, 1, now_pages, now_pos);

                }

                if(i == 0)
                {
                    for(int j=0;j<now_len;j++)
                    {
                        pre_pages[j] = now_pages[j];
                        pre_positions[j] = now_pos[j];
                        last_pages[j] = now_pages[j];
                        last_positions[j] = now_pos[j];
                    }
                    pre_len = now_len;
                    last_len = now_len;
                }
                else
                {
                    if(condition.opcodes[i-1] == Condition::AND)
                    {
                        last_len = 0;
                        for(int i=0;i<now_len;i++)
                            for(int j=0;j<pre_len;j++)
                            {
                                if(pre_pages[j] == now_pages[i] && pre_positions[j] == now_pos[i])
                                {
                                    last_pages[last_len] = now_pages[i];
                                    last_positions[last_len] = now_pos[i];
                                    last_len++;
                                    break;
                                }
                            }
                        for(int i=0;i<last_len;i++)
                        {
                            pre_pages[i] = last_pages[i];
                            pre_positions[i] = last_positions[i];
                        }
                        pre_len = last_len;
                    }
                    else
                    {
                        last_len = 0;
                        for(int i=0;i<now_len;i++)
                        {
                            int j = 0;
                            for(j=0;j<pre_len;j++)
                            {
                                if(pre_pages[j] == now_pages[i] && pre_positions[j] == now_pos[i])
                                {
                                    break;
                                }
                            }
                            if(j == pre_len)
                            {
                                last_pages[last_len] = now_pages[i];
                                last_positions[last_len] = now_pos[i];
                                last_len++;
                            }
                        }
                        for(int i=0;i<pre_len;i++)
                        {
                            last_pages[last_len] = pre_pages[i];
                            last_positions[last_len] = pre_positions[i];
                            last_len++;
                        }
                        for(int i=0;i<last_len;i++)
                        {
                            pre_pages[i] = last_pages[i];
                            pre_positions[i] = last_positions[i];
                        }
                        pre_len = last_len;
                    }
                }

            }

            db_1->closeFile();
            db_2->closeFile();
            
            string attrName[attrs.size()];
            int a_num = 0;
            
            for(int i=0;i<attrs.size();i++)
            {
                if(attrs[i].aggr != Attr::NONE)
                    continue;
                int j=0;
                string in_name = attrs[i].tbname + '.' + attrs[i].attrname;
                for(j=0;j<db_ins->t_number;j++)
                {
                    if(in_name == db_ins->t_name[j])
                        break;
                }
                if(j<db_ins->t_number)
                {
                    attrName[a_num++] = in_name;
                }
                else
                {
                    cout << "no column" << endl;
                    return;
                }
            }

            if(last_len == 0)
            {
                db_ins->deleteFile(table_path_ins.c_str());

                delete last_pages;
                delete last_positions;
                delete pre_pages;
                delete pre_positions;
                delete db_1;
                delete db_2;
                delete dd;
                delete db_ins;
                return;
            }
            
            string new_ins;
            if(attrs.size() != 0)
            {
                new_ins = dd->Select(table_path_ins, attrName, a_num, last_pages, last_positions, last_len);
            }
            else
            {
                new_ins = dd->Select(table_path_ins, db_ins->t_name, db_ins->t_number, last_pages, last_positions, last_len);
            }
            
            db_ins->deleteFile(table_path_ins.c_str());
            db_ins->deleteFile(new_ins.c_str());
            
            delete last_pages;
            delete last_positions;
            delete pre_pages;
            delete pre_positions;
            delete db_1;
            delete db_2;
            delete dd;
            delete db_ins;
            return;
        }
        if(tables.size() == 1)
        {
            DB_File* db = new DB_File();

            QL_Manager* dd = new QL_Manager();

            string table_path(filemanager->npath);
            table_path = table_path + '/' + tables[0];

            if(db->openFile(table_path.c_str()) == 0)
            {
                cout << "[ERROR] There is no such table!" << endl;;
                delete db;
                delete dd;
                return;
            }
            db->resetKeys();

            db->closeFile();

            int* last_pages = new int[100000];
            int* last_positions = new int[100000];
            int last_len = 0;
            int* pre_pages = new int[100000];
            int* pre_positions = new int[100000];
            int pre_len = 0;

            if(condition.operands.size() == 0)
            {
                last_len = dd->getAllPiece(table_path, last_pages, last_positions);
            }

            for(int i=0;i<condition.operands.size();i++)
            {
                int now_pages[100000] = {};
                int now_pos[100000] = {};
                int now_len = 0;
                if(condition.operands[i].op == CondEntry::LIKE || condition.operands[i].op == CondEntry::IN)
                {

                    if(i!=0)
                        continue;
                    if(condition.operands[i].op == CondEntry::IN)
                        continue;
                    else
                    {
                        db->openFile(table_path.c_str());
                        db->resetKeys();

                        now_len = db->getAllPiece(now_pages, now_pos);

                        if(condition.operands[i].left.type != Expr::ATTR)
                        {
                            db->closeFile();
                            continue;
                        }

                        string name = condition.operands[i].left.attr.attrname;

                        int lab;
                        for(lab = 0;lab < db->t_number;lab++)
                        {
                            if(name == db->t_name[lab])
                                break;
                        }

                        if(lab == db->t_number)
                        {
                            cout << "[ERROR] Key does not exist!" << endl;
                            db->closeFile();
                            delete db;
                            delete dd;
                            return;
                        }

                        for(int ind = 0;ind < now_len;ind++)
                        {
                            char source[8001] = {};
                            unsigned int* bu = db->getPiece(now_pages[ind], now_pos[ind]);
                            CodeTransfer* cf =  new CodeTransfer();
                            cf->decode(source, &bu[db->t_position[lab]+1], db->t_length[lab]);
                            delete cf;
                            string sou(source);
                            bool gett = like(condition.operands[i].mode, sou);
                            if(gett)
                            {
                                last_pages[last_len] = now_pages[ind];
                                last_positions[last_len] = now_pos[ind];
                                last_len++;
                            }
                        }
                        db->closeFile();
                        continue;
                    }
                }
                if(condition.operands[i].left.type == Expr::ATTR)
                {
                    if(condition.operands[i].right.type == Expr::ATTR)
                    {
                        cout << "[ERROR] WHERE search failed!" << endl;
                        delete db;
                        delete dd;
                        return;
                    }
                    string attr_name[1] = {condition.operands[i].left.attr.attrname.c_str()};
                    string attr_data[1] = {};
                    int attrNull[1];
                    int attrCmp[1];
                    int type = -1;
                    if(condition.operands[i].op == CondEntry::IS)
                    {
                        attrNull[0] = 0;
                        attrCmp[0] = 0;
                    }
                    else
                    {
                        attrNull[0] = 1;
                    }
                    if(condition.operands[i].right.type == Expr::INTEGER)
                    {
                        char number[100] = {};
                        sprintf(number, "%d", condition.operands[i].right.integer);
                        attr_data[0] = number;
                        type = 0;
                    }
                    else if(condition.operands[i].right.type == Expr::LITERAL)
                    {
                        attr_data[0] = condition.operands[i].right.literal;
                        type = 1;
                    }

                    if(condition.operands[i].op == CondEntry::EQUAL)
                        attrCmp[0] = 0;
                    else if(condition.operands[i].op == CondEntry::NEQUAL)
                        attrCmp[0] = 1;
                    else if(condition.operands[i].op == CondEntry::GREATER)
                        attrCmp[0] = 2;
                    else if(condition.operands[i].op == CondEntry::LESS)
                        attrCmp[0] = 3;
                    else if(condition.operands[i].op == CondEntry::GREATERE)
                        attrCmp[0] = 4;
                    else if(condition.operands[i].op == CondEntry::LESSE)
                        attrCmp[0] = 5;

                    int k;
                    for(k=0;k<db->t_number;k++)
                    {
                        if(attr_name[0] == db->t_name[k])
                        {
                            if(attrNull[0] == 0 && db->t_null[k] == 0)
                                break;
                            if((type == 0 && db->t_type[k]!=0) || (type == 1 && db->t_type[k]!=1 && db->t_type[k]!=3))
                            {
                                cout << "[ERROR] WHERE search failed!" << endl;
                                delete db;
                                delete dd;
                                return;
                            }
                            if(type == 1)
                            {
                                if(attr_data[0].size() > db->t_length[k])
                                {
                                    cout << "[ERROR] String is more than limitation!" << endl;
                                    delete db;
                                    delete dd;
                                    return;
                                }
                            }
                            break;
                        }
                    }

                    if(k == db->t_number)
                    {
                        cout << "[ERROR] WHERE search failed!" << endl;
                        delete db;
                        delete dd;
                        return;
                    }
                    //struct timeb start, end;
                    //ftime(&start);
                    now_len = dd->whereSearch(table_path, attr_name, attr_data, attrNull, attrCmp, 1, now_pages, now_pos);
                    //ftime(&end);
                    //cout << "Searching time: ";
                    //cout << (int)((end.time - start.time) * 1000 + end.millitm - start.millitm) << endl;
                }
                else
                {
                    if(condition.operands[i].right.type == Expr::INTEGER || condition.operands[i].right.type == Expr::LITERAL)
                    {
                        cout << "[ERROR] WHERE search failed!" << endl;
                        delete db;
                        delete dd;
                        return;
                    }
                    string attr_name[1] = {condition.operands[i].right.attr.attrname.c_str()};
                    string attr_data[1] = {};
                    int attrNull[1];
                    int attrCmp[1];
                    int type = -1;
                    if(condition.operands[i].op == CondEntry::IS)
                    {
                        attrNull[0] = 0;
                        attrCmp[0] = 0;
                    }
                    else
                    {
                        attrNull[0] = 1;
                    }
                    if(condition.operands[i].left.type == Expr::INTEGER)
                    {
                        char number[100] = {};
                        sprintf(number, "%d", condition.operands[i].left.integer);
                        attr_data[0] = number;
                        type = 0;
                    }
                    else if(condition.operands[i].left.type == Expr::LITERAL)
                    {
                        attr_data[0] = condition.operands[i].left.literal;
                        type = 1;
                    }

                    if(condition.operands[i].op == CondEntry::EQUAL)
                        attrCmp[0] = 0;
                    else if(condition.operands[i].op == CondEntry::NEQUAL)
                        attrCmp[0] = 1;
                    else if(condition.operands[i].op == CondEntry::GREATER)
                        attrCmp[0] = 3;
                    else if(condition.operands[i].op == CondEntry::LESS)
                        attrCmp[0] = 2;
                    else if(condition.operands[i].op == CondEntry::GREATERE)
                        attrCmp[0] = 5;
                    else if(condition.operands[i].op == CondEntry::LESSE)
                        attrCmp[0] = 4;
                    
                    int k;
                    for(k=0;k<db->t_number;k++)
                    {
                        if(attr_name[0] == db->t_name[k])
                        {
                            if((type == 0 && db->t_type[k]!=0) || (type == 1 && db->t_type[k]!=1 && db->t_type[k]!=3))
                            {
                                cout << "[ERROR] WHERE search failed!" << endl;
                                delete db;
                                delete dd;
                                return;
                            }
                            if(type == 1)
                            {
                                if(attr_data[0].size() > db->t_length[k])
                                {
                                    cout << "[ERROR] String is more than limitation!" << endl;
                                    delete db;
                                    delete dd;
                                    return;
                                }
                            }
                            break;
                        }
                    }

                    if(k == db->t_number)
                    {
                        cout << "[ERROR] WHERE search failed!" << endl;
                        delete db;
                        delete dd;
                        return;
                    }

                    now_len = dd->whereSearch(table_path, attr_name, attr_data, attrNull, attrCmp, 1, now_pages, now_pos);
                }

                if(i == 0)
                {
                    for(int j=0;j<now_len;j++)
                    {
                        pre_pages[j] = now_pages[j];
                        pre_positions[j] = now_pos[j];
                        last_pages[j] = now_pages[j];
                        last_positions[j] = now_pos[j];
                    }
                    pre_len = now_len;
                    last_len = now_len;
                }
                else
                {
                    if(condition.opcodes[i-1] == Condition::AND)
                    {
                        last_len = 0;
                        for(int i=0;i<now_len;i++)
                            for(int j=0;j<pre_len;j++)
                            {
                                if(pre_pages[j] == now_pages[i] && pre_positions[j] == now_pos[i])
                                {
                                    last_pages[last_len] = now_pages[i];
                                    last_positions[last_len] = now_pos[i];
                                    last_len++;
                                    break;
                                }
                            }
                        for(int i=0;i<last_len;i++)
                        {
                            pre_pages[i] = last_pages[i];
                            pre_positions[i] = last_positions[i];
                        }
                        pre_len = last_len;
                    }
                    else
                    {
                        last_len = 0;
                        for(int i=0;i<now_len;i++)
                        {
                            int j = 0;
                            for(j=0;j<pre_len;j++)
                            {
                                if(pre_pages[j] == now_pages[i] && pre_positions[j] == now_pos[i])
                                {
                                    break;
                                }
                            }
                            if(j == pre_len)
                            {
                                last_pages[last_len] = now_pages[i];
                                last_positions[last_len] = now_pos[i];
                                last_len++;
                            }
                        }
                        for(int i=0;i<pre_len;i++)
                        {
                            last_pages[last_len] = pre_pages[i];
                            last_positions[last_len] = pre_positions[i];
                            last_len++;
                        }
                        for(int i=0;i<last_len;i++)
                        {
                            pre_pages[i] = last_pages[i];
                            pre_positions[i] = last_positions[i];
                        }
                        pre_len = last_len;
                    }
                }
            }
            
            if(attrs.size() > 0 && attrs[0].aggr != Attr::NONE)
            {
                string new_ins;
                new_ins = dd->Select(table_path, db->t_name, db->t_number, last_pages, last_positions, last_len);
                DB_File* db_ins =  new DB_File();
                db_ins->openFile(new_ins.c_str());
                db_ins->resetKeys();
                int kk = 0;
                for(kk = 0;kk<db_ins->t_number;kk++)
                {
                    if(attrs[0].attrname == db_ins->t_name[kk])
                    {
                        break;
                    }
                }
                int type = db_ins->t_type[kk];
                
                if(type != 0)
                {
                    cout << "[ERROR] Only INT can Gather!" << endl;
                    db_ins->deleteFile(new_ins.c_str());
                    delete db_ins;
                    delete db;
                    delete dd;
                    return;
                }
                
                BPlusTree* aim_b = db_ins->trees[kk];
                
                search_pos first = aim_b->keySearchMax(0, 1, 1);
                
                if(first.pos.pageNum == 0 && first.pos.posNum == 0)
                {
                    cout << "[ERROR] No avilable Key for Gather!" << endl;
                    db_ins->deleteFile(new_ins.c_str());
                    delete db_ins;
                    delete db;
                    delete dd;
                    return;
                }
                
                if(attrs[0].aggr == Attr::MIN)
                {
                    page node = aim_b->getNode(first.pos.pageNum, first.pos.posNum);
                    long long result = node.data[first.index];
                    result = result - 3;
                    result = result - 2147483647;
                    cout << "result : " << result << endl;
                }
                else
                {
                    int last_average = 0;
                    long long last_sum = 0;
                    int max = 0;
                    int l = 0;
                    page now = aim_b->getNode(first.pos.pageNum, first.pos.posNum);
                    int ini = first.index;
                    while(1)
                    {
                        for(;ini<2*DU-1;ini++)
                        {
                            if(now.data[ini] == 0)
                                break;
                            long long result = now.data[ini];
                            result = result - 3;
                            result = result - 2147483647;
                            last_sum = last_sum + result;
                            l++;
                        }
                        if((now.sibling.pageNum == 0 && now.sibling.posNum == 0))
                        {
                            long long result = now.data[ini - 1];
                            result = result - 3;
                            result = result - 2147483647;
                            max = (int)result;
                            break;
                        }
                        else
                            now = aim_b->getNode(now.sibling.pageNum, now.sibling.posNum);
                        ini = 0;
                    }
                    last_average = last_sum/l;
                    if(attrs[0].aggr == Attr::MAX)
                        cout << "Result : " << max << endl;
                    if(attrs[0].aggr == Attr::SUM)
                        cout << "Result : " << last_sum << endl;
                    if(attrs[0].aggr == Attr::AVG)
                        cout << "Result : " << last_average << endl;
                }
                
                db_ins->deleteFile(new_ins.c_str());
                delete db_ins;
                delete db;
                delete dd;
                return;
            }
            
            string attrName[attrs.size()];
            int a_num = 0;
            
            for(int i=0;i<attrs.size();i++)
            {
                if(attrs[i].aggr != Attr::NONE)
                    continue;
                int j=0;
                string in_name = attrs[i].attrname;
                for(j=0;j<db->t_number;j++)
                {
                    if(in_name == db->t_name[j])
                        break;
                }
                if(j<db->t_number)
                {
                    attrName[a_num++] = in_name;
                }
                else
                {
                    cout << "no column" << endl;
                    return;
                }
            }
            
            for(int i=0;i<a_num;i++)
                cout << attrName[i] << endl;

            if(last_len == 0)
            {
                cout << "[INFO] No data avilable" << endl;
                delete db;
                delete dd;
                return;
            }
            
            string new_ins;
            if(attrs.size() != 0)
            {
                new_ins = dd->Select(table_path, attrName, a_num, last_pages, last_positions, last_len);
            }
            else
            {
                new_ins = dd->Select(table_path, db->t_name, db->t_number, last_pages, last_positions, last_len);
            }

            db->deleteFile(new_ins.c_str());
            delete db;
            delete dd;
            delete last_pages;
            delete last_positions;
            delete pre_pages;
            delete pre_positions;
        }
        break;
    }
    }
}
