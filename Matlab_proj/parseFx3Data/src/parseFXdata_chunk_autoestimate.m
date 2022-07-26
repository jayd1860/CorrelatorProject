function [timestamps, eventtable, nevents, adc_timestamps_values] = parseFXdata_chunk_autoestimate(fname, options)

if ~exist('options','var') || nargin < 2
    options = [];
end
if ~isfield(options,'memavailable')
%     [~, sys] = memory;
    options.memavailable = 6e9; %floor(0.75*sys.PhysicalMemory.Total);
    % memavailable = 1e9 will result in approximately 250 MB large files
    %(depends on data compressibility)
end

options.keepADC = true;
toks = strsplit(fname,'.');

ds = dir(fname);
if ds.bytes < 100e6
    options.nwords = floor(ds.bytes /2);
    options.adc_nevent_estimate = ceil(options.nwords/6);
    options.nevent_estimate = ceil(options.nwords/2).*ones(4,1);
    [timestamps, eventtable, nevents, adc_timestamps_values] = parseFXdata_chunk2(fname, options);
    [vset, vset_idx] = create_vset2(eventtable, timestamps);
%     adc_timestamps_values = decimate_adc_timestamps(adc_timestamps_values_in,options);
    disp(['Writing ' toks{1} '_1of1.mat']);
    save([toks{1} '_1of1.mat'],'timestamps','eventtable','nevents','adc_timestamps_values','vset','vset_idx','-v7.3');
else
    options.nwords = 1e6;
    options.adc_nevent_estimate = ceil(options.nwords/6);
    options.nevent_estimate = ceil(options.nwords/2).*ones(4,1);
    disp('Estimating required matrix sizes');
    [~, ~, nevents, adc_timestamps_values] = parseFXdata_chunk2(fname, options);
    tmult = 1.25*floor(ds.bytes /2)/options.nwords;
    memest = sum(ceil(nevents(:,5).*tmult))*(8*2+1*4) + ceil(length(adc_timestamps_values)*tmult)*5*8;
    disp(['Estimated memory usage: ' num2str(memest/1e9,4) ' GB']);
    nseg = ceil(memest/options.memavailable);
    disp(['Splitting file into ' num2str(nseg) ' segments with memory usage of ' num2str(memest/1e9/nseg,4) ' GB']);
    options.nwords = floor(ds.bytes /2 /nseg);
    options.nevent_estimate = ceil(nevents(:,5).*tmult/nseg);
    options.adc_nevent_estimate = ceil(length(adc_timestamps_values)*tmult/nseg);
    options.wrapcnt = zeros(4,1);
    options.ADCwrapcnt = [0, 0];
    options.offset = 0;
    for iseg = 1:nseg
        disp('**************************');
        disp(['Running iseg = ' num2str(iseg) ' / ' num2str(nseg)]);
        disp('**************************');
        [timestamps, eventtable, nevents, adc_timestamps_values, wrapcnt, ADCwrapcnt, words_not_read] = parseFXdata_chunk2(fname, options);
        disp(['words not read: ' num2str(words_not_read)]);
        options.offset = options.offset + options.nwords*2 - words_not_read*2;
        options.wrapcnt = wrapcnt;
        options.ADCwrapcnt = ADCwrapcnt;
        [vset, vset_idx] = create_vset2(eventtable, timestamps);
%         adc_timestamps_values = decimate_adc_timestamps(adc_timestamps_values_in,options);
        disp(['Writing ' toks{1} '_' num2str(iseg) 'of' num2str(nseg) '.mat']);
        save([toks{1} '_' num2str(iseg) 'of' num2str(nseg) '.mat'],'timestamps','eventtable','nevents','adc_timestamps_values','vset','vset_idx','-v7.3');
    end
end
