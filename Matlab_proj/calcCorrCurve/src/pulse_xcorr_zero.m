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

