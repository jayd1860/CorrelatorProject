function [acflb, tvlb] = logbin2(acf, tv, n)

if nargin < 3
    n = 100;
end

% precalculate:  Determine k intevals
tv_binedge = logspace(log10(tv(1)),log10(tv(end)),n+1);

% precalculate: This is taking mean of k interval for new delay values
tvlb = (tv_binedge(2:end) + tv_binedge(1:end-1))/2;

acflb = zeros(1,n);

% precalculate: This for loop is instead of rounding of k edges. 
i_binedge = zeros(size(tv_binedge));
for ii = 1:length(i_binedge)
    [~, i_binedge(ii)] = min(abs(tv-tv_binedge(ii)));
end

% Calculating g mean. Can precalculate devisor (i_binedge(ii+1)-i_binedge(ii)+1))
for ii = 1:n
    acflb(ii) = sum(acf(i_binedge(ii):i_binedge(ii+1))/(i_binedge(ii+1)-i_binedge(ii)+1));
end

