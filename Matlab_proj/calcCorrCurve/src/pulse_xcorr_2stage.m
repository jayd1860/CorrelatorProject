function [acf, t] = pulse_xcorr_2stage(tstamps, tstamps_shift, max_lag, samplef, elimi)

% pulse_corr_2stage - 2-stage pulse autocorrelation
%  [acf, t] = pulse_corr_2stage(tstamps, max_lag, samplef, elimi)
%  acf - computed autocorrelation bins
%  t - lag time of computed bins in [s]
%  max_lag - maximum lag time to compute in samplef [periods]
%  samplef - 1/timestamps_resolution (e.g. 150e6)
%  elimi - number of bins to be removed from the start
%
%
%  written by Kuan-Cheng (Tony) Wu & Bernhard Zimmermann
%  MGH May 2016
%  with ideas from:
%  Emmanuel Schaub, "High countrate real-time FCS using F2Cor," Opt. Express 21, 23543-23555 (2013)
%  Davide Magatti and Fabio Ferri, "Fast multi-tau real-time software correlator for dynamic light scattering," Appl. Opt. 40, 4011-4021 (2001)

P = 20;
m = 2;

navg = length(tstamps)/(tstamps(end)-tstamps(1));
K = round(log(1/navg)/log(2)) +1;

ncorr = ceil(log(max_lag/P)/log(m))+1;
% K = 12;
K = max(min(K, ncorr+1), 2);

override = true;

if K>ncorr | override
    
    % if count rate is low, just use normal pulse autocorrelation
    disp('Using normal pulse acf');
    [acf_temp, t_temp] = pulse_xcorr_zero(tstamps, tstamps_shift, ones(size(tstamps)), ones(size(tstamps_shift)), max_lag, samplef);
    % [acf_temp, t_temp] = pulse_xcorrelation(tstamps, max_lag, samplef, elimi);
    [acf, t] = logbin2(acf_temp(2:end), t_temp(2:end), P+P/m*(ncorr-2));
    
else
    
    % do the 2-stage algorithm
    acf = zeros(1,P+P/m*(ncorr-1));
    t = zeros(size(acf));

    tstamps_rebin = unique(tstamps);
    tstamps_counts = hist(tstamps,tstamps_rebin);
    
    tstamps_rebin_shift = unique(tstamps_shift);
    tstamps_counts_shift = hist(tstamps_shift,tstamps_rebin_shift);

    % for short lag times, use normal pulse acf
    ic = 1;
    iclast = ic;
    [acf_temp, t_temp] = pulse_xcorr_zero( tstamps_rebin, ...
                                           tstamps_rebin_shift, ...
                                           tstamps_counts, ...
                                           tstamps_counts_shift, ...
                                           P*m.^(K-2), ...
                                           samplef/(m^(ic-1)) );

    acf_temp_rebin = zeros(1,P + (P/m)*(K-2));
    t_temp_rebin = zeros(1,P + (P/m)*(K-2));

    acf_temp_rebin(1:P) = acf_temp(2:(P+1));
    t_temp_rebin(1:P) = t_temp(2:(P+1));

    % rebin result into exponentially sized bins
    for ib = 0:(K-3)
        rbsel = P+ib*P/m+(1:P/m);
        orsel = P*m.^ib + 1 + (m^(ib+1))*(0:(P/m)-1);
        for is = 0:(m^(ib+1)-1)
            acf_temp_rebin(rbsel) = acf_temp_rebin(rbsel) + acf_temp(orsel + is);
        end
        acf_temp_rebin(rbsel) = acf_temp_rebin(rbsel) ./ (m^(ib+1));
        t_temp_rebin(rbsel) = t_temp(orsel);
    end


    acf(1:P + (P/m)*(K-2)) = acf_temp_rebin;
    t(1:P + (P/m)*(K-2)) = t_temp_rebin;
    
    % for long lag times, use multi-tau approach
    for ic = K:ncorr
        [tstamps_rebin, ~, indc] = unique(round(tstamps_rebin/(m^(ic-iclast))));
        tstamps_counts_new = zeros(size(tstamps_rebin));
        [tstamps_rebin_shift, ~, indc_shift] = unique(round(tstamps_rebin_shift/(m^(ic-iclast))));
        tstamps_counts_new_shift = zeros(size(tstamps_rebin_shift));
        for is = 1:length(indc)
            tstamps_counts_new(indc(is)) = tstamps_counts_new(indc(is)) + tstamps_counts(is);
        end
        tstamps_counts = tstamps_counts_new;  
        for is = 1:length(indc_shift)
            tstamps_counts_new_shift(indc_shift(is)) = tstamps_counts_new_shift(indc_shift(is)) + tstamps_counts_shift(is);
        end
        tstamps_counts_shift = tstamps_counts_new_shift;
        
        [acf_temp, t_temp] = pulse_xcorr_zero( tstamps_rebin, ...
                                               tstamps_rebin_shift, ...
                                               tstamps_counts, ...
                                               tstamps_counts_shift, ...
                                               P,samplef/(m^(ic-1)) );
                                          
        acf(P + (P/m)*(K-2) + (ic-K)*P/m + (1:P/m)) = acf_temp((P/m+2):end);
        t(P + (P/m)*(K-2) + (ic-K)*P/m + (1:P/m)) = t_temp((P/m+2):end);
        iclast = ic;
    end
    
end



% --------------------------------------------------------------------------------------------------------------
function [acf, t] = pulse_xcorr_zero(apd_r_fixed, apd_r_shift, apd_r_counts, apd_r_counts_shift, max_lag,samplef)

acf = zeros(1,max_lag+1);
bi_start = 1;

h = waitbar(0,'Please wait...');
waitbar_update_interval = 1000;
for ai = 1:length(apd_r_shift)
   notset = true;
   
   if mod(ai, waitbar_update_interval)==0
        waitbar(ai/length(apd_r_shift), h, sprintf('Completed %d of %d',ai, length(apd_r_shift)));
   end
   
   for bi = bi_start:length(apd_r_fixed)
       temp_delta = apd_r_fixed(bi)-apd_r_shift(ai) ;
       if (temp_delta >= 0)
           if (temp_delta <= max_lag)
                acf(temp_delta+1) = acf(temp_delta+1) + apd_r_counts(bi) * apd_r_counts_shift(ai);
                if notset
                    notset = false;
                    bi_start = bi;
               end
           else
               break;
           end
       end
   end
end

acf = (max([(apd_r_fixed(end)-apd_r_fixed(1)), (apd_r_shift(end)-apd_r_shift(1))])/(sum(apd_r_counts)*sum(apd_r_counts_shift))).*acf;
t = ((0:max_lag).*(1/samplef));
close(h);

