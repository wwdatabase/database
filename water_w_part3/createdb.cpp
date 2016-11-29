#include<iostream>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
using namespace std;

int main (int argc,char **argv )      // length of array argv
   // argv is an array of pointers
   // argv[0] points to the name of the exe file
    // argv[1] points to argument DBname
{// here we didn't handle the repeated case
   char *dbname;
   char command[80] = "mkdir ";
   if (argc != 2) {
      cerr << "Usage: " << argv[0] << " dbname \n";
      exit(1);
   }
   // The database name is the second argument
   dbname = argv[1];
   // Create a subdirectory for the database
   system (strcat(command,dbname));
   if (chdir(dbname) < 0) {
      cerr << argv[0] << " chdir error to " << dbname << "\n";
      exit(1);
   }
   else printf("create successfully\n");
return 0;
}
