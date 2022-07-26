function readFXDataFile(fname, chunkSize, segSize)

global dataDCS;
global dataADC;

%%%%%%%%%%%%%%%%%%%%%%%%
% Process arguments
%%%%%%%%%%%%%%%%%%%%%%%%
if ~exist('chunkSize','var')
    chunkSize = 2^17;
end
if ~exist('segSize','var')
    segSize = 2^24;
end

fprintf('chunckSize: %d\n', chunkSize);
fprintf('segSize:    %d\n', segSize);

% chnukSize and segSize are specified in bytes. 
% Convert chunkSize and segSize into number of words
chunkSize = chunkSize / 2;
segSize = segSize / 2;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Read the file in segments and chunks, rather 
% than a words at a time. Find number of segments 
% and chunks in the file.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
[p, f, ~] = fileparts(fname);
if isempty(p)
    p = '.';
end
p = [p, '/'];
fstat     = dir(fname);
nwords    = fstat.bytes/2;
if chunkSize>=nwords
    chunkSize = nwords;
end
if segSize>=nwords
    segSize = nwords;
end
nSeg      = ceil(nwords / segSize);
nChunks   = ceil(segSize / chunkSize);
datachunk = uint16(zeros(chunkSize,1));

nCh_dcs = 4;
nCh_adc = 4;

wrapcount  = zeros(nCh_dcs+1,1);
totalcount = [];

% Get file handle
fid = fopen(fname, 'rb');

% Get start and time of measurement and return file pointer to the beginning of
% file
datetime_start = fread(fid, 8, 'uint16=>uint16');
fseek(fid, -16, 'eof');
datetime_end   = fread(fid, 8, 'uint16=>uint16');
fseek(fid, 0, 'bof');

%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Main data processing loop
%%%%%%%%%%%%%%%%%%%%%%%%%%%
for jj=1:nSeg
              
    % Reset data and counts for next segment
    dataDCS   = initDCSdata(nCh_dcs, segSize);
    dataADC   = initADCdata(nCh_adc, segSize);
    
    for ii=1:nChunks
        
        % Load chunk into memory
        datachunk = fread(fid, chunkSize, 'uint16=>uint16');
        [photcount, adccount, wrapcount, kk] = parseFXData(datachunk, wrapcount);
        
        % Display stats
if 1
        fprintf('Processed chunk %d of %d in segment %d of %d, with [', ii, nChunks, jj, nSeg)
        dispCounts(wrapcount);
        fprintf('] counts\n');
end    
        % If we read back into datachunk less than chunkSize, then this is 
        % last chunk, so we can exit the loop
        if length(datachunk) < chunkSize
            break;
        end

    end   
        
    % Crop allocated data before writing it out to a file
    cropFXData();
    fnameOut = [p, f, '_seg', num2str(jj), '.mat'];
    save(fnameOut, 'dataDCS', 'dataADC');
    
    % Display stats
    fprintf('Saved segment %d of %d with [', jj, nSeg);
    dispCounts(wrapcount);
    if isempty(totalcount)
        totalcount = [dataDCS.count; dataADC.count];
    else
        totalcount = totalcount + [dataDCS.count; dataADC.count];
    end
    fprintf('] counts, to file %s\n', fnameOut);
    
end

fprintf('Final counts: ', ii, nChunks, jj, nSeg)
dispCounts(wrapcount);
fprintf(']\n');

% Delete file handle
fclose(fid);


