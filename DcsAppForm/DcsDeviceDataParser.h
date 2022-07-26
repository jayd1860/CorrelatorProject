#pragma once


#include <cstring>

struct AnalogData
{
	AnalogData(int segSize, int nCh0)
	{
		nCh = nCh0;
		maxsize = segSize / 6;

		t = new unsigned long long[maxsize]();

		amp = new unsigned long long*[nCh];
		for(int ii=0; ii < nCh; ii++)
			amp[ii] = new unsigned long long[maxsize]();

		chIdxOrder = new int[nCh]();
		chIdxOrder[0] = 4;  // get data for ADC 1 from word 4
		chIdxOrder[1] = 2;  // get data for ADC 2 from word 2
		chIdxOrder[2] = 5;  // get data for ADC 3 from word 5
		chIdxOrder[3] = 3;  // get data for ADC 4 from word 3

		count = 0;
        countTotal = 0;
        countThreshold = maxsize < (int)1e6 ? maxsize : (int)1e6;

        macrotime = 0;
        macrotimePrev = 0;
	}

	int nCh;
	unsigned long long* t;
	unsigned long long** amp;
	unsigned long count;
	unsigned long countTotal;
	int* chIdxOrder;
	int maxsize;
    int countThreshold;
    unsigned long long   macrotime;
    unsigned long   macrotimePrev;

};



struct LightIntensityData
{
	LightIntensityData(int segSize, int nCh0)
	{
		nCh = nCh0;
		maxsize = segSize;

		arrtimes = new unsigned long long**[nCh]();
        eventtable = new unsigned char**[nCh];
        for(int ii=0; ii < nCh; ii++)
		{
            arrtimes[ii] = new unsigned long long*[2]();
            arrtimes[ii][0] = new unsigned long long[maxsize]();
            arrtimes[ii][1] = new unsigned long long[maxsize]();

            eventtable[ii] = new unsigned char*[4]();
            eventtable[ii][0] = new unsigned char[maxsize]();
            eventtable[ii][1] = new unsigned char[maxsize]();
            eventtable[ii][2] = new unsigned char[maxsize]();
            eventtable[ii][3] = new unsigned char[maxsize]();
		}
		countTotal = new unsigned long[nCh]();
		countTotalDropped = new unsigned long[nCh]();
		countThreshold = maxsize < (int)1e6 ? maxsize : (int)1e6;
	}

	int nCh;
    unsigned long long*** arrtimes;
    unsigned char*** eventtable;
    unsigned long* countTotal;
	unsigned long* countTotalDropped;
	unsigned long long countThreshold;
    unsigned long   macrotime;
	int maxsize;
};



class DcsDeviceDataParser
{
public:
	DcsDeviceDataParser(int segSize);
	~DcsDeviceDataParser();

	void parseData(unsigned short* dataraw, int n);
	void ClearBuffers();
    void ClearBuffersAll();

public:
	int nCh_dcs;
	int nCh_adc;
	unsigned long   maxsize;
    unsigned short* word;
	unsigned long long* wrapcount;
	LightIntensityData* dataDCS;
	AnalogData*         dataADC;
};


