#pragma once

#include <windows.h>
#include <shlwapi.h> 


//////////////////////////////////////////////////////////////////////
class MyFile
{

public:

    // --------------------------------------------------------------------------------------------
    MyFile()
    {
        m_MaxNameSize = 512;

        m_dirnameRoot = new char[m_MaxNameSize]();
        m_dirnameAcq = new char[m_MaxNameSize]();
        m_dirnameAcqFull = new char[m_MaxNameSize]();

        m_filenameBase = new char[m_MaxNameSize]();
        m_filenameExt = new char[15]();
        m_filenameBaseFull = new char[m_MaxNameSize]();
        m_filenameFull = new char[m_MaxNameSize]();
    }


    // --------------------------------------------------------------------------------------------
    MyFile(char* dirnameroot, char* dirname, char* filenameBase, char* ext)
    {
        m_MaxNameSize = 512;

        m_dirnameRoot = new char[m_MaxNameSize]();
        m_dirnameAcq = new char[m_MaxNameSize]();
        m_dirnameAcqFull = new char[m_MaxNameSize]();

        m_filenameBase = new char[m_MaxNameSize]();
        m_filenameExt = new char[15]();
        m_filenameBaseFull = new char[m_MaxNameSize]();
        m_filenameFull = new char[m_MaxNameSize]();


        sprintf_s(m_dirnameRoot, m_MaxNameSize, "%s", dirnameroot);
        sprintf_s(m_dirnameAcq, m_MaxNameSize, "%s", dirname);
        if(dirname[0] == '\0')
            sprintf_s(m_dirnameAcqFull, m_MaxNameSize, "%s", m_dirnameRoot);
        else
            sprintf_s(m_dirnameAcqFull, m_MaxNameSize, "%s\\%s", m_dirnameRoot, m_dirnameAcq);

        sprintf_s(m_filenameBase, m_MaxNameSize, "%s", filenameBase);
        sprintf_s(m_filenameExt, 15, "%s", ext);
        sprintf_s(m_filenameBaseFull, m_MaxNameSize, "%s.%s", m_filenameBase, m_filenameExt);
        sprintf_s(m_filenameFull, m_MaxNameSize, "%s\\%s", m_dirnameAcqFull, m_filenameBaseFull);
    }



    // --------------------------------------------------------------------------------------------
    MyFile(char* dirname, char* filenameBase, char* ext)
    {
        m_MaxNameSize = 512;

        m_dirnameRoot = new char[m_MaxNameSize]();
        m_dirnameAcq = new char[m_MaxNameSize]();
        m_dirnameAcqFull = new char[m_MaxNameSize]();

        m_filenameBase = new char[m_MaxNameSize]();
        m_filenameExt = new char[15]();
        m_filenameBaseFull = new char[m_MaxNameSize]();
        m_filenameFull = new char[m_MaxNameSize]();


        sprintf_s(m_dirnameAcq, m_MaxNameSize, "%s", dirname);
        sprintf_s(m_dirnameAcqFull, m_MaxNameSize, "%s", m_dirnameAcq);

        sprintf_s(m_filenameBase, m_MaxNameSize, "%s", filenameBase);
        sprintf_s(m_filenameExt, 15, "%s", ext);
        sprintf_s(m_filenameBaseFull, m_MaxNameSize, "%s.%s", m_filenameBase, m_filenameExt);
        sprintf_s(m_filenameFull, m_MaxNameSize, "%s\\%s", m_dirnameAcqFull, m_filenameBaseFull);
    }



    // --------------------------------------------------------------------------------------------
    MyFile(char* dirname, char* filename)
    {
        m_MaxNameSize = 512;

        m_dirnameRoot = new char[m_MaxNameSize]();
        m_dirnameAcq = new char[m_MaxNameSize]();
        m_dirnameAcqFull = new char[m_MaxNameSize]();

        m_filenameBase = new char[m_MaxNameSize]();
        m_filenameExt = new char[15]();
        m_filenameBaseFull = new char[m_MaxNameSize]();
        m_filenameFull = new char[m_MaxNameSize]();


        sprintf_s(m_dirnameAcq, m_MaxNameSize, "%s", dirname);
        sprintf_s(m_dirnameAcqFull, m_MaxNameSize, "%s", m_dirnameAcq);

        sprintf_s(m_filenameBase, m_MaxNameSize, "%s", filename);
        sprintf_s(m_filenameBaseFull, m_MaxNameSize, "%s", m_filenameBase);
        sprintf_s(m_filenameFull, m_MaxNameSize, "%s\\%s", m_dirnameAcqFull, m_filenameBaseFull);
    }


