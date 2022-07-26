
#include <windows.h>
#include <stdio.h>
#include <Shlobj.h>
#include <time.h>
#include <stdint.h>
#include <Utils.h>
#include <vcclr.h>  

static uint8_t wordbits[65536] = {};

#if defined(_WIN32)
static SYSTEMTIME convertStringToSystemTime(char *dateTimeString)
{
    SYSTEMTIME systime;

    memset(&systime, 0, sizeof(systime));

    // Date string should be "yyyy-mo-dd hh:mm:ss"
    sscanf_s(dateTimeString, "%d:%d:%d:%d:%d:%d", 
             (int*)&systime.wYear, 
             (int*)&systime.wMonth,
             (int*)&systime.wDay,
             (int*)&systime.wHour,
             (int*)&systime.wMinute,
             (int*)&systime.wSecond);
    return systime;
}

#endif




///////////////////////////////////////////////////////////////////
unsigned long long getCurrTime(char* timeref)
{
    unsigned long long  msec;
    unsigned long long  msecRef=0;

    // We need a common reference time (that is common to all processes)
    // against which to measure time and we need a time resolution that is great 
	// than a second. Here are the reasons that common time routines don't work 
	// for us:

	// clock(): returns time since process start. This doesn't work for us because 
	// any 2 processes will then get different current time values at any one moment. 

	// GetTickCount(): returns time since system start, any 2 process will get
    // same value at any one moment but since the return value is long which is 
	// a 32-bit integer on windows (even 64-bit windows) it'll overflow in about 49 days. 
	// If windows is running continuously for longer than that, we need to add code to 
	// handle the overflow which is needlessly complicated. 

#if defined(_WIN32)

	SYSTEMTIME          stRef;
    FILETIME            ftRef;
	SYSTEMTIME          stCurr;
    FILETIME            ftCurr;

	GetLocalTime(&stCurr);
    SystemTimeToFileTime(&stCurr, &ftCurr);
	msec = ftCurr.dwHighDateTime;
    msec <<= 32;
    msec |= ftCurr.dwLowDateTime;
	msec /= 1e4;
	if((timeref != NULL) && (timeref[0] != 0))
	{
		stRef = convertStringToSystemTime(timeref);
		SystemTimeToFileTime(&stRef, &ftRef);
		msecRef = ftRef.dwHighDateTime;
		msecRef <<= 32;
		msecRef |= ftRef.dwLowDateTime;
		msecRef /= 1e4;
		if(msecRef < msec)
			msec -= msecRef;
	}

#elif defined(_LINUX_)

    struct timespec tp;

    clock_gettime(CLOCK_MONOTONIC, &tp);
	msec = tp.tv_sec*1e3;
    msec = msec + (int)(tp.tv_nsec/1e6);

#endif

    return msec;
}



////////////////////////////////////////////////////////////////////
double getElapsedTime(unsigned long long starttime, int unit)
{
	unsigned long long  currtime;

#if defined(_WIN32)

	SYSTEMTIME          stCurr;
    FILETIME            ftCurr;

	GetLocalTime(&stCurr);
    SystemTimeToFileTime(&stCurr, &ftCurr);
	currtime = ftCurr.dwHighDateTime;
    currtime <<= 32;
    currtime |= ftCurr.dwLowDateTime;
	currtime /= 1e4;

#elif defined(_LINUX_)

    struct timespec tp;

    clock_gettime(CLOCK_MONOTONIC, &tp);
	currtime = tp.tv_sec*1e3;
    currtime = currtime + (int)(tp.tv_nsec/1e6);

#endif

	return (double)((currtime - starttime) / (double)unit);
}




// -------------------------------------------------------------------
void getCurrDateTimeString(wchar_t* datetimestr)
{
    time_t rawtime;
    time(&rawtime);
    struct tm* timeinfo = localtime(&rawtime);

    char yyyy[4];
    char yy[4];
    char mo[4];
    char dd[4];
    char hh[4];
    char mm[4];
    char ss[4];

    char datetime_temp[20];

    memset(datetime_temp, 0, sizeof(datetime_temp));

    memset(yyyy, 0, sizeof(yyyy));
    memset(yy, 0, sizeof(yy));
    memset(mo, 0, sizeof(mo));
    memset(dd, 0, sizeof(dd));
    memset(hh, 0, sizeof(hh));
    memset(mm, 0, sizeof(mm));
    memset(ss, 0, sizeof(ss));


    timeinfo->tm_year = timeinfo->tm_year+1900;

    _itoa(timeinfo->tm_year, yyyy, 10);
    if(timeinfo->tm_year>9)
        _itoa(timeinfo->tm_year%2000, yy, 10);
    else {
        yy[0] = '0';
        _itoa(timeinfo->tm_year%2000, &(yy[1]), 10);
    }

    if(timeinfo->tm_mon+1>9)
        _itoa(timeinfo->tm_mon+1,  mo, 10);
    else {
        mo[0] = '0';
        _itoa(timeinfo->tm_mon+1,  &(mo[1]), 10);
    }

    if(timeinfo->tm_mday>9)
        _itoa(timeinfo->tm_mday, dd, 10);
    else {
        dd[0] = '0';
        _itoa(timeinfo->tm_mday, &(dd[1]), 10);
    }

    if(timeinfo->tm_hour>9)
        _itoa(timeinfo->tm_hour, hh, 10);
    else {
        hh[0] = '0';
        _itoa(timeinfo->tm_hour, &(hh[1]), 10);
    }

    if(timeinfo->tm_min>9)
        _itoa(timeinfo->tm_min,  mm, 10);
    else {
        mm[0] = '0';
        _itoa(timeinfo->tm_min,  &(mm[1]), 10);
    }

    if(timeinfo->tm_sec>9)
        _itoa(timeinfo->tm_sec,  ss, 10);
    else {
        ss[0] = '0';
        _itoa(timeinfo->tm_sec,  &(ss[1]), 10);
    }

    sprintf(datetime_temp, "%s%s%s_%s%s%s", yy, mo, dd, hh, mm, ss);
    for(int ii=0; ii<11; ii++)
        datetimestr[ii] = datetime_temp[ii];
}





