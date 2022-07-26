
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>
#include "MyFile.h"
#include "Fx3DataParser.h"

using namespace std;


// -----------------------------------------------------------------
void dispCounts(Fx3DataParser* parser)
{
    unsigned long* pktcount = parser->dataDCS->countTotal;
    unsigned long adccount = parser->dataADC->countTotal;
    int kk;


    for(kk=0; kk<parser->dataDCS->nCh; kk++)
        printf("%d, ", pktcount[kk]);

    printf("%d, ", adccount);
    for(kk=0; kk<parser->dataDCS->nCh+1; kk++)
    {
        if(kk==parser->dataDCS->nCh)
            printf("%d", parser->wrapcount[kk]);
        else
            printf("%d, ", parser->wrapcount[kk]);
    }
}



// -----------------------------------------------------------------
int SaveFileSeg(MyFileStandardIO**  outputfile, 
                char* dname, 
                char* fname, 
                Fx3DataParser*  parser, 
                int nSeg, 
                unsigned short* datetime_start, 
                unsigned short* datetime_end)
{
    int kk;
    MyFileStandardIO* outputfile0 = *outputfile;

    // Allocate new file object and open it for writing, 
    // if the output file doesn't exist yet
    if(*outputfile==NULL)
    {
        *outputfile = new MyFileStandardIO(dname, fname);
        if((*outputfile)->OpenForWriting()==NULL)
        {
            printf("Error: Output file %s\\%s failed to open...\n", dname, fname);
            return -1;
        }
    }

    // Write start of measurement date and time to segment file
    (*outputfile)->Write(datetime_start, sizeof(unsigned short), 8);

    // Write number of segments and other header info for this segment
    (*outputfile)->Write(&nSeg, sizeof(int), 1);
    (*outputfile)->Write(&parser->dataDCS->nCh, sizeof(int), 1);
    (*outputfile)->Write(&parser->dataADC->nCh, sizeof(int), 1);
    (*outputfile)->Write(parser->dataDCS->countTotal, sizeof(unsigned long), parser->dataDCS->nCh);
    (*outputfile)->Write(&parser->dataADC->countTotal, sizeof(unsigned long), 1);
    (*outputfile)->Write(parser->dataADC->chIdxOrder, sizeof(int), parser->dataADC->nCh);

    // Write photon arrival times to file
    for(kk=0; kk<parser->dataDCS->nCh; kk++)
    {
        // Macrotime
        (*outputfile)->Write(parser->dataDCS->arrtimes[kk][0], sizeof(unsigned long long), parser->dataDCS->countTotal[kk]);

        // Microtime
        (*outputfile)->Write(parser->dataDCS->arrtimes[kk][1], sizeof(unsigned long long), parser->dataDCS->countTotal[kk]);
    }

    // Write photon event tables to file
    for(kk=0; kk<parser->dataDCS->nCh; kk++)
    {
        // START
        (*outputfile)->Write(parser->dataDCS->eventtable[kk][0], sizeof(unsigned char), parser->dataDCS->countTotal[kk]);

        // VALID START
        (*outputfile)->Write(parser->dataDCS->eventtable[kk][1], sizeof(unsigned char), parser->dataDCS->countTotal[kk]);

        // VALID STOP
        (*outputfile)->Write(parser->dataDCS->eventtable[kk][2], sizeof(unsigned char), parser->dataDCS->countTotal[kk]);

        // VALID CONVERSION
        (*outputfile)->Write(parser->dataDCS->eventtable[kk][3], sizeof(unsigned char), parser->dataDCS->countTotal[kk]);
    }

    // Write analog data to file
    (*outputfile)->Write(parser->dataADC->t, sizeof(unsigned long long), parser->dataADC->countTotal);
    for(kk=0; kk<parser->dataADC->nCh; kk++)
        (*outputfile)->Write(parser->dataADC->amp[kk], sizeof(unsigned long long), parser->dataADC->countTotal);


    // Write end of measurement date and time to segment file
    (*outputfile)->Write(datetime_start, sizeof(unsigned short), 8);


    // close and delete segment file object
    if(outputfile0 == NULL)
    {
        delete *outputfile;
        *outputfile = NULL;
    }

    return 0;
}