    // --------------------------------------------------------------------------------------------
    ~MyFile()
    {
        delete m_filenameExt;
        delete m_dirnameRoot;
        delete m_dirnameAcq;
        delete m_dirnameAcqFull;

        delete m_filenameBase;
        delete m_filenameBaseFull;
        delete m_filenameFull;
    }


    // --------------------------------------------------------------------------------------------
    inline char* GetFilenameBaseFull()
    {
        return m_filenameBaseFull;
    }



    // --------------------------------------------------------------------------------------------
    inline void SetFilenameBaseFull(char* filename)
    {
        memset(m_filenameBase, 0, 512 * sizeof(char));
        sprintf_s(m_filenameBaseFull, m_MaxNameSize, "%s", filename);
        sprintf_s(m_filenameFull, m_MaxNameSize, "%s\\%s", m_dirnameAcqFull, m_filenameBaseFull);
    }



    // --------------------------------------------------------------------------------------------
    inline char* GetFilenameFull()
    {
        return m_filenameFull;
    }



    // --------------------------------------------------------------------------------------------
    inline void SetFilenameFull(char* filename)
    {
        memset(m_filenameBase, 0, m_MaxNameSize * sizeof(char));
        memset(m_filenameBaseFull, 0, m_MaxNameSize * sizeof(char));
        memset(m_filenameFull, 0, m_MaxNameSize * sizeof(char));

        sprintf_s(m_filenameBaseFull, m_MaxNameSize, "%s", filename);
        sprintf_s(m_filenameFull, m_MaxNameSize, "%s\\%s", m_dirnameAcqFull, m_filenameBaseFull);
    }



    // --------------------------------------------------------------------------------------------
    inline char* GetFilenameBase()
    {
        return m_filenameBase;
    }



    // --------------------------------------------------------------------------------------------
    inline void SetFilenameBase(char* filename)
    {
        memset(m_filenameBase, 0, m_MaxNameSize * sizeof(char));
        memset(m_filenameBaseFull, 0, m_MaxNameSize * sizeof(char));
        memset(m_filenameFull, 0, m_MaxNameSize * sizeof(char));

        sprintf_s(m_filenameBase, m_MaxNameSize, "%s", filename);
        sprintf_s(m_filenameBaseFull, m_MaxNameSize, "%s.%s", m_filenameBase, m_filenameExt);
        sprintf_s(m_filenameFull, m_MaxNameSize, "%s\\%s", m_dirnameAcqFull, m_filenameBaseFull);
    }



    // --------------------------------------------------------------------------------------------
    //virtual int ReadWords(unsigned short* buffer, int nwordstoread, int* nwordsread);


    // --------------------------------------------------------------------------------------------
   // virtual void Close();



    // --------------------------------------------------------------------------------------------
    inline char* GetDirnameAcqFull()
    {
        return m_dirnameAcqFull;
    }



    // --------------------------------------------------------------------------------------------
    inline void SetDirnameAcqFull(char* dirname)
    {
        memset(m_dirnameRoot, 0, m_MaxNameSize * sizeof(char));
        memset(m_dirnameAcq, 0, m_MaxNameSize * sizeof(char));
        memset(m_dirnameAcqFull, 0, m_MaxNameSize * sizeof(char));

        sprintf_s(m_dirnameAcqFull, m_MaxNameSize, "%s", dirname);
        sprintf_s(m_filenameFull, m_MaxNameSize, "%s\\%s", m_dirnameAcqFull, m_filenameBaseFull);
    }



    // --------------------------------------------------------------------------------------------
    inline int GetMaxNameSize()
    {
        return m_MaxNameSize;
    }


protected:

    char*   m_dirnameRoot;
    char*   m_dirnameAcq;
    char*   m_dirnameAcqFull;

    char*   m_filenameBase;
    char*   m_filenameExt;
    char*   m_filenameBaseFull;
    char*   m_filenameFull;

    int     m_MaxNameSize;
};




