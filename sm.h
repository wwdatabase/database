#ifndef SM_MANAGER
#define SM_MANAGER
#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<cstring>
#include<string.h>
#include<map>
#include"./rcdmana/rm.h"
#include"./utils/pagedef.h"
#include<vector>
#include<unistd.h>
#include<dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
using namespace std;
struct table_entry{
		char tablename[100];
		int recordSize;
		int attrNum;
		int prikeyNum;
		vector<string> attrName;
		vector<int> attrType;
		vector<int> attrLength;
		vector<int> isNull;
}
class DB{
	char dbname[255];
	vector<table_entry> table;
	DB(){;}
	DB(const char* name){
		strcpy(dbname,name);
		mkdir(name,0700);//create dir in root directory
	};
}


class SM_Manager{
public:
	vector<DB> databases; // store all informations about database
	int current_db_index= 0;
	RecordManager* record_manager;

	SM_Manager(){ //init the database
		record_manager = RecordManager::getInstance();
		DIR* nowdir = opendir(".");
		dirent* nowf;
		vector<string> dbs;
		while((nowf = readdir(nowdir))!= NULL)
		{
			string fileName(nowf->d_name);
			if(nowf->d_type == DT_DIR)
			{
				if(fileName == ".."||fileName == ".")
					continue;
				DB db;
				strcpy(db.dbname,fileName.c_str()); 
				databases.push_back(db);
			}
		}
		closedir(nowdir);
		for(int i = 0;i < databases.size();i++)
		{
			nowdir = opendir(databases[i].dbname);
			while((nowf = readdir(nowdir))!= NULL)
			{
				char c[255];
				strcpy(c,nowf->d_name);
				int len = strlen(c);
				for(int i = len-1;i >= 0;i--)
				{
					if(c[i] == '.')
					{
						if(strcmp(c+i+1,"www") == 0)// got table info
						{
							string 
							freopen(c,"r",stdin);
							table_entry tb;
							char temp_attr_name[255];
							int temp_type;
							int temp_attrlen;
							int temp_isnull;
							scanf("%s%d%d",&tb.tablename,&tb.attrNum,&tb.prikeyNum);
								for(int i = 0;i < tb.attrNum;i++)
								{
									scanf("%s%d%d%d",temp_attr_name,temp_type,temp_attrlen,temp_isnull);
									tb.attrName.push_back(temp_attr_name);
									tb.attrType.push_back(temp_type);
									tb.attrLength.push_back(temp_attrlen);
									tb.isNull.push_back(temp_isnull);
								}
							databases[i].table.push_back(tb);
						}
					}
				}

			}
			closedir(nowdir);


		}
		return 0;



	}
	RC use_db(const char* dbname);//cd to the directory  
  	RC create_db(const char* dbname);
  	RC drop_db(const char* dbname);
  	RC show_db(const char* dbname);
  	RC show_table(const char* tablename); // list all the table
  	RC create_table(const char* tablename,string attrname[],int attrtype[],int len[],int isnull[],int attrnum,int prim_key);
  	RC drop_table(const char* tablename);
}
RC SM_Manager::create_db(const char* dbname)
{
	char* name;
	DB new_db = new DB(dbname);
	databases.push_back(new_db);// revise the system file
	return 0;
}
RC SM_Manager::drop_db(const char* dbname)// still need to revise the system file
{
	for(vector<DB>::iterator i = databases.begin();i != databases.end();i++)
	{
		if(strcmp(dbname,i->dbname) == 0)
		{
			char command[255] = "rm -r ";
			strcat(command,dbname);
			system(command);
			printf("Deletion finished!\n");
			databases.erase(i);
			break;
		}
	}
	return 0;
}
RC SM_Manager::use_db(const char* dbname)
{
	int i;
	for(i = 0;i < databases.size();i++)
	{
		if(strcmp(dbname,databases[i].dbname) == 0)
		{
			current_db_index = i;
			printf("Current in database %s\n",databases[i].dbname);
			break;
		}
	}
	if(i == databases.size())
	{
		printf("Database %s does not exists!\n",dbname);
	}
	return 0;
}
RC SM_Manager::show_db(const char* dbname)
{
	cout << "Show database:" << endl;
	if(strcmp(dbname,"*") == 0)
	{
		for(vector<DB>::iterator i = databases.begin();i != databases.end();i++)
		{
				printf("Database name: %s\n",i->dbname);
				printf("Table name:\n");
				for(int j =0 ;j <(i->table).size();j++)
				{
					cout << (i->table)[j].tablename << endl;
				}
		}
	}
	else{
			for(vector<DB>::iterator i = databases.begin();i != databases.end();i++)
			{
				if(strcmp(dbname,i->dbname) == 0)
				{
					printf("Database name: %s\n",i->dbname);
					printf("Table name:\n");
					for(int j =0 ;j <(i->table).size();j++)
					{
						cout << (i->table)[j].tablename << endl;
					}
					break;
				}

			}
		}

	return 0;
}
RC SM_Manager::create_table(const char* tablename,string attrname[],int attrtype[],int len[],int isnull[],int attrnum,int prim_key)//create two files
{
	table_entry tb;
	strcpy(tb.tablename,tablename);
	tb.prikeyNum = prim_key;
	tb.attrNum = attrnum;
	int record_size = 0;
	for(int i = 0;i < attrnum;i++)
	{
		switch(attrtype[i])
		{
			case ATTRINT :
				record_size+=4;
				tb.attrName.push_back(attrname[i]);
				tb.attrType.push_back(ATTRINT);
				tb.attrLength.push_back(4);
				tb.isNull.push_back(isnull[i]);
				break;
			case ATTRFLOAT:
				record_size+=4;
				tb.attrName.push_back(attrname[i]);
				tb.attrType.push_back(ATTRFLOAT);
				tb.attrLength.push_back(4);
				tb.isNull.push_back(isnull[i]);
				break;
			case ATTRSTRING:
				record_size+=len[i];
				tb.attrName.push_back(attrname[i]);
				tb.attrType.push_back(ATTRSTRING);
				tb.attrLength.push_back(len[i]);
				tb.isNull.push_back(isnull[i]);
				break;
			default:
				printf("Blackman question mark?\n");
				break;
		}
	}
	tb.recordSize = record_size;
	databases[current_db_index].table.push_back(tb);
	string create_location(databases[current_db_index].dbname);
	create_location  = create_location+"/"+tb.tablename;
	record_manager->createFile(create_location.c_str(),record_size,tb.attrType,tb.attrLength,tb.isNull);
	create_location  = create_location + ".www";
	freopen(create_location.c_str(),"w",stdout);
	printf("%s\n%d %d\n",tb.tablename,tb.attrNum,tb.prikeyNum);
	for(int i = 0;i < tb.attrNum;i++)
	{
		printf("%s %d %d %d\n",tb.attrName[i],tb.attrType[i],tb.attrLength[i],tb.isNull[i]);
	}
	return 0;
}

