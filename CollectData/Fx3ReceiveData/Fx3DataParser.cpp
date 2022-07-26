#include "stdafx.h"
#include "Fx3DataParser.h"
#include <math.h>


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//% Definitions common to DCS and ADC
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#define SRCMASK    0xf800
#define PKTCNTMASK 0x0700
#define EVENTMASK  0x00f8
#define MACROMASK1 0x0007
#define MACROMASK2 0x3fff
#define MICROMASK  0x7fff
#define MACRO_COUNTER_SIZE  0x1ffff

//%%%%%%%%%%%%%%%%%%%%%%%%%%
//% DCS definitions
//%%%%%%%%%%%%%%%%%%%%%%%%%%
#define MACROOVERFLOWMASK    0x0080
#define STARTMASK            0x0040
#define VALIDSTARTMASK       0x0020
#define VALIDSTOPMASK        0x0010
#define VALIDCONVERSIONMASK  0x0008


// ---------------------------------------------------------------------------
Fx3DataParser::Fx3DataParser(int segSize)
{
	nCh_dcs = 4;
	nCh_adc = 4;
	int nCh_tot = nCh_dcs+1;

	macrotimeDCS = 0;

	// %%%%%%%%%%%%%%%%%%%%%%%%%
	//% ADC definitions
	//%%%%%%%%%%%%%%%%%%%%%%%%%
	macrotimeADC = 0;
	macrotimeADCPrev = 0;

	dataDCS = new LightIntensityData(segSize, nCh_dcs);
	dataADC = new AnalogData(segSize, nCh_adc);

	word = new unsigned short[6];

	wrapcount = new unsigned long long[nCh_tot];
	memset(wrapcount, 0, nCh_tot * sizeof(unsigned long long));
}



// ---------------------------------------------------------------------------
Fx3DataParser::~Fx3DataParser()
{

}




// ---------------------------------------------------------------------------
void Fx3DataParser::ClearBuffers()
{
	int nCh_tot = dataDCS->nCh+1;

	for(int ii=0; ii<dataDCS->nCh; ii++)
	{
		memset(dataDCS->arrtimes[ii][0], 0, sizeof(unsigned long long) * dataDCS->maxsize);
		memset(dataDCS->arrtimes[ii][1], 0, sizeof(unsigned long long) * dataDCS->maxsize);
		dataDCS->countTotal[ii] = 0;
		dataDCS->countTotalDropped[ii] = 0;
	}

	memset(dataADC->t, 0, sizeof(unsigned long long) * dataADC->maxsize);
	for(int ii=0; ii<dataADC->nCh; ii++)
		memset(dataADC->amp[ii], 0, sizeof(unsigned long long) * dataADC->maxsize);
	dataADC->count = 0;
	dataADC->countTotal = 0;

	macrotimeDCS = 0;
	macrotimeADC = 0;
	macrotimeADCPrev = 0;

	memset(wrapcount, 0, nCh_tot * sizeof(unsigned long long));
}


// ---------------------------------------------------------------------------
void Fx3DataParser::parseData(unsigned short* dataraw, int n)
{
	unsigned short src;
	int kk = 0;

	nCh_dcs = dataDCS->nCh;
	nCh_adc = dataADC->nCh;

	unsigned long* photcount = dataDCS->countTotal;
	unsigned long* photcountDropped = dataDCS->countTotalDropped;
	unsigned long adccount = 0;

	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	//% Main data loop for parsing a chunk
	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	while(kk < n)
	{
		word = &(dataraw[kk]);

		// % src = bitshift(bitand(word(1), SRCMASK), -11);
		// % is a lot slower than this:
		src = (word[0] & SRCMASK) >> 11;

		if((src < 4) && (photcount[src] < dataDCS->countThreshold))
		{
			//% %%% Packet from DCS
			if((kk + 1) > n)
				break;

			//% validconversion tells us if the packet is fast DCS or
			//% time domain DCS
			if((word[0] & VALIDCONVERSIONMASK) == VALIDCONVERSIONMASK)
			{
				if (kk + 2 > n)
					break;
			}

			//% Get macro count
			macrotimeDCS = ((word[0] & MACROMASK1) << 14) | (word[1] & MACROMASK2);

			//% Check for counter overflow event
			if(((word[0] & MACROOVERFLOWMASK) == MACROOVERFLOWMASK) & (macrotimeDCS == 0))
				wrapcount[src] = wrapcount[src] + 1;

			//% Check for photon arrival event
			if((word[0] & STARTMASK) == STARTMASK)
			{
				// If we are overrunning the buffer then discard photons
				if(photcount[src] < dataDCS->maxsize)
				{
					//% Get macro count
					dataDCS->arrtimes[src][0][photcount[src]] = (MACRO_COUNTER_SIZE * wrapcount[src] + wrapcount[src]) + macrotimeDCS;

					//% Get micro count
					if((word[0] & VALIDCONVERSIONMASK) == VALIDCONVERSIONMASK)
						dataDCS->arrtimes[src][1][photcount[src]] = word[3] & MICROMASK;
				}

				//% Increment number photon packets received
				photcount[src] = photcount[src] + 1;
			}

			//% Skip to next packet
			if ((word[0] & VALIDCONVERSIONMASK) == VALIDCONVERSIONMASK)
				kk = kk + 3;
			else if ((word[0] & VALIDCONVERSIONMASK) == 0)
				kk = kk + 2;
		}
		else if(src < 4)
		{
			if ((word[0] & VALIDCONVERSIONMASK) == VALIDCONVERSIONMASK)
			{
				if (kk + 2 > n)
					break;
			}

			if ((word[0] & STARTMASK) == STARTMASK)
			{
				//% Increment number photon packets discarded
				photcountDropped[src] = photcountDropped[src] + 1;
			}

			//% Skip to next packet
			if ((word[0] & VALIDCONVERSIONMASK) == VALIDCONVERSIONMASK)
				kk = kk + 3;
			else if ((word[0] & VALIDCONVERSIONMASK) == 0)
				kk = kk + 2;
		}
		else if(src == 4)
		{
			//%%%% Packet from ADC
			if(kk + 6 > n)
				break;

			if((dataraw[kk+1] & 0x4000) == 0)
			{
				kk=kk+1;
				continue;
			}

			macrotimeADCPrev = macrotimeADC;

			//% Get macro count
			macrotimeADC = ((word[0] & MACROMASK1) << 14) | (word[1] & MACROMASK2);
			if (macrotimeADC < macrotimeADCPrev)
				wrapcount[src] = wrapcount[src] + 1;

			dataADC->t[adccount] = (unsigned long long)((unsigned long long)MACRO_COUNTER_SIZE * wrapcount[src] + wrapcount[src]) + macrotimeADC;

			//% Get ADC data and increment number ADC packets received
			for (int hh=0; hh<nCh_adc; hh++)
				dataADC->amp[hh][adccount] = word[dataADC->chIdxOrder[hh]];

			//% Skip to next packet
			kk = kk + 6;

			adccount = adccount + 1;
		}
		else
		{
			//	%%%% Unknown packet type

			//	% Skip to next packet
			kk = kk + 1;
		}
	}

	dataADC->count = adccount;
	dataADC->countTotal = dataADC->countTotal + adccount;
}


