function cropFXData()

global dataDCS;
global dataADC;

photcount = dataDCS.count;
adccount = dataADC.count;

nCh_dcs = dataDCS.nCh;

%%%%%%%%%%%%%%%%%%%%%%%%%%
% DCS definitions 
%%%%%%%%%%%%%%%%%%%%%%%%%%

for ii=1:nCh_dcs
    if photcount(ii)==0
        dataDCS.arrtimes{ii} = [];
        continue;
    end    
    dataDCS.arrtimes{ii}(photcount(ii)+1:end,:) = [];
end

%%%%%%%%%%%%%%%%%%%%%%%%%
% ADC definitions 
%%%%%%%%%%%%%%%%%%%%%%%%%

if adccount>0
    dataADC.t(adccount+1:end) = [];
    dataADC.amp(adccount+1:end,:) = [];
end


