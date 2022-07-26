function [tmean, gmean] = readCorrData(filenm)

tmean = [];
gmean = [];

fid = fopen(filenm,'rb');
if fid<0
    fprintf('ERROR: Could not open file %s. Exiting.\n', filenm);
    return;
end
nbins = fread(fid, 1, 'int32');
nch = fread(fid, 1, 'int32');
tmean = fread(fid, nbins, 'double');

tt = 1;
while 1
    [input, count] = fread(fid, nch*nbins, 'double');
    if count<nch*nbins
        break;
    end    
    gmean(:,:,tt) = reshape(input,nbins,nch);
    tt = tt+1;
end
fclose(fid);

h = figure; 
for tt=1:size(gmean,3)
    plot(tmean, gmean(:,1,tt))
    set(gca,'ylim',[.95, 1.60]);
    set(gca, 'xscale','log');
    pause(.5);
    cla;
    pause(.5)
end
close(h);


