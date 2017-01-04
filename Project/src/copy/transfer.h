#ifndef WF_TRANSFER
#define WF_TRANSFER

#include <cstdio>
#include <cstdlib>
#include <string.h>

class CodeTransfer
{
public:
    CodeTransfer(){}
    //encode
    void encode(double code, unsigned int codes[])
    {
        unsigned int * k= (unsigned int *)(&(code));
        codes[0] = k[0];
        codes[1] = k[1];
    }
    void encode(const char* code, unsigned int codes[], int length)
    {
        int lenth = strlen(code);
        int kk = length/4;
        if( length%4 !=0 )
            kk++;
        int len = kk*4;
        char c[len]={};
        int end = len - lenth;
        for(int i=end;i<len;i++)
            c[i] = code[i-end];
        unsigned int* k = (unsigned int *)c;
        for(int i=0;i<kk;i++)
            codes[i] = k[i];
    }

    //decode
    void decode(double& code, unsigned int codes[])
    {
        unsigned int k[2]={};
        k[0] = codes[0];
        k[1] = codes[1];
        code = *((double*)k);
    }
    void decode(char code[], unsigned int codes[], int length)
    {
        char* k = (char*) codes;
        int l = 0;
        int i;
        int lenth = strlen(code);
        int kk = length/4;
        if( length%4 !=0 )
            kk++;
        int len = kk*4;
        for(i=0;i<len;i++)
            if(k[i]!=0) break;
        for(int j=i;j<len;j++)
            code[j-i] = k[j];
    }
};

#endif
