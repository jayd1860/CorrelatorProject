#pragma once

#include <stdint.h>

using namespace System;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;

double getElapsedTime(unsigned long long start, int unit);
unsigned long long getCurrTime(char* timeref);
void getCurrDateTimeString(wchar_t* datetime);
void getCurrDateTime(struct tm* timeinfo, unsigned short* datetime);
int browseForFolder(wchar_t* rootdir, wchar_t* szPath);
void convertCharToWchar(wchar_t* dst, char* src, int len);
void convertWcharToChar(char* dst, wchar_t* src, int len);
int numbitson_init(void);
int numbitson(uint32_t x);
int convertStringToChar(String^ src, char* dst);
int strlen_u(unsigned char* s);
int convertStringToWchar(String^ src, wchar_t* dst);
void getFileExtension(wchar_t* filename, wchar_t* ext);


ref class MyLock
{

public:
    // -------------------------------------------
    MyLock()
    {
        resource=0;
    }

    // -------------------------------------------
    ~MyLock()
    {

    }

    // -------------------------------------------
    int acquire(int timeout)
    {
        int elapsedtime=0;
        while(resource)  {
            Sleep(10);
            elapsedtime += 10;
            if(elapsedtime>timeout)
                return -1;
        }
        resource = 1;
        return 0;
    }

    // -------------------------------------------
    void release()
    {
        resource = 0;
    }


private:

    int resource;

};