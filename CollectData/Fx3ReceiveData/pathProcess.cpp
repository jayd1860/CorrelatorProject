#include "stdafx.h"
#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include "pathProcess.h"


/////////////////////////////////////////////////////////////////////////////////
Pathparts::Pathparts()
{
    dirname=NULL;
    dirsep=NULL;
}



/////////////////////////////////////////////////////////////////////////////////
Pathparts::~Pathparts()
{
    free(dirname);
    free(dirsep[0]);
    free(dirsep[1]);
}


/////////////////////////////////////////////////////////////////////////////////
Pathparts* getpathparts(char* pathname)
{
    Pathparts* pp = (Pathparts*)calloc(MAX_DIR_NUM, sizeof(Pathparts));
    char      dirsepstart[MAX_DIRSEP_LEN];
    char      dirname[MAX_DIRNAME_LEN];
    int       ii = 0;
    int       jj = 0;
    int       kk = 0;

    if(pathname==NULL || (pathname!=NULL && pathname[0]=='\0'))
        return NULL;

    while(pathname[jj] != '\0') 
    {
        memset(dirname, 0, sizeof(dirname));
        memset(dirsepstart, 0, sizeof(dirsepstart));

        // Skip dir separators
        for(int p=0; (pathname[jj] == '/') || (pathname[jj] == '\\'); p++, jj++)
            dirsepstart[p] = pathname[jj];

        /* 
        * We are at the start of a dir name. Copy it to a new entry in pathparts
        */

        // copy dir name from source (pathname) to the target (pathparts)
        kk=0;
        while(pathname[jj] != '\0' && 
            pathname[jj] != '/' && 
            pathname[jj] != '\\')
        {
            dirname[kk] = pathname[jj];
            jj++;
            kk++;
        }

        if(dirname[0] != '\0') 
        {
            // Copy dirname 
            pp[ii].dirname = (char*)calloc(MAX_DIRNAME_LEN, sizeof(char));
            strcpy(pp[ii].dirname, dirname);

            // Store the separators; first in front 
            pp[ii].dirsep = (char**)calloc(2, sizeof(char*));
            if(dirsepstart[0]!='\0')
            {
                pp[ii].dirsep[0] = (char*)calloc(MAX_DIRSEP_LEN, sizeof(char));
                sprintf(pp[ii].dirsep[0], "%s\0", dirsepstart);
            }
            else
                pp[ii].dirsep[0] = NULL;

            // Then at the end 
            if((pathname[jj]!='\0') && (pathname[jj]=='\\') || (pathname[jj]=='/'))
            {
                pp[ii].dirsep[1] = (char*)calloc(MAX_DIRSEP_LEN, sizeof(char));
                sprintf(pp[ii].dirsep[1], "%c\0", pathname[jj]);
            }
            else
                pp[ii].dirsep[1] = NULL;
        }

        ii++;
    }
    return pp;
}




/////////////////////////////////////////////////////////////////////////////////
char* buildpath(Pathparts* pp)
{
    char* pathname = NULL;
    int ii;
    int nparts;
    char dirsep[MAX_DIRSEP_LEN] = "/\0"; 

    if(pp==NULL)
        return pathname;

    pathname = (char*)calloc(200, sizeof(char));
    for(ii=0; pp[ii].dirname!=NULL; ii++)
    {
        if(pp[ii].dirsep==NULL)
        {
            pp[ii].dirsep = (char**)calloc(2, sizeof(char*));
            pp[ii].dirsep[0] = (char*)calloc(MAX_DIRSEP_LEN, sizeof(char));
            pp[ii].dirsep[1] = (char*)calloc(MAX_DIRSEP_LEN, sizeof(char));
            strcpy(pp[ii].dirsep[0], dirsep); 
            strcpy(pp[ii].dirsep[1], dirsep); 
        }
        else
        {
            if(pp[ii].dirsep[0]!=NULL) 
                strcpy(pp[ii].dirsep[0], dirsep); 
            if(pp[ii].dirsep[1]!=NULL) 
                strcpy(pp[ii].dirsep[1], dirsep); 
        }
    }
    nparts = ii;

#ifdef DEBUG
    for(ii=0; ii<nparts; ii++)
        printf("[%s, %s]\n", pp[ii].dirsep[0], pp[ii].dirsep[1]);
    printf("\n");
#endif 

    // Now set the output parameter pathname.
    pathname[0]='\0';
    for(int ii=0; ii<nparts; ii++)
    {
        if(pathname[0]=='\0')
        {
            if(pp[ii].dirsep[0] != NULL && pp[ii].dirsep[1] != NULL)
                sprintf(pathname, "%s%s%s", pp[ii].dirsep[0], pp[ii].dirname, pp[ii].dirsep[1]);
            else if(pp[ii].dirsep[0] != NULL && pp[ii].dirsep[1] == NULL)
                sprintf(pathname, "%s%s", pp[ii].dirsep[0], pp[ii].dirname);
            else if(pp[ii].dirsep[0] == NULL && pp[ii].dirsep[1] != NULL)
                sprintf(pathname, "%s%s", pp[ii].dirname, pp[ii].dirsep[1]);
            else if(pp[ii].dirsep[0] == NULL && pp[ii].dirsep[1] == NULL)
                sprintf(pathname, "%s",pp[ii].dirname);
        }
        else
        {
            if(pp[ii].dirsep[1] != NULL)
                sprintf(pathname, "%s%s%s", pathname, pp[ii].dirname, pp[ii].dirsep[1]);
            else
                sprintf(pathname, "%s%s", pathname, pp[ii].dirname);
        }
    }
    if( (ii>=0) && (pp[ii-1].dirsep[1] == NULL) )
        sprintf(pathname, "%s%s", pathname, dirsep);

    return pathname;
}




char* pathProcess(char* pathname0)
{       
    char* pathname;
    Pathparts* pp;

    pp = getpathparts(pathname0);
    if(pp==NULL)
    {
        printf("PathProcess Error: No user input: exiting...\n");
        return NULL;
    }

#ifdef DEBUG
    for(int ii=0; pp[ii].dirname != NULL; ii++)
        printf("%s\n",pp[ii].dirname);
    printf("\n");

    for(int ii=0; pp[ii].dirname != NULL; ii++)
        printf("[%s, %s]\n", pp[ii].dirsep[0], pp[ii].dirsep[1]);
    printf("\n");
#endif

    pathname = buildpath(pp);
    if(pathname==NULL)
    {
        printf("ERROR: Can't rebuild pathname: exiting...\n");
        return NULL;
    }
#ifdef DEBUG
    printf("pathname = %s\n\n", pathname);
#endif

    free(pp);

    return(pathname);
}



