function [t, auxdata, nwrites] = readAuxData(filenm)

t       = uint64([]);
auxdata = {};

nwrites = [];

fid = fopen(filenm,'rb');
if fid<0
    fprintf('ERROR: Could not open file %s. Exiting.\n', filenm);
    return;
end

[nCh, nread]   = fread(fid, 1, 'int32');
if nread>0 & nCh>0
    auxdata = cell(nCh, 1);
    
    nwrites = 1;
    while 1
        [count, nread] = fread(fid, 1, 'int32');
        if nread<1
            break;
        end
        t = [t; fread(fid, count, 'int64')];
        for ii=1:nCh
            auxdata_temp = fread(fid, count, 'int64');
            auxdata{ii} = [auxdata{ii}; auxdata_temp];
        end
        nwrites=nwrites+1;
    end
end
fclose(fid);

