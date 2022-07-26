function [g, t] = my_pulse_xcorr_zero(y, y_shift, counts, counts_shift, maxlag, samplef)

% my_pulse_xcorr_zero - 2-stage pulse autocorrelation
%  g - computed autocorrelation bins
%  t - lag time of computed bins in [s]
%  maxlag - maximum lag time to compute in samplef [periods]
%  samplef - 1/timestamps_resolution (e.g. 150e6)
%  elimi - number of bins to be removed from the start
%
%  written by Kuan-Cheng (Tony) Wu & Bernhard Zimmermann
%  MGH May 2016
%  with ideas from:
%  Emmanuel Schaub, "High countrate real-time FCS using F2Cor," Opt. Express 21, 23543-23555 (2013)
%  Davide Magatti and Fabio Ferri, "Fast multi-tau real-time software correlator for dynamic light scattering," Appl. Opt. 40, 4011-4021 (2001)


g = zeros(1,maxlag);
t = 1:length(g);
g1 = zeros(1,maxlag);
g1_flags = false(1,maxlag);
start = 1;
N = y(end);
n1 = length(y);
n2 = length(y_shift);
elimi = 1;

if isempty(y)
    return;
end

HAMAMATSU = 0;
HAMAMATSU_APPROX = 0;


h = waitbar(0,'Please wait...');
waitbar_update_interval = 1000;
 
for ii = 1:n1
   notset = true;
   
   if mod(ii, waitbar_update_interval)==0
        waitbar(ii/length(y), h, sprintf('Completed %d of %d', ii, length(y)));
   end
   
   for jj = ii+1:n2
       k = y_shift(jj) - y(ii);
       
       if (k <= maxlag) && (k > 0)
           
           % g(k) = g(k) + counts(ii) * counts_shift(jj);
           g(k) = g(k) + 1;
           
           if HAMAMATSU
               
               if ~g1_flags(k+1)
                   g1(k+1) = length(counts(ii:n2));
                   g1_flags(k+1) = true;
               end
               
           end
                      
       else
           
           break;
           
       end
   end
   

end


if HAMAMATSU

    g = (N*g ./ (sum(counts)*g1)) - 1;
	t = ((0:maxlag) .* (1/samplef));

elseif HAMAMATSU_APPROX

    g = (N*g) / (sum(counts)*sum(counts_shift)) - 1;
	t = ((0:maxlag) .* (1/samplef));

else

    countR = length(y) / ((y(end) - y(1)) / samplef); %countR
    t = (((1:(length(g) - elimi)) + elimi) .* (1/samplef));
    g = (y(end) - y(1) - t .* samplef) ./ ((length(y) - t .* countR).^2) .* g;
    
    % g = (max([(y(end)-y(1)), (y_shift(end)-y_shift(1))]) / (sum(counts) * sum(counts_shift))) .* g;
    
end


close(h);

