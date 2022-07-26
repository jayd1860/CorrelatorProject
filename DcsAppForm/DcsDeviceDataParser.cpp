#include <math.h>
#include <stdio.h>
#include "DcsDeviceDataParser.h"

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
DcsDeviceDataParser::DcsDeviceDataParser(int segSize)
{
    nCh_dcs = 4;
    nCh_adc = 4;
    int nCh_tot = nCh_dcs+1;

    // %%%%%%%%%%%%%%%%%%%%%%%%%
    //% ADC definitions
    //%%%%%%%%%%%%%%%%%%%%%%%%%
    dataDCS = new LightIntensityData(segSize, nCh_dcs);
    dataADC = new AnalogData(segSize, nCh_adc);

    word = new unsigned short[6];

    wrapcount = new unsigned long long[nCh_tot];
    memset(wrapcount, 0, nCh_tot * sizeof(unsigned long long));
}



// ---------------------------------------------------------------------------
DcsDeviceDataParser::~DcsDeviceDataParser()
{

}




// ---------------------------------------------------------------------------
void DcsDeviceDataParser::ClearBuffersAll()
{
    int nCh_tot = dataDCS->nCh+1;

    ClearBuffers();
    memset(wrapcount, 0, nCh_tot * sizeof(unsigned long long));
}


// ---------------------------------------------------------------------------
void DcsDeviceDataParser::ClearBuffers()
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
}


// ---------------------------------------------------------------------------
void DcsDeviceDataParser::parseData(unsigned short* dataraw, int n)
{
    unsigned short src;
    int kk = 0;

    nCh_dcs = dataDCS->nCh;
    nCh_adc = dataADC->nCh;

    unsigned long* pktcount = dataDCS->countTotal;
    unsigned long* pktcountDropped = dataDCS->countTotalDropped;
    unsigned long* adccount = &dataADC->countTotal;

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    //% Main data loop for parsing a chunk
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    while(kk < n)
    {
        word = &(dataraw[kk]);

        // % src = bitshift(bitand(word(1), SRCMASK), -11);
        // % is a lot slower than this:
        src = (word[0] & SRCMASK) >> 11;

        if((src < 4) && (pktcount[src] < dataDCS->countThreshold))
        {
            //% %%% Packet from DCS
            if((kk + 2) > n)
                break;

            //% Get macro count
            dataDCS->macrotime = ((word[0] & MACROMASK1) << 14) | (word[1] & MACROMASK2);

            //% Check for counter overflow event
            if(((word[0] & MACROOVERFLOWMASK) == MACROOVERFLOWMASK) & (dataDCS->macrotime == 0))
                wrapcount[src] = wrapcount[src] + 1;

            //% Check for packet START event
            if((word[0] & STARTMASK) == STARTMASK)
            {
                //% Get macro count
                dataDCS->arrtimes[src][0][pktcount[src]] = (MACRO_COUNTER_SIZE * wrapcount[src] + wrapcount[src]) + dataDCS->macrotime;
                dataDCS->eventtable[src][0][pktcount[src]] = 1;

                //% Increment number photon packets received
                pktcount[src] = pktcount[src] + 1;
            }

            //% Check for packet VALIDSTARTMASK event
            if((word[0] & VALIDSTARTMASK) == VALIDSTARTMASK)
            {
                //% Get macro count
                dataDCS->arrtimes[src][0][pktcount[src]] = (MACRO_COUNTER_SIZE * wrapcount[src] + wrapcount[src]) + dataDCS->macrotime;
                dataDCS->eventtable[src][1][pktcount[src]] = 1;

                //% Increment number photon packets received
                pktcount[src] = pktcount[src] + 1;
            }

            //% Check for packet VALIDSTOPMASK event
            if((word[0] & VALIDSTOPMASK) == VALIDSTOPMASK)
            {
                //% Get macro count
                dataDCS->arrtimes[src][0][pktcount[src]] = (MACRO_COUNTER_SIZE * wrapcount[src] + wrapcount[src]) + dataDCS->macrotime;
                dataDCS->eventtable[src][2][pktcount[src]] = 1;

                //% Increment number photon packets received
                pktcount[src] = pktcount[src] + 1;
            }

            //% Check for packet VALIDSTOPMASK event
            //% validconversion tells us if the packet is fast DCS or
            //% time domain DCS
            if((word[0] & VALIDCONVERSIONMASK) == VALIDCONVERSIONMASK)
            {
                //% Get macro count
                dataDCS->arrtimes[src][0][pktcount[src]] = (MACRO_COUNTER_SIZE * wrapcount[src] + wrapcount[src]) + dataDCS->macrotime;
                dataDCS->arrtimes[src][1][pktcount[src]] = word[2] & MICROMASK;
                dataDCS->eventtable[src][3][pktcount[src]] = 1;

                //% Increment number photon packets received
                pktcount[src] = pktcount[src] + 1;

                //% Skip to next packet
                kk = kk + 3;
            }
            else
            {
                //% Skip to next packet
                kk = kk + 2;
            }
        }
        else if(src < 4)
        {
            if((word[0] & VALIDCONVERSIONMASK) == VALIDCONVERSIONMASK)
            {
                if(kk + 2 > n)
                    break;
            }

            if((word[0] & STARTMASK) == STARTMASK)
            {
                //% Increment number photon packets discarded
                pktcountDropped[src] = pktcountDropped[src] + 1;
            }

            //% Skip to next packet
            if((word[0] & VALIDCONVERSIONMASK) == VALIDCONVERSIONMASK)
                kk = kk + 3;
            else if((word[0] & VALIDCONVERSIONMASK) == 0)
                kk = kk + 2;
        }
        else if((src == 4) && (*adccount < dataADC->countThreshold))
        {
            //%%%% Packet from ADC
            if(kk + 6 > n)
                break;

            if((dataraw[kk+1] & 0x4000) == 0)
            {
                kk=kk+1;
                continue;
            }

            dataADC->macrotimePrev = dataADC->macrotime;

            //% Get macro count
            dataADC->macrotime = ((word[0] & MACROMASK1) << 14) | (word[1] & MACROMASK2);
            if(dataADC->macrotime < dataADC->macrotimePrev)
                wrapcount[src] = wrapcount[src] + 1;

            dataADC->t[*adccount] = (unsigned long long)((unsigned long long)MACRO_COUNTER_SIZE * wrapcount[src] + wrapcount[src]) + dataADC->macrotime;

            //% Get ADC data and increment number ADC packets received
            for(int hh=0; hh<nCh_adc; hh++)
                dataADC->amp[hh][*adccount] = word[dataADC->chIdxOrder[hh]];

            //% Skip to next packet
            kk = kk + 6;

            *adccount = *adccount + 1;
        }
        else
        {
            //	%%%% Unknown packet type

            //	% Skip to next packet
            kk = kk + 1;
        }
    }
}


