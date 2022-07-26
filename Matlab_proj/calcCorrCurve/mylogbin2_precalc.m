function mylogbin2_precalc()

global nbins;
global t;
global tmean;
global i_binedge;
global t_binmean;
global maxlag;

% precalculate:  Determine k intevals
t_binedge = mylogspace(log10(t(2)), log10(t(end)), nbins+1);

% precalculate: This is taking mean of k interval for new delay values
% tmean = (t_binedge(2:end) + t_binedge(1:end-1))/2;
for ii=1:nbins
    tmean(ii) = (t_binedge(ii+1) + t_binedge(ii)) / 2;
end

% precalculate: This for loop is instead of rounding of k edges. 
i_binedge = zeros(size(t_binedge));
for ii = 1:nbins+1
    % [~, i_binedge(ii)] = min(abs(t-t_binedge(ii)));
    binedge_min = abs(t(2)-t_binedge(ii));
    i_binedge_min = 1;
    for jj = 2:maxlag+1
        if abs(t(jj)-t_binedge(ii)) < binedge_min
            binedge_min = abs(t(jj)-t_binedge(ii));
            i_binedge_min = jj;
        end
    end
    i_binedge(ii) = i_binedge_min;
end


t_binmean = zeros(1,nbins);
for ii = 1:nbins
    t_binmean(ii) = i_binedge(ii+1)-i_binedge(ii)+1;
end

