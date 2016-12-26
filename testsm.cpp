#include "smmana/sm.h"
#include <iostream>
#include <string.h>
#include<vector>

using namespace std;



int main()
{
	SM_Manager sm ;
	char tablename[255] = "testtable1";
	string attrname1[4] = {"attr1_1","attr1_2","attr1_3","attr1_4"};
	string attrname2[4] = {"attr2_1","attr2_2","attr2_3","attr2_4"};
	int attrtype1[4] = {ATTRINT,ATTRFLOAT,ATTRSTRING,ATTRSTRING};
	int attrtype2[4] = {ATTRINT,ATTRFLOAT,ATTRSTRING,ATTRINT};
	int len1[4] = {4,4,7,3};
	int len2[4]= {4,4,3,4};
	int attrnum1 = 4;
	int attrnum2=  4;
	int prikey1 = 2;
	int prikey2 = 0;
	int isnull_1[4] = {1,1,0,0};
	int isnull_2[4] = {0,0,1,1};
	char dbname[255] = "testdb";
	sm.create_db(dbname);
	sm.use_db(dbname);
	sm.create_table("tb1",attrname1,attrtype1,len1,isnull_1,attrnum1,prikey1);
	sm.create_table("tb2",attrname2,attrtype2,len2,isnull_2,attrnum2,prikey2);
	sm.show_db(dbname);
	sm.show_table("*");
return 0;


}

