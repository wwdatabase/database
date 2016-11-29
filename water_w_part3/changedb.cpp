#include<iostream>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
using namespace std;

int main (int argc,char **argv )      // length of array argv
   // argv is an array of pointers
   // argv[0] points to the name of the exe file
     // argv[1] points to argument DBname
{
   char *dbname;
   char command[80] = "cd ../";
   char last_dir[100];
   getcwd(last_dir,sizeof(last_dir)); // get current directory
   int len = strlen(last_dir);
   last_dir[len] = '/';
   last_dir[len+1] = '\0';
   strcat(last_dir,dbname);
   if (argc != 2) {
      cerr << "Usage: " << argv[0] << " dbname \n";
      exit(1);
   }
   // The database name is the second argument
   dbname = argv[1];
   // Create a subdirectory for the database
   system (strcat(command,dbname));
   char buffer[100];
   getcwd(buffer,sizeof(buffer));
   if(strcmp(buffer,last_dir) == 0)
   {
     printf("Change directory successfully!\n");
   }
   else {
     cerr << "Oh no!!!!" << "\n";
   }
return 0;
}