//////////////////////////////////////////////////////////////////////
class MyFileStandardIO : public MyFile
{

public:

    // --------------------------------------------------------------------------------------------
    MyFileStandardIO (char* dirname, char* filename) : MyFile(dirname, filename)
    {
        m_hFile = NULL;
    }



    // --------------------------------------------------------------------------------------------
    MyFileStandardIO (char* dirname, char* filename, char* ext) : MyFile(dirname, filename, ext)
    {
        m_hFile = NULL;
    }


    // --------------------------------------------------------------------------------------------
    ~MyFileStandardIO ()
    {
        Close();
    }



    // --------------------------------------------------------------------------------------------
    FILE* OpenForReading()
    {
        char* errmsg = NULL;

        if(!IsValid())
        {
            if(!PathFileExists(m_filenameFull))
            {
                // Lets display the windows error code from the failed file creation operation.
                errmsg = new char[m_MaxNameSize];
                sprintf_s(errmsg, m_MaxNameSize, "File %s doesn't exist.", m_filenameFull);
            }

            if(errmsg == NULL) {
                m_hFile = fopen(m_filenameFull, "rb");
                if(m_hFile == NULL)
                {
                    // Lets display the windows error code from the failed file creation operation.
                    errmsg = new char[m_MaxNameSize];
                    sprintf_s(errmsg, m_MaxNameSize, "%s", L"File failed to open");
                }
            }
        }

        // Display error message if there was a problem 
        if(errmsg != NULL) {
            // String^ s = gcnew String(errmsg);
            // MessageBox::Show(s);
            delete errmsg;
            return NULL;
        }
        else
            return m_hFile;
    }



    // --------------------------------------------------------------------------------------------
    FILE* OpenForWriting()
    {
        char* errmsg = NULL;

        if(!IsValid())
        {
            if(!PathFileExists(m_dirnameAcqFull))
            {
                // Lets display the windows error code from the failed file creation operation.
                errmsg = new char[m_MaxNameSize];
                sprintf_s(errmsg, m_MaxNameSize, "Folder %s used for saving doesn't exist.", m_dirnameAcqFull);
            }

            if(errmsg == NULL) {
                m_hFile = fopen(m_filenameFull, "wb");
                if(m_hFile == NULL)
                {
                    // Lets display the windows error code from the failed file creation operation.
                    errmsg = new char[m_MaxNameSize];
                    sprintf_s(errmsg, m_MaxNameSize, "%s", L"File failed to open");
                }
            }
        }

        // Display error message if there was a problem 
        if(errmsg != NULL) {
            // String^ s = gcnew String(errmsg);
            // MessageBox::Show(s);
            delete errmsg;
            return NULL;
        }
        else
            return m_hFile;
    }



    // --------------------------------------------------------------------------------------------
    inline int ReadWords(unsigned short* buffer, int nwordstoread, int* nwordsread)
    {
        (*nwordsread) = fread(buffer, sizeof(unsigned short), nwordstoread, m_hFile);
        return 0;
    }



    // --------------------------------------------------------------------------------------------
    inline int Seek(long offset, int origin)
    {
        fseek(m_hFile, offset, origin);
        return 0;
    }



    // --------------------------------------------------------------------------------------------
    inline int Write(void* buffer, int elemsize, int elemcount)
    {
        if(fwrite(buffer, elemsize, elemcount, m_hFile) < 0)
            return -1;
        return 0;
    }


    // --------------------------------------------------------------------------------------------
    long long GetFileSize(char* dname, char* fname)
    {
        long long filesize = 0;

        if(m_hFile != NULL)
        {
            fseek(m_hFile, 0L, SEEK_END);
            filesize = ftell(m_hFile);
            rewind (m_hFile);
            printf("File size is %d K bytes\n", filesize / 1000);
        }
        else
            printf("ERROR: file %s\\%s doesn't exist\n", dname, fname);

        return filesize;
    }




    // --------------------------------------------------------------------------------------------
    bool IsValid()
    {
        if(m_hFile == NULL)
            return false;
        return true;
    }


    // --------------------------------------------------------------------------------------------
    inline FILE* GetHandle()
    {
        return m_hFile;
    }


