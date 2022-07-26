% apd_r_temp = (real) time-stamps of the photons
% max_lag = the maximum lag
% acf = result of the auto-correlation = g2
% samplef = sampling rate
% section_photon = amount of photons in this data
% elimi = start from "lag = elimi"
%function [acf, t] = 
%pulse_xcorrelation(apd_r_fixed,max_lag,samplef,elimi)...autocorr
%   without negative lag 
% or
%function [acf, t] =
%pulse_xcorrelation(apd_r_fixed)...autocorr
%   without negative lag
% or
%function [acf, t] = 
%pulse_xcorrelation(apd_r_fixed,apd_r_shift,max_lag,samplef,elimi)...xcorr
%   without negative lag 
% or
%function [acf, t] =
%pulse_xcorrelation(apd_r_fixed,apd_r_shift)...xcorr
%   WITH negative lag
function [acf, t, varargout] = pulse_xcorrelation(apd_r_fixed,varargin)
% tic
switch nargin
    case 4 %autocorr
        max_lag = varargin{1};
        samplef = varargin{2};
        elimi = varargin{3};
        acf = zeros(1,max_lag);
        if false
            for ai = 1:length(apd_r_fixed)
               for bi = (ai+1):length(apd_r_fixed)
                   temp_delta = apd_r_fixed(bi)-apd_r_fixed(ai) ;
                   if (temp_delta <= max_lag)
                       if (temp_delta > elimi)
                            acf(temp_delta) = acf(temp_delta)+1;
                       end
                   else
                       break;
                   end
               end
            end
        else
            for ai = 1:length(apd_r_fixed)
               for bi = (ai+1):length(apd_r_fixed)
                   temp_delta = apd_r_fixed(bi)-apd_r_fixed(ai) ;
                   if (temp_delta <= max_lag) && (temp_delta > 0)
                       acf(temp_delta) = acf(temp_delta)+1;
                   else
                       break;
                   end
               end
            end
        end
        countR = length(apd_r_fixed)/((apd_r_fixed(end)-apd_r_fixed(1))/samplef);%countR
        varargout{1} = countR;
        t = (((1:(length(acf)-elimi))+elimi).*(1/samplef));
        acf = (apd_r_fixed(end)-apd_r_fixed(1)-t.*samplef)./((length(apd_r_fixed)-t.*countR).^2).*acf(elimi+1:end);
%         acf = (((apd_r_fixed(end)-apd_r_fixed(1))/((length(apd_r_fixed))^2)).*acf(elimi+1:end));
%         t = (((1:length(acf))+elimi).*(1/samplef));
    case 1 %autocorr
        acf = zeros(1,apd_r_fixed(end)-apd_r_fixed(1));
        max_lag = length(acf);
        for ai = 1:length(apd_r_fixed)
           for bi = (ai+1):length(apd_r_fixed)
               temp_delta = apd_r_fixed(bi)-apd_r_fixed(ai) ;
               if (temp_delta > 0) && (temp_delta <= max_lag)
                   acf(temp_delta) = acf(temp_delta)+1;
               else
                   break;
               end
           end
        end
        t = (1:length(acf));
        
    case 5 %xcorr
        apd_r_shift = varargin{1};
        max_lag = varargin{2};
        samplef = varargin{3};
        elimi = varargin{4};
        acf = zeros(1,max_lag);
        bi_start = 1;
        for ai = 1:length(apd_r_shift)
            notset = true;
           for bi = bi_start:length(apd_r_fixed)
               temp_delta = apd_r_fixed(bi)-apd_r_shift(ai) ;
               if (temp_delta > 0)
                   if (temp_delta <= max_lag)
                        acf(temp_delta) = acf(temp_delta)+1;
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
        
        countR_fixed = length(apd_r_fixed)/((apd_r_fixed(end)-apd_r_fixed(1))/samplef);%countR
        countR_shift = length(apd_r_shift)/((apd_r_shift(end)-apd_r_shift(1))/samplef);%countR
        varargout{1} = countR_fixed;
        varargout{2} = countR_shift;
        acf = (max([(apd_r_fixed(end)-apd_r_fixed(1)) (apd_r_shift(end)-apd_r_shift(1))])/(length(apd_r_shift)*length(apd_r_fixed))).*acf(elimi+1:end);
        t = (((1:length(acf))+elimi).*(1/samplef));
    case 2 %xcorr
        apd_r_shift = varargin{1};
        smallest = (apd_r_fixed(1))-(apd_r_shift(end));
        largest = ((apd_r_fixed(end))-(apd_r_shift(1)));
        acf = zeros(1,largest-smallest+1);
        for ai = 1:length(apd_r_shift)
           for bi = 1:length(apd_r_fixed)
               temp_delta = apd_r_fixed(bi)-apd_r_shift(ai) ;
               acf(temp_delta-smallest+1) = acf(temp_delta-smallest+1)+1;
           end
        end
        t = smallest:largest;
    otherwise
        error('not enough input')
end
% temp_t = toc;
% disp(['New Auto/XCorrelation takes ' num2str(temp_t) 'sec to calculate corr from ' num2str(length(apd_r_fixed)) ' photons'])
end