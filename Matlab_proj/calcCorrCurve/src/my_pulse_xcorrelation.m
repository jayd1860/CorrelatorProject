function [g, t] = my_pulse_xcorrelation(y, y_shift, maxlag, samplef, elimi)

%function [g, t] =
%pulse_xcorrelation(y,apd_r_shift,maxlag,samplef,elimi)...xcorr
%   without negative lag

%{
function [g, t] = my_pulse_xcorrelation(y, maxlag, samplef, elimi)

g = zeros(1, maxlag);
n1 = length(y);
n2 = length(y);

if isempty(y)
    return;
end

for ii = 1:n1
    for jj = (ii+1):n2
        k = y(jj) - y(ii);
              
        if (k <= maxlag) && (k > 0)
            g(k) = g(k)+1;
        else
            break;
        end
    end
end

countR = length(y) / ((y(end) - y(1)) / samplef); %countR
t = (((1:(length(g) - elimi)) + elimi) .* (1/samplef));
g = (y(end)-y(1) - t .* samplef) ./ ((length(y) - t .* countR).^2) .* g(elimi+1:end);

%}

% Modified by Jay Dubb for easy portability to C/C++, Sep 13, 2016

if ~exist('elimi','var')
    elimi = round(maxlag/1e3);
end
g0 = zeros(1, maxlag);
g = zeros(1, maxlag-elimi);
t = 1:length(g);
n1 = length(y);
n2 = length(y_shift);
c1 = y(end)-y(1);
c2 = (1/samplef);

if isempty(y)
    return;
end

for ii = 1:n1
 
   for jj = (ii+1):n2
       k = y_shift(jj) - y(ii);
       
       if (k <= maxlag) && (k > 0)
           g0(k) = g0(k) + 1;
       else
           break;
       end
   end

end

countR = n1 / (c1 / samplef); %countR

for ii=1:maxlag-elimi
    t(ii) = (ii+elimi) * c2;
end
for ii=1:maxlag-elimi
    g(ii) = (c1 - t(ii) * samplef) / ((n1 - t(ii) * countR) * (n1 - t(ii) * countR)) * g0(ii+elimi);
end

