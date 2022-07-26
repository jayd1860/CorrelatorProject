function [acf, t, countR] = pulse_corr_2stage(tstamps, max_lag, samplef, elimi)
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

%  Edit history
%  Jul-16-2016 Tony
%  make both situations creat the tau vector with the same length, but does not have the same number inside.
%  only use the 2-stage
%  Jul-17-2016 Tony
%  1. Add count rate output.
%  2. Make the curve stays at 1 while having small amount of photons.
%  3. Fake the navg so that it always gives us a same tau vector.

P = 20;
m = 2;

navg = 300*1000/150e6;%length(tstamps)/(tstamps(end)-tstamps(1));
K = round(log(1/navg)/log(2)) +1;

ncorr = ceil(log(max_lag/P)/log(m))+1;
% K = 12;
K = max(min(K, ncorr+1), 2);

t_tstamps = tstamps(end)-tstamps(1)+1;
countR = length(tstamps)/(t_tstamps/samplef);

if false%K>ncorr
    % if count rate is low, just use normal pulse autocorrelation
    disp('Using normal pulse acf');
    [acf_temp, t_temp] = pulse_corr_zero(tstamps,ones(size(tstamps)),max_lag,samplef,countR);
    [acf, t] = logbin2(acf_temp(2:end), t_temp(2:end), P+P/m*(ncorr-1)); %P+P/m*(ncorr-2)
    acf = acf((elimi+1):end); %acf = acf(elimi:end);
    t = t((elimi+1):end); %t = t(elimi:end);
else
    % do the 2-stage algorithm
    acf = zeros(1,P+P/m*(ncorr-1));
    t = zeros(size(acf));

    tstamps_rebin = unique(tstamps);
    tstamps_counts = hist(tstamps,tstamps_rebin);

    % for short lag times, use normal pulse acf
    ic = 1;
    iclast = ic;
%     disp(['ic: ' num2str(ic) ' / ' num2str(ncorr) ' Rec_len: ' num2str(length(tstamps_rebin)) ' n photons: ' num2str(sum(tstamps_counts))]);
% pulse_corr_zero(apd_r_fixed,apd_r_counts,max_lag,samplef)
    [acf_temp, t_temp] = pulse_corr_zero(tstamps_rebin,tstamps_counts,P*m.^(K-2),samplef/(m^(ic-1)));

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
    
    % for long lag times, use multi-tau approach (for k >= K)
    for ic = K:ncorr
        [tstamps_rebin, ~, indc] = unique(round(tstamps_rebin/(m^(ic-iclast))));
        tstamps_counts_new = zeros(size(tstamps_rebin));
        for is = 1:length(indc)
            tstamps_counts_new(indc(is)) = tstamps_counts_new(indc(is)) + tstamps_counts(is);
        end
        tstamps_counts = tstamps_counts_new;  
%         disp(['ic: ' num2str(ic) ' / ' num2str(ncorr) ' Rec_len: ' num2str(length(tstamps_rebin)) ' n photons: ' num2str(sum(tstamps_counts))]);
        [acf_temp, t_temp] = pulse_corr_zero(tstamps_rebin,tstamps_counts,P,samplef/(m^(ic-1)));
        acf(P + (P/m)*(K-2) + (ic-K)*P/m + (1:P/m)) = acf_temp((P/m+2):end);
        t(P + (P/m)*(K-2) + (ic-K)*P/m + (1:P/m)) = t_temp((P/m+2):end);
        iclast = ic;
    end
    acf = acf((elimi+1):end);
    t = t((elimi+1):end);
end
acf = (((t_tstamps-t.*samplef)./((length(tstamps)-t.*countR).^2))./((t_tstamps/(length(tstamps)^2)))).*acf;
% temp_t = toc;
% disp(['New two stage autocorrelation takes ' num2str(temp_t) 's'])
end

function [acf, t] = pulse_corr_zero(apd_r_fixed,apd_r_counts,max_lag,samplef)

acf = zeros(1,max_lag+1);
for ai = 1:length(apd_r_fixed)
   for bi = (ai+1):length(apd_r_fixed)
       temp_delta = apd_r_fixed(bi)-apd_r_fixed(ai) ;
       if (temp_delta <= max_lag) && (temp_delta >= 0)
           acf(temp_delta+1) = acf(temp_delta+1) + apd_r_counts(bi)*apd_r_counts(ai) ;
       else
           break;
       end
   end
end
t = (0:max_lag).*(1/samplef);
acf = (apd_r_fixed(end)-apd_r_fixed(1))./(((sum(apd_r_counts)).^2)).*acf;
end