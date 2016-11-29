#include<iostream>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
using namespace std;

int main ( int argc, char **argv)     // length of array argv
   // argv is an array of pointers
     // argv[0] points to the name of the exe file
     // argv[1] points to argument DBname
{
   char *dbname;
   char command[80] = "rm -r ";

   if (argc != 2) {
      cerr << "Usage: " << argv[0] << " dbname \n";
      exit(1);
   }
   // The database name is the second argument
   dbname = argv[1];
   // Create a subdirectory for the database
   int res = system (strcat(command,dbname));
   if(res != 0) // failed to destroy
   {
     cerr << argv[0] << " rmdir error for " << dbname << "\n";
     exit(res);
   }

return 0;
}
