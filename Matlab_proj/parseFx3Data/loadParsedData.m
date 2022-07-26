function [dataDCS, dataADC] = loadParsedData(fname, iSeg_selected)


dataDCS = initDCSdata();
dataADC = initADCdata();

fp = fopen(fname, 'rb');

% Get end and start of measurement date and time and return file pointer to the beginning of
% file
fseek(fp, -16, 'eof');
datetime_end   = fread(fp, 8, 'uint16=>uint16');
fseek(fp, 0, 'bof');
datetime_start = fread(fp, 8, 'uint16=>uint16');

% Peek ahead to see how many segments there are in this file and how many
% dcs channels
nSeg = fread(fp, 1, 'int');
if ~exist('iSeg_selected','var')
    iSeg_selected = 1:nSeg;
end

nCh_dcs = fread(fp, 1, 'int');
nCh_adc = fread(fp, 1, 'int');

% Rewind file pointer to beginning of file
fseek(fp, 0, 'bof');

for iSeg=1:nSeg
    
    dataDCS_seg = initDCSdata(nCh_dcs);
    dataADC_seg = initADCdata(nCh_adc);
    
    % Get start of meas date and time
    d = fread(fp, 8, 'uint16=>uint16');
    
    % Number of segments in this file
    n = fread(fp, 1, 'int');
    
    % DCS and ADC num of channels
    dataDCS_seg.nCh = fread(fp, 1, 'int=>int32');
    dataADC_seg.nCh = fread(fp, 1, 'int=>int32');
    
    % DCS and ADC data counts
    dataDCS_seg.count = fread(fp, 4, 'uint32=>uint32');
    dataADC_seg.count = fread(fp, 1, 'uint32=>uint32');
    
    % ADC channel order
    dataADC_seg.chIdxOrder = fread(fp, 4, 'int=>int32');
    
    % photon arrival times
    for kk=1:dataDCS_seg.nCh
        if dataDCS_seg.count(kk)==0
            dataDCS_seg.arrtimes{kk} = cell(0,2);
            continue;
        end
        dataDCS_seg.arrtimes{kk}(:,1) = fread(fp, dataDCS_seg.count(kk), 'uint64=>uint64');
        dataDCS_seg.arrtimes{kk}(:,2) = fread(fp, dataDCS_seg.count(kk), 'uint64=>uint64');
    end
    
    % photon arrival event table    
    for kk=1:dataDCS_seg.nCh
        if dataDCS_seg.count(kk)==0
            dataDCS_seg.eventtable{kk} = cell(0,4);
            continue;
        end
        dataDCS_seg.eventtable{kk}(:,1) = fread(fp, dataDCS_seg.count(kk), 'uint8=>uint8');
        dataDCS_seg.eventtable{kk}(:,2) = fread(fp, dataDCS_seg.count(kk), 'uint8=>uint8');
        dataDCS_seg.eventtable{kk}(:,3) = fread(fp, dataDCS_seg.count(kk), 'uint8=>uint8');
        dataDCS_seg.eventtable{kk}(:,4) = fread(fp, dataDCS_seg.count(kk), 'uint8=>uint8');
    end
    
    % analog data
    dataADC_seg.t     = fread(fp, dataADC_seg.count, 'uint64=>uint64');
    for kk=1:dataADC_seg.nCh
        dataADC_seg.amp(:,kk) = fread(fp, dataADC_seg.count, 'uint64=>uint64');
    end
    
    % Get start of meas date and time
    d = fread(fp, 8, 'uint16=>uint16');
    
    if ~ismember(iSeg, iSeg_selected)
        continue;
    end
    
    
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    
    % DCS
    dataDCS.nCh   = dataDCS_seg.nCh;
    if iSeg==iSeg_selected(1)
        for kk=1:dataDCS.nCh
            dataDCS.count(kk)      = dataDCS_seg.count(kk);
            dataDCS.arrtimes{kk}   = dataDCS_seg.arrtimes{kk};            
            dataDCS.eventtable{kk} = dataDCS_seg.eventtable{kk};
        end
    else
        for kk=1:dataDCS.nCh
            dataDCS.count(kk)      = dataDCS.count(kk) + dataDCS_seg.count(kk);
            dataDCS.arrtimes{kk}   = [dataDCS.arrtimes{kk}; dataDCS_seg.arrtimes{kk}];
            dataDCS.eventtable{kk} = [dataDCS.eventtable{kk}; dataDCS_seg.eventtable{kk}];
        end
    end
    dataDCS.datetime_start = copyDateTime(datetime_start);
    dataDCS.datetime_end = copyDateTime(datetime_end);
    
    % ADC
    dataADC.nCh        = dataADC_seg.nCh;
    dataADC.count      = dataADC.count + dataADC_seg.count;
    dataADC.chIdxOrder = dataADC_seg.chIdxOrder;
    dataADC.t          = [dataADC.t; dataADC_seg.t];
    if iSeg==iSeg_selected(1)
        dataADC.amp = dataADC_seg.amp;
    else
        dataADC.amp = [dataADC.amp; dataADC_seg.amp];
    end
    dataADC.datetime_start = copyDateTime(datetime_start);
    dataADC.datetime_end = copyDateTime(datetime_end);

    fprintf('Finished loading segment %d of %d\n', iSeg, nSeg);
    
end

fclose(fp);