// -------------------------------------------------------------------
void getCurrDateTime(struct tm* timeinfo, unsigned short* datetime)
{
    time_t rawtime;
    time(&rawtime);
    timeinfo = localtime(&rawtime);

    timeinfo->tm_year += 1900;

    datetime[0] = timeinfo->tm_year;
    datetime[1] = timeinfo->tm_mon+1;
    datetime[2] = timeinfo->tm_mday;
    datetime[3] = timeinfo->tm_hour;
    datetime[4] = timeinfo->tm_min;
    datetime[5] = timeinfo->tm_sec;
}





// --------------------------------------------------------------------------------------------
int browseForFolder(wchar_t* rootdir, wchar_t* szPath)
{
	// Got this most of this code online from 
	// https://www.experts-exchange.com/articles/1600/Browse-for-Folder-Advanced-Options.html
	//

	PIDLIST_ABSOLUTE pidlRoot;
	HRESULT hR = SHParseDisplayName(rootdir, 0, &pidlRoot, 0, 0);
	BROWSEINFO bi = {0};

	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
	bi.pidlRoot = pidlRoot;
	bi.lpszTitle = L"Browse for Folder";

	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
	if(pidl == NULL) {
		return -1;
	}
	BOOL fRet = SHGetPathFromIDList(pidl, szPath);
	CoTaskMemFree(pidl);

	return 0;
}



// ----------------------------------------------------------------------
void convertCharToWchar(wchar_t* dst, char* src, int len)
{
    for(int ii=0; ii<len; ii++)
        dst[ii] = src[ii];
}



// ----------------------------------------------------------------------
void convertWcharToChar(char* dst, wchar_t* src, int len)
{
    for(int ii=0; ii<len; ii++)
        dst[ii] = src[ii];
}




//Optionally, the wordbits[] table could be filled using this function
int numbitson_init(void)
{
	uint32_t i;
	uint16_t x;
	int count;
	for(i=0; i <= 0xFFFF; i++)
	{
		x = i;
		for(count=0; x; count++) // borrowed from popcount64d() above
			x &= x - 1;
		wordbits[i] = count;
	}
	return count;
}


// ----------------------------------------------------------------------
int numbitson(uint32_t x)
{
	int r = wordbits[x & 0xFFFF] + wordbits[x >> 16];
	return r;
}




// -----------------------------------------------------------------------
int convertStringToChar(String^ src, char* dst)
{
    pin_ptr<const wchar_t> str = PtrToStringChars(src);  

    size_t convertedChars = 0;  
    size_t  sizeInBytes = ((src->Length + 1) * 2);  
    errno_t err = 0;  

    err = wcstombs_s(&convertedChars, dst, sizeInBytes, str, sizeInBytes);

    return 0;
}



// -----------------------------------------------------------------------
int convertStringToWchar(String^ src, wchar_t* dst)
{
    char* dst0;

    pin_ptr<const wchar_t> str = PtrToStringChars(src);  

    size_t convertedChars = 0;  
    size_t  sizeInBytes = ((src->Length + 1) * 2);  
    errno_t err = 0;  

    dst0 = new char[sizeInBytes+1]();

    err = wcstombs_s(&convertedChars, dst0, sizeInBytes, str, sizeInBytes);

    convertCharToWchar(dst, dst0, sizeInBytes);

    return 0;
}



// -----------------------------------------------------------------------
int strlen_u(unsigned char* s)
{
    int len;
    for(len=0; s[len] != 0; len++)
        ;
    return len;
}




// -----------------------------------------------------------------------
void getFileExtension(wchar_t* filename, wchar_t* ext)
{
    int kk=0;
    int ilastdot=0;

    // Find last '.'
    for(int ii=0; filename[ii] != '\0'; ii++)
    {
        if(filename[ii] == '.')
            ilastdot = ii;
    }

    // Copy extension to output variable ext
    if(ilastdot>0) 
    {
        for(int ii=ilastdot+1; filename[ii] != '\0'; ii++, kk++)
            ext[kk] = filename[ii];
    }
}