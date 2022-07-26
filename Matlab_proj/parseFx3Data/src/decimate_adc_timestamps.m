function adc_timestamps_values = decimate_adc_timestamps(adc_timestamps_values_in,options)

if ~exist('options','var') || nargin < 2
    options = [];
end

if ~isfield(options,'target_fs_adc')
    options.target_fs_adc = 5e3; 
end


fs_ts = 150e6;

fs_adc_in = fs_ts/(adc_timestamps_values_in(2,1)-adc_timestamps_values_in(1,1));

M = fs_adc_in/options.target_fs_adc;
if rem(M,1) == 0 && M == 1
    disp(['M = 1; nothing to do']);
    adc_timestamps_values = adc_timestamps_values_in;
    return;
elseif rem(M,1) == 0
    disp(['Input ADC sampling freq: ' num2str(fs_adc_in/1000) 'kHz, Output ADC sampling freq: ' num2str(options.target_fs_adc/1000) ' kHz']);
else
    disp(['Error: Input sampling frequency not multiple of target sampling frequency']);
    adc_timestamps_values = [];
    return;
end


% lpFilt = designfilt('lowpassfir', 'PassbandFrequency', 0.7*0.5/M, 'StopbandFrequency', 0.5/M, 'PassbandRipple', 0.25, 'StopbandAttenuation', 50, 'DesignMethod', 'equiripple');
lpFilt = designfilt('lowpassiir', 'FilterOrder', 6, 'HalfPowerFrequency', 0.7*1/M);
adc_timestamps_values = zeros(size(adc_timestamps_values_in(1:M:end,:)));
adc_timestamps_values(:,1) = adc_timestamps_values_in(1:M:end,1);
tmp_values = filtfilt(lpFilt,adc_timestamps_values_in(:,2:end));
adc_timestamps_values(:,2:end) = tmp_values(1:M:end,:);