    // --------------------------------------------------------------------------------------------
    void Close()
    {
        if(m_hFile != NULL) {
            fclose(m_hFile);
            m_hFile == NULL;
        }
    }


private:

    FILE*   m_hFile;
};



//////////////////////////////////////////////////////////////////////
class MyFileWindowsIO : public MyFile
{

public:

    // --------------------------------------------------------------------------------------------
    MyFileWindowsIO(char* dirname, char* filename) : MyFile(dirname, filename)
    {
        m_hFile = INVALID_HANDLE_VALUE;
    }



    // --------------------------------------------------------------------------------------------
    MyFileWindowsIO(char* dirname, char* filename, char* ext) : MyFile(dirname, filename, ext)
    {
        m_hFile = INVALID_HANDLE_VALUE;
    }


    // --------------------------------------------------------------------------------------------
    int OpenForWriting()
    {
        // Check that filename is not empty when user selects to save
        if(m_filenameBaseFull[0]==0)
            return -1;

        // We are asked to run the data collection job in the file.

        // Sanity check on the file to dump data .
        // Is the file open already?, if so please close the file.
        if(m_hFile != INVALID_HANDLE_VALUE)
            CloseHandle(m_hFile);

        m_hFile = INVALID_HANDLE_VALUE;

        if(!m_filenameFull[0]==0)
        {
            if(!PathFileExists(m_dirnameAcqFull))
                CreateDirectory(m_dirnameAcqFull, NULL);

            m_hFile = CreateFile(m_filenameFull, (GENERIC_READ | GENERIC_WRITE), FILE_SHARE_READ, NULL,
                CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

            // Did file creation succeed?
            if(m_hFile == INVALID_HANDLE_VALUE)
                return -1;
        }
    }


    // --------------------------------------------------------------------------------------------
    HANDLE OpenForReading()
    {
        char* errmsg = NULL;

        if(!IsValid())
        {
            if(!PathFileExists(m_filenameFull))
            {
                // Lets display the windows error code from the failed file creation operation.
                errmsg = new char[m_MaxNameSize];
                sprintf_s(errmsg, m_MaxNameSize, "File doesn't exist.", m_filenameFull);
            }

            if(errmsg == NULL) {
                m_hFile = CreateFile(m_filenameFull, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                if(m_hFile == INVALID_HANDLE_VALUE)
                {
                    // Lets display the windows error code from the failed file creation operation.
                    errmsg = new char[m_MaxNameSize];
                    sprintf_s(errmsg, m_MaxNameSize, "%s", L"File failed to open");
                }
            }
        }

        // Display error message if there was a problem 
        if(errmsg != NULL) {
            // String^ s = gcnew String(errmsg);
            // MessageBox::Show(s);
            delete errmsg;
            return NULL;
        }
        else
            return m_hFile;
    }



    // --------------------------------------------------------------------------------------------
    inline int ReadWords(unsigned short* buffer, int nwordstoread, int* nwordsread)
    {
        if(ReadFile(m_hFile, buffer, nwordstoread * sizeof(unsigned short), (DWORD*)nwordsread, NULL) == 0)
            return -1;

        *nwordsread = *nwordsread/2;

        return 0;
    }



    // --------------------------------------------------------------------------------------------
    long long GetFileSize(char* dname, char* fname)
    {
        LARGE_INTEGER    out;
        long long        filesize = 0;

        if(m_hFile != INVALID_HANDLE_VALUE)
        {
            GetFileSizeEx(m_hFile, &out);
            filesize = out.QuadPart;
            printf("File size is %d K bytes\n", filesize / 1000);
        }
        else
            printf("ERROR: file %s\\%s doesn't exist\n", dname, fname);

        return filesize;
    }


    // --------------------------------------------------------------------------------------------
    bool IsValid()
    {
        if(m_hFile == INVALID_HANDLE_VALUE)
            return false;
        if(m_hFile == NULL)
            return false;
        return true;
    }


    // --------------------------------------------------------------------------------------------
    inline HANDLE GetHandle()
    {
        return m_hFile;
    }


    // --------------------------------------------------------------------------------------------
    void Close()
    {
        if(m_hFile != INVALID_HANDLE_VALUE) {
            CloseHandle(m_hFile);
            m_hFile = INVALID_HANDLE_VALUE;
        }
    }


private:

    HANDLE  m_hFile;
};


