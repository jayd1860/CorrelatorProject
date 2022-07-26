#pragma once

#include <windows.h>
#include <shlwapi.h> 
#include "Utils.h"

namespace DcsAppForm
{
using namespace System;
using namespace System::Windows::Forms;
	using namespace System::Threading;

	ref class MyFile
    {

    public:

        // --------------------------------------------------------------------------------------------
        MyFile()
        {
            m_hFile = INVALID_HANDLE_VALUE;

            m_MaxNameSize = 512;

            m_dirnameRoot = new wchar_t[m_MaxNameSize]();
            m_dirnameAcq = new wchar_t[m_MaxNameSize]();
            m_dirnameAcqFull = new wchar_t[m_MaxNameSize]();

            m_filenameBase = new wchar_t[m_MaxNameSize]();
            m_filenameExt = new wchar_t[15]();
            m_filenameBaseFull = new wchar_t[m_MaxNameSize]();
            m_filenameFull = new wchar_t[m_MaxNameSize]();
		
			_critSect = gcnew Object();
        }


        // --------------------------------------------------------------------------------------------
        MyFile(wchar_t* dirnameroot, wchar_t* dirname, wchar_t* filenameBase, wchar_t* ext)
        {
            m_hFile = INVALID_HANDLE_VALUE;

            m_MaxNameSize = 512;

            m_dirnameRoot = new wchar_t[m_MaxNameSize]();
            m_dirnameAcq = new wchar_t[m_MaxNameSize]();
            m_dirnameAcqFull = new wchar_t[m_MaxNameSize]();

            m_filenameBase = new wchar_t[m_MaxNameSize]();
            m_filenameExt = new wchar_t[15]();
            m_filenameBaseFull = new wchar_t[m_MaxNameSize]();
            m_filenameFull = new wchar_t[m_MaxNameSize]();


            wsprintf(m_dirnameRoot, dirnameroot);
            wsprintf(m_dirnameAcq, dirname);
            if(dirname[0] == '\0')
                wsprintf(m_dirnameAcqFull, L"%s", m_dirnameRoot);
            else
                wsprintf(m_dirnameAcqFull, L"%s\\%s", m_dirnameRoot, m_dirnameAcq);

            wsprintf(m_filenameBase, filenameBase);
            wsprintf(m_filenameExt, ext);
            wsprintf(m_filenameBaseFull, L"%s.%s", m_filenameBase, m_filenameExt);
            wsprintf(m_filenameFull, L"%s\\%s", m_dirnameAcqFull, m_filenameBaseFull);

			_critSect = gcnew Object();
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

			// Is this operation meant for discard data or file write?
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
			return 0;
		}
		
		
		// --------------------------------------------------------------------------------------------
        HANDLE OpenForReading()
        {
            wchar_t* errmsg = NULL;

            if(!IsValid())
            {
                if(!PathFileExists(m_filenameFull))
                {
                    // Lets display the windows error code from the failed file creation operation.
                    errmsg = new wchar_t[m_MaxNameSize];
                    wsprintf(errmsg, L"File doesn't exist.", m_filenameFull);
                }

                if(errmsg == NULL) {
                    m_hFile = CreateFile(m_filenameFull, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                    if(m_hFile == INVALID_HANDLE_VALUE)
                    {
                        // Lets display the windows error code from the failed file creation operation.
                        errmsg = new wchar_t[m_MaxNameSize];
                        wsprintf(errmsg, L"File failed to open");
                    }
                }
            }

            // Display error message if there was a problem 
            if(errmsg != NULL) {
                String^ s = gcnew String(errmsg);
                MessageBox::Show(s);
                delete errmsg;
                return NULL;
            }
            else
                return m_hFile;
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
        inline wchar_t* GetFilenameBaseFull()
        {
            return m_filenameBaseFull;
        }



        // --------------------------------------------------------------------------------------------
        inline void SetFilenameBaseFull(wchar_t* filename)
        {
            memset(m_filenameBase, 0, 512 * sizeof(wchar_t));
            wsprintf(m_filenameBaseFull, filename);
            wsprintf(m_filenameFull, L"%s\\%s", m_dirnameAcqFull, m_filenameBaseFull);
        }



		// --------------------------------------------------------------------------------------------
		inline wchar_t* GetFilenameFull()
		{
			return m_filenameFull;
		}



		// --------------------------------------------------------------------------------------------
		inline void SetFilenameFull(wchar_t* filename)
		{
			memset(m_filenameBase, 0, m_MaxNameSize * sizeof(wchar_t));
			memset(m_filenameBaseFull, 0, m_MaxNameSize * sizeof(wchar_t));
			memset(m_filenameFull, 0, m_MaxNameSize * sizeof(wchar_t));

			wsprintf(m_filenameBaseFull, filename);
			wsprintf(m_filenameFull, L"%s\\%s", m_dirnameAcqFull, m_filenameBaseFull);
		}



		// --------------------------------------------------------------------------------------------
		inline wchar_t* GetFilenameBase()
		{
			return m_filenameBase;
		}



		// --------------------------------------------------------------------------------------------
		inline void SetFilenameBase(wchar_t* filename)
		{
			memset(m_filenameBase, 0, m_MaxNameSize * sizeof(wchar_t));
			memset(m_filenameBaseFull, 0, m_MaxNameSize * sizeof(wchar_t));
			memset(m_filenameFull, 0, m_MaxNameSize * sizeof(wchar_t));

			wsprintf(m_filenameBase, filename);
			wsprintf(m_filenameBaseFull, L"%s.%s", m_filenameBase, m_filenameExt);
			wsprintf(m_filenameFull, L"%s\\%s", m_dirnameAcqFull, m_filenameBaseFull);
		}



		// --------------------------------------------------------------------------------------------
        inline wchar_t* GetDirnameAcqFull()
        {
            return m_dirnameAcqFull;
        }



        // --------------------------------------------------------------------------------------------
        inline void SetDirnameAcqFull(wchar_t* dirname)
        {
            memset(m_dirnameRoot, 0, m_MaxNameSize * sizeof(wchar_t));
            memset(m_dirnameAcq, 0, m_MaxNameSize * sizeof(wchar_t));
			memset(m_dirnameAcqFull, 0, m_MaxNameSize * sizeof(wchar_t));

            wsprintf(m_dirnameAcqFull, dirname);
            wsprintf(m_filenameFull, L"%s\\%s", m_dirnameAcqFull, m_filenameBaseFull);
        }



		// --------------------------------------------------------------------------------------------
		inline int GetMaxNameSize()
		{
			return m_MaxNameSize;
		}


		
		// --------------------------------------------------------------------------------------------
        void Close()
        {
			Monitor::Enter(_critSect);
			if(m_hFile != INVALID_HANDLE_VALUE)  {
				CloseHandle(m_hFile);
				m_hFile = INVALID_HANDLE_VALUE;
			}
			Monitor::Exit(_critSect);
        }


    private:

        HANDLE     m_hFile;

        wchar_t*   m_dirnameRoot;
        wchar_t*   m_dirnameAcq;
        wchar_t*   m_dirnameAcqFull;

        wchar_t*   m_filenameBase;
        wchar_t*   m_filenameExt;
        wchar_t*   m_filenameBaseFull;
        wchar_t*   m_filenameFull;

        int        m_MaxNameSize;
		Object^    _critSect;
    };
}