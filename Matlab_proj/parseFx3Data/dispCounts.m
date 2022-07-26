function dispCounts(wrapcount, varargin)

global dataDCS;
global dataADC;

photcount = dataDCS.count;
adccount = dataADC.count;
if nargin==3
    photcount = varargin{1};
    adccount = varargin{2};
end

for kk=1:length(photcount)
    fprintf('%d, ', photcount(kk));
end
fprintf('%d, ', adccount);
for kk=1:length(wrapcount)
    if kk==length(wrapcount)
        fprintf('%d', wrapcount(kk));
    else
        fprintf('%d, ', wrapcount(kk));
    end
end
