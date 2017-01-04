#ifndef _SYS_MANAGER_H
#define _SYS_MANAGER_H

#include "../rcdmana/rm.h"
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>

class DB
{
public:
    char name[1024];
    DB *next;
    DB_File *fh;

    DB()
    {
        next = NULL;
        fh = NULL;
    }
    void initDB(char* path)
    {
        DB_File* p, *q;
        
        string pa(path);
        pa = pa + '/' + name;
        
        DIR* nowDir = opendir(pa.c_str());
        
        pa = pa + '/';
        dirent* nowFile;
        
        while((nowFile = readdir(nowDir)) != NULL)
        {
            string fileName(nowFile->d_name);
            if(nowFile->d_type == 4) continue;
            int length = strlen(fileName.c_str());
            if(fileName == "../" || fileName == "./") continue;
            p = new DB_File();
            p->next = NULL;
            p->setDBpath(pa.c_str());
            p->openFile(fileName.c_str());
            p->resetKeys();
            p->closeFile();
            if(fh == NULL)
            {
                fh = p;
                q = fh;
            }
            else
            {
                q->next = p;
                q = p;
            }
        }
        closedir(nowDir);
    }
    bool create_file(char *filename, int type[], string name[], bool type_not_null[], int primary_one, int type_length[], int number, char *npath)
    {
        DB_File *p = new DB_File();
        char temp[1024];
        char tt[10] = "/";
        strcpy(temp, npath);
        strcat(temp, tt);
        p->setDBpath(temp);
        if (p->openFile(filename) == 1)
        {
            cout << "[Error] This table already exists!" << endl;
            delete p;
            return 0;
        }
        p->createFile(filename);
        p->openFile(filename);
        p->initFile(type, name, type_not_null, primary_one, type_length, number);
        p->resetKeys();
        p->closeFile();
        p->next = fh;
        fh = p;
        return 1;
    }
    void showTables()
    {
        DB_File *p = fh;
        if (!p)
        {
            cout << "The database is null now!" << endl;
            return;
        }
        while (p)
        {
            cout << p->getFilename() << endl;
            p = p->next;
        }
    }
    bool drop_file(char *filename)
    {
        DB_File *p = fh, *q = NULL;
        while (p)
        {
            if (strcmp(p->getFilename(), filename) == 0)
            {
                p->deleteFile();
                if (q) q->next = p->next;
                else fh = p->next;
                delete p;
                cout << "Delete table " << filename << " successfully!" << endl;
                return 1;
            }
            q = p;
            p = p->next;
        }
        cout << "[Error] There is no such table!" << endl;
        return 0;
    }
    bool show(char *filename)
    {
        DB_File *p = fh;
        while (p)
        {
            if (strcmp(p->getFilename(), filename) == 0)
            {
                p->showStores();
                return 1;
            }
            p = p->next;
        }
        cout << "[Error] There is no such table!" << endl;
        return 0;
    }
};

class DBT
{
    DB *head;

public:
    char path[1024];
    char npath[1024];
    char nowName[1024];

    DBT()
    {
        head = NULL;
        getcwd(path, 1024);
        char temp[15] = "/database";
        strcat(path, temp);
        strcpy(npath, path);
        strcpy(nowName, "");
        
        DB* p, *q;
        
        DIR* nowDir = opendir(path);
        dirent* nowFile;
        
        while((nowFile = readdir(nowDir)) != NULL)
        {
            string fileName(nowFile->d_name);
            if(nowFile->d_type == 4) fileName += "/";
            int length = strlen(fileName.c_str());
            if(fileName == "../" || fileName == "./") continue;
            if(nowFile->d_type == 4)
            {
                p = new DB();
                strcpy(p->name, nowFile->d_name);
                p->initDB(path);
                if(head == NULL)
                {
                    head = p;
                    q = head;
                }
                else
                {
                    q->next = p;
                    q = p;
                }
            }
        }
        closedir(nowDir);
    }
    bool create(char *s)
    {
        DB *p = head;
        while (p)
        {
            if (strcmp(p->name, s) == 0)
            {
                cout << "[Error] This database already exists!" << endl;
                return 0;
            }
            p = p->next;
        }
        char temp[1024] = "mkdir ";
        strcat(temp, path);
        char tt[10] = "/";
        strcat(temp, tt);
        strcat(temp, s);
        system(temp);
        DB *q = new DB();
        q->next = NULL;
        if (head)
        {
            q->next = head;
            head = q;
        }
        else head = q;
        strcpy(q->name, s);
        cout << "Create database " << s << " successfully!" << endl;
        return 1;
    }
    bool drop(char *s)
    {
        DB *p = head, *q = NULL;
        while (p)
        {
            if (strcmp(p->name, s) == 0)
            {
                char temp[1024] = "rm -rf ";
                strcat(temp, path);
                char tt[10] = "/";
                strcat(temp, tt);
                strcat(temp, s);
                system(temp);
                if (strcmp(s, nowName) == 0)
                {
                    strcpy(nowName, "");
                    strcpy(npath, path);
                }
                if (q) q->next = p->next;
                else head = p->next;
                delete p;
                cout << "Drop database " << s << " successfully!" << endl;
                return 1;
            }
            q = p;
            p = p->next;
        }
        cout << "[Error] There is no such database!" << endl;
        return 0;
    }
    bool show_dbs()
    {
        DB *p = head, *q = NULL;
        while (p)
        {
            cout << (p->name) << endl;
            q = p;
            p = p->next;
        }
        return 0;
    }
    bool use(char *s)
    {
        DB *p = head;
        while (p)
        {
            if (strcmp(p->name, s) == 0)
            {
                char temp[1024];
                strcpy(temp, path);
                char tt[10] = "/";
                strcat(temp, tt);
                strcat(temp, s);
                strcpy(npath, temp);
                strcpy(nowName, s);
                cout << "Use database " << s << " successfully!" << endl;
                return 1;
            }
            p = p->next;
        }
        cout << "[Error] There is no such database!" << endl;
        return 0;
    }
    bool show()
    {
        DB *p = head;
        while (p)
        {
            if (strcmp(p->name, nowName) == 0)
            {
                p->showTables();
                return 1;
            }
            p = p->next;
        }
        cout << "[Error] There is no such database!" << endl;
        return 0;
    }
    bool create_file(char *filename, int type[], string name[], bool type_not_null[], int primary_one, int type_length[], int number)
    {
        DB *p = head;
        while (p)
        {
            if (strcmp(p->name, nowName) == 0)
            {
                return p->create_file(filename, type, name, type_not_null, primary_one, type_length, number, npath);
            }
            p = p->next;
        }
        cout << "Please choose a database!" << endl;
        return 0;
    }
    bool drop_file(char *filename)
    {
        DB *p = head;
        while (p)
        {
            if (strcmp(p->name, nowName) == 0)
            {
                return p->drop_file(filename);
            }
            p = p->next;
        }
        cout << "Please choose a database!" << endl;
        return 0;
    }
    bool show_file(char *filename)
    {
        DB *p = head;
        while (p)
        {
            if (strcmp(p->name, nowName) == 0)
            {
                return p->show(filename);
            }
            p = p->next;
        }
        cout << "Please choose a database!" << endl;
        return 0;
    }
};

#endif
