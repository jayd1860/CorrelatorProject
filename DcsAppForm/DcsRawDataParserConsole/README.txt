Usage Instructions:

=============================
Using DcsRawDataParserConsole 
=============================

Passing arguments:
------------------

The DcsRawDataParserConsole Visual Studio project is in 

   <Root correlatorproject folder>/DcsCollectDataForm/DcsRawDataParserConsole

The DcsRawDataParserConsole expects at least 3 arguments to be passed

   mandatory arguments:

   1) full path of folder where the acquisition file is
   2) base file name ( without extension )
   3) extension ( without the . ) 
 
   optional arguments:
   
   4) option to select single output file for all data segments, instead of the default of one segment per file. 
      The possible values are: {'multfile' | 'onefile'}
   5) segment size (tells the application the amount of memory available)
   6) chunk size (tells the application the amount of data to read in and process at a time)

In Visual Studio to pass arguments to main you go to project 

    Properties --> Debugging --> Command arguments

and list the arguments on the same line with spaces separating the arumetns

For example to read parse the file c:\users\Public\DCS\CollectData.bin you enter 

    c:\users\Public\DCS CollectData bin

spaces separate each argument. 


Output data structure:
----------------------

The structure of the data writtin to each segment file is the following 
     
      nSeg              // Number of segments in this file
      nCh_dcs
      nCh_adc
      count_dcs[0]
        ....
      count_dcs[nCh]
      count_adc
      chIdxOrder_adc    // The association between amp array index and the physical analog channel

      // DCS data: 

      arrtimes[0]
        ....
      arrtimes[nCh]
      eventtable[0]
        ....
      eventtable[nCh]

      // ADC data:

      t             // time
      amp[0]        // analog value in channel[chIdxOrder[0]]
        ....
      amp[nCh]      // analog value in channel[chIdxOrder[nCh]]


=============================
Loading parsed data in MATLAB
=============================

The matlab script to load the parse data is loadParsedData.m in 

    <Root correlatorproject folder>/Matlab_proj/parseFx3Data

Here's an example of using it to load a segment

    >> [dataDCS, dataADC] = loadParsedData('c:/users/public/dcs/CollectData_seg1.bin')
    Finished loading segment 1 of 1

     dataDCS = 

       arrtimes: {4x1 cell}
     eventtable: {4x1 cell}
          count: [4x1 double]
            nCh: 4

     dataADC = 

              t: [108225x1 double]
            amp: [108225x4 uint64]
          count: 108225
            nCh: 4
     chIdxOrder: [4x1 double]

One parsed file can have all the segments. You can retrieve any segment or segments 
by supplying the second argument to loadParsedData.

For example:

    >> [dataDCS2, dataADC2] = loadParsedData('c:/Users/Public/DCS/test/AnalogIN_test_seg1.bin',[2,6,7]);
    Finished loading segment 2 of 10
    Finished loading segment 6 of 10
    Finished loading segment 7 of 10
    >>

 