// -----------------------------------------------------------------
int main(int argc, char* argv[])
{
    int               segSize;
    int               chunkSize;
    long long         filesize;
    char*             dname = new char[1024]();
    char*             fname = new char[256]();
    char*             fnameOut = new char[256]();
    char*             ext = new char[15]();
    long long         nwords = 0;
    int               nSeg;
    int               nChunks;
    unsigned short*   datachunk;
    long long         totalcount;
    int               nRead;
    int               ii, jj;
    int               iSeg;
    int               n;
    bool              singleOutputFile = false;
    MyFileStandardIO* outputfile = NULL;
    unsigned short*   datetime_start = new unsigned short[8]();
    unsigned short*   datetime_end = new unsigned short[8]();


    // Check that user is supplying mandatory args
    if(argc<4)
    {
        printf("Error: Not enough arguments\n");
        getchar();
        return -1;
    }

    // Get optional arguments

    // Arg 5: option if true, writes out all segments to one file
    if(argc>4)
        singleOutputFile = !strcmp("onefile", argv[4]);

    // Arg 6: segment size is amount of available memory
    if(argc>5)
        segSize   = atoi(argv[5]);
    else
        segSize   = pow(2, 24);

    // Arg 7: how many words to read and parse at a time
    if(argc>6)
        chunkSize = atoi(argv[6]);
    else
        chunkSize = pow(2, 20);


    //% chunkSize and segSize are specified in bytes.
    //% Convert chunkSize and segSize into number of words
    segSize = segSize / 2;
    chunkSize = chunkSize / 2;

    printf("segSize:    %d\n", segSize);
    printf("chunkSize:  %d\n", chunkSize);

    Fx3DataParser*   parser = new Fx3DataParser(segSize);

    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    // % Read the file in segments and chunks, rather
    // % than a words at a time. Find number of segments
    // % and chunks in the file.
    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    sprintf_s(dname, 1024, argv[1]);
    sprintf_s(fname, 256, argv[2]);
    sprintf_s(ext, 15, argv[3]);

    printf("\n");

    MyFileStandardIO*  inputfile = new MyFileStandardIO(dname, fname, ext);
    if(inputfile->OpenForReading()==NULL)
    {
        printf("Error: Input file %s\\%s.%s doesn't exist...\n", dname, fname, ext);
        getchar();
        return -1;
    }

    filesize = inputfile->GetFileSize(dname, fname);

    nwords    = filesize/2;
    if(segSize>=nwords)
        segSize = nwords;
    if(chunkSize>=nwords)
        chunkSize = nwords;

    nSeg      = ceil((double)nwords / (double)segSize);
    nChunks   = ceil((double)segSize / (double)chunkSize);
    datachunk = new unsigned short[chunkSize]();

    int exp = (sizeof(parser->dataDCS->countThreshold)*8)-1;
    parser->dataDCS->countThreshold = pow(2, exp);

    // Recover start date and time at the beginning of file
    if(inputfile->ReadWords(datetime_start, 8*sizeof(unsigned short), &nRead) < 0)
        return -1;

    // Recover end date and time at the end of the file
    inputfile->Seek(-8 * ((int)sizeof(short)), SEEK_END);
    if(inputfile->ReadWords(datetime_end, 8*sizeof(unsigned short), &nRead) < 0)
        return -1;
    inputfile->Seek(0, SEEK_SET);

    // Create output file object.
    if(!singleOutputFile)
        outputfile = NULL;
    else {
        sprintf_s(fnameOut, 256, "%s_seg1.%s", fname, ext);
        outputfile = new MyFileStandardIO(dname, fnameOut);
        if(outputfile->OpenForWriting()==NULL)
        {
            printf("Error: Output file %s\\%s failed to open...\n", dname, fname);
            return NULL;
        }
    }

    for(jj=0; jj<nSeg; jj++)
    {
        parser->ClearBuffers();

        //% Reset data and counts for next segment
        for(ii=0; ii<nChunks; ii++)
        {
            memset(datachunk, 0, sizeof(unsigned short) * chunkSize);

            //% Load chunk into memory
            //if(ReadFile(hFile, datachunk, chunkSize * sizeof(unsigned short), (DWORD*)&nRead, NULL) == 0)
            if(inputfile->ReadWords(datachunk, chunkSize, &nRead) < 0)
                break;
            parser->parseData(datachunk, nRead);

            //% Display stats
#if 0
            printf("Processed chunk %d of %d in segment %d of %d, with [", ii+1, nChunks, jj+1, nSeg);
            dispCounts(parser);
            printf("] counts\n");
#endif

            //% If we read back into datachunk less than chunkSize, then this is 
            //% last chunk, so we can exit the loop
            if(nRead < chunkSize)
                break;
        }

        ////// Write out each completed segment to binary file

        // First create segment output file name
        // If outputfile object doesn't exist then use a new file. 
        // Else if outputfile object exists, then reuse the same file.
        memset(fnameOut, 0, 256);
        if(outputfile == NULL)
        {
            iSeg = jj+1;
            n = 1;
        }
        else
        {
            iSeg = 1;
            n = nSeg;
        }
        sprintf_s(fnameOut, 256, "%s_seg%d.%s", fname, iSeg, ext);

        // Save file 
        if(SaveFileSeg(&outputfile, dname, fnameOut, parser, n, datetime_start, datetime_end) < 0)
        {
            getchar();
            return -1;
        }

        //% Display segment stats
        printf("Saved segment %d of %d with [", jj+1, nSeg);
        dispCounts(parser);
        printf("] counts, to file %s\n", fnameOut);
    }

    // Write end of measurement date and time to single output file
    if(singleOutputFile)
        outputfile->Write(datetime_end, sizeof(unsigned short), 8);

    // Close inout and output files
    inputfile->Close();
    if(outputfile != NULL)
        outputfile->Close();

    // Wait for user input before exiting console
    getchar();
    return 0;

}


