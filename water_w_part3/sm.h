#ifndef SM_MANAGER
#define SM_MANAGER
#include<iostream>
#include<stdlib.h>
#include<stirng.h>
#include<map>
#include"../rcdmana/rm.h"
#include"../ixmana/ix.h" // humm
#include"parser.h"


struct AttrInfo {// used by SM_Manager:: CreateTable
   char     *attrName;           // Attribute name
   AttrType attrType;            // Type of attribute
   int      attrLength;          // Length of attribute
};
struct DataAttrInfo {// used by printer class
   char     relName[MAXNAME+1];  // Relation name
   char     attrName[MAXNAME+1]; // Attribute name
   int      offset;              // Offset of attribute
   AttrType attrType;            // Type of attribute
   int      attrLength;          // Length of attribute
   int      indexNo;             // Attribute index number
};


class SM_Manager{
  public:
  SM_Manager(IX_Manager &ixm,RM_Manager &rmm);// constructor
  ~SM_Manager();// destructor
  RC OpenDb      (const char *dbName);                // Open database
  RC CloseDb     ();                                  // Close database
  RC show_table       (const char *tableName);
  RC createDB   (const char *dbname);
  RC dropDB   (const char *dbname);
  RC useDB    (const char *dbname);
  RC CreateTable (const char *relName,                // Create relation
                  int        attrCount,
                  AttrInfo   *attributes);
  RC DropTable   (const char *relName);               // Destroy relation
}


RC SM_Manager::createDB (const char *dbname)
{
    char command[80] = "./createDB "
    int res = system(strcat(command,dbname)); // run the exe file
    if(res != 0)
      exit(-1);// failed
    return 0;
}
RC SM_Manager::dropDB (const char *dbname)
{
  char command[80] = "./dropDB "
  int res = system(strcat(command,dbname));
  if(res != 0)
    exit(-1);// failed
  return 0;
}

RC SM_Manager::useDB(const char *dbname)
{
  char command[80] = "./useDB " // i.e. changedb
  int res = system(strcat(command,dbname));
  if(res != 0)
    exit(-1);// failed
  return 0;
}
