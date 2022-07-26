function [gmean, tmean] = calcAutocorrCurve(fpath, chunkSize, segSize)

%
% Example:
%
% [gmean, tmean] = calcAutocorrCurve('../../../data/004_10s_ampli2300mA.bin', 1.5e6, 'newcorr1')
%

global elimi;
global samplef;
global maxlag;
global nbins;
global g;
global t;
global nCh;
global gmean;
global tmean;
global q;
global P;
global ncorr;

if ~exist('chunkSize','var') | isempty(chunkSize)
    chunkSize = 2^17;
end
if ~exist('segSize','var')  | isempty(segSize)
    segSize = 2^22;
end

if ~exist(fpath, 'file')
    fprintf('Warning: Input file doesn''t exist. Exiting ...\n');
    return;
end
[pname, fname, ext] = fileparts(fpath);
files = dir([pname, '/', fname, '_seg*.mat']);
if isempty(files)
    readFXDataFile(fpath, chunkSize, segSize);
    files = dir([pname, '/', fname, '_seg*.mat']);
end
if ~exist('maxlag', 'var') | isempty(maxlag) 
    maxlag = samplef/1e3;
end

nCh = 4;
elimi = 1;
samplef = 1.5e8;
maxlag = 6e5;
g = zeros(1,maxlag+1);
t = 1:length(g);
P = 20;
q = 2;
ncorr = ceil(log(maxlag/P)/log(q))+1;
nbins = P+P/q*(ncorr-2);

%t = ((0:maxlag).*(1/samplef));
for ii=0:maxlag
    t(ii+1) = ii * 1/samplef;
end
gmean = zeros(nbins, nCh);
tmean = zeros(nbins, 1);

mylogbin2_precalc();

h = figure;
%for ii=1:length(files)-1
for ii=1:1
    displayCleared = false;
    
    fnameParsed = [pname, '/', fname, '_seg', num2str(ii), '.mat'];
    load(fnameParsed);
    
    if iscell(dataDCS)        
        dataDCS = struct('arrtimes', {dataDCS}, 'nCh', length(dataDCS));
        for iCh=1:dataDCS.nCh
           dataDCS.count(iCh) = length(dataDCS.arrtimes{iCh}(:,1));
        end        
    end
    
    gmean(:) = 0;
    
    for iCh=4:dataDCS.nCh
        if dataDCS.count(iCh)==0
            continue;
        end
        
        y = dataDCS.arrtimes{iCh}(:,1);
        
        my_pulse_xcorr_zero(y, y);
        mylogbin2(iCh);
    
        fprintf('Calculated corr curve for ch %d segment %d\n', iCh, ii);
        if displayCleared==false           
    		cla(gca, 'reset');    
            displayCleared = true;
    	end
	    plot(tmean(elimi+1:end), gmean(elimi+1:end,iCh));
    	hold all;
	    set(gca,'ylim',[.99, 1.04]);
        set(gca, 'xscale','log');
    
	    if (max(tmean) > min(tmean))
	        set(gca,'xlim',[min(tmean), max(tmean)]);
	    end
    	drawnow;
    end
    
end


