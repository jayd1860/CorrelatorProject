#ifndef PATHPROCESS_H
#define PATHPROCESS_H

#define MAX_FILENAME_LEN 512
#define MAX_DIR_NUM 50
#define MAX_DIRNAME_LEN 100
#define MAX_DIRSEP_LEN 15

// Extern declaration 
#if defined(_WIN32)

#ifdef I_AM_UTILS_DLL
#define PATHPROCDLLAPI_U extern "C" __declspec(dllexport)
#else 
#define PATHPROCDLLAPI_U extern "C" __declspec(dllimport)
#endif 

#elif defined(_LINUX_) 

#ifdef I_AM_UTILS_DLL
#define PATHPROCDLLAPI_U
#else
#define PATHPROCDLLAPI_U extern
#endif

#endif


class Pathparts
{
public:
    Pathparts();
    ~Pathparts();
    char* dirname;
    char** dirsep;
};



PATHPROCDLLAPI_U char* pathProcess(char* pathname0);
PATHPROCDLLAPI_U Pathparts* getpathparts(char* pathname);
PATHPROCDLLAPI_U char* buildpath(Pathparts* pp);

#endif

