function mylogbin2(iCh)

global nbins;
global g;
global gmean;
global i_binedge;
global t_binmean;

% Calculating g mean. Can precalculate devisor 
% (i_binedge(ii+1)-i_binedge(ii)+1))
gbinsum = zeros(1,nbins);
for ii = 1:nbins
    % gmean(ii) = sum(g(i_binedge(ii):i_binedge(ii+1)) / (i_binedge(ii+1)-i_binedge(ii)+1));
    for jj=i_binedge(ii):i_binedge(ii+1)
        gbinsum(ii) = gbinsum(ii) + g(jj);
    end
    gmean(ii,iCh) = gbinsum(ii) / t_binmean(ii);
end