RC SM_Manager::drop_table(const char* tablename)//////////////need more
{
	for(int i = 0;i < databases[current_db_index].table.size();i++)
	{
		if(strcmp(tablename,databases[current_db_index].table[i].tablename) == 0)
		{// delete this table,
			string de_file(dbname);
			de_file = de_file + "/"+tablename; 
			record_manager->destroyFile(de_file.c_str());
			databases[current_db_index].table.erase(databases[current_db_index].table.begin()+i);
			de_file = de_file+".www";
			record_manager->destroyFile(de_file.c_str());
				//delete corresponding table info
		}

	}
	printf("Table %s does not exists!\n",tablename);
	return 0;

}
RC SM_Manager::show_table(const char* tablename)
{
	cout << "Show table:" << endl;
	if(strcmp(tablename,"*") == 0)
	{
		cout << "show all tables!" << endl;
		for(int i = 0;i < databases.size();i++)
		{
			printf("Database name: %s\n",databases[i].dbname);
			for(int j = 0;j < databases[i].table.size();j++)
			{
				printf("Table name: %s\n",databases[i].table[j].tablename);
				printf("Table info:\n");
				cout << "Record Size: " << databases[i].table[j].recordSize << endl;
				for(int k = 0; k< databases[i].table[j].attrNum;k++)
				{
					cout << databases[i].table[j].attrName[k] <<" "<<databases[i].table[j].attrType[k] << " " << databases[i].table[j].attrLength[k] << " " << databases[i].table[j].isNull[k] << endl;
				}
				cout <<"Primary key number: " <<databases[i].table[j].prikeyNum << endl;
			}
		}

	}
	else
	{
		cout << "show one table!" << endl;
		for(int i = 0;i < databases.size();i++)
		{
			printf("Database name: %s\n",databases[i].dbname);
			for(int j = 0;j < databases[i].table.size();j++)
			{
				if(strcmp(tablename,databases[i].table[j].tablename) == 0)
				{
					printf("Table name: %s\n",databases[i].table[j].tablename);
					printf("Table info:\n");
					cout << "Record Size: " << databases[i].table[j].recordSize << endl;
					for(int k = 0; k< databases[i].table[j].attrNum;k++)
					{
						cout << databases[i].table[j].attrName[k] <<" "<<databases[i].table[j].attrType[k] << " " << databases[i].table[j].attrLength[k] << " " << databases[i].table[j].isNull[k] << endl;
					}
					cout <<"Primary key number: " <<databases[i].table[j].prikeyNum << endl;
					break;
				}
			}
		}
	}

	return 0;
}












#endif