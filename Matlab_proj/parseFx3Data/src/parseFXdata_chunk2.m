function [timestamps, eventtable, nevents, adc_timestamps_values, wrapcnt, ADCwrapcnt, words_not_read] = parseFXdata_chunk2(fname, options)

nch = 4;
nadc = 4;
chunk_read_len = 5e5;

if ~exist('options','var') || nargin < 2
    options = [];
end
if ~isfield(options,'keepADC')
    options.keepADC = true;
end
if ~isfield(options,'adc_nevent_estimate')
    options.adc_nevent_estimate = 1e7;
end
if ~isfield(options,'nevent_estimate')
    options.nevent_estimate = 1e7.*ones(4,1);
    disp('Warning: default estimate probably not accurate');
end 
if ~isfield(options,'nwords')
    ds = dir(fname);
    options.nwords = 1e6;%ds.bytes /2;  
end
if ~isfield(options,'offset')
    options.offset = 0;
end
if ~isfield(options,'ADCwrapcnt')
    options.ADCwrapcnt = [0, 0];
    ADCwrapcnt = [0, 0];
else
    ADCwrapcnt = options.ADCwrapcnt;
end
if ~isfield(options,'wrapcnt')
    options.wrapcnt = zeros(nch,1);
    wrapcnt = zeros(nch,1);
else
    wrapcnt = options.wrapcnt;
end



w_nevents = zeros(nch,6);
pktcntmask = cast(bin2dec('0000011100000000'),'uint16');
srcmask    = cast(bin2dec('0011100000000000'),'uint16');
macromask1 = cast(bin2dec('0000000000000111'),'uint16');
macromask2 = cast(bin2dec('0011111111111111'),'uint16');
micromask  = cast(bin2dec('0111111111111111'),'uint16');


% allocate memory
for ii = 1:nch
    eventtable{ii} = false(5,options.nevent_estimate(ii));
    timestamps{ii} = zeros(options.nevent_estimate(ii),2);
end
if options.keepADC
    w_adc_timestamps_values = zeros(options.adc_nevent_estimate,nadc+1);
else
    w_adc_timestamps_values = zeros(1,nadc+1);
end

fid = fopen(fname);
fseek(fid,options.offset,'bof'); % offset for multi file parsing
Acrop = fread(fid,chunk_read_len,'uint16=>uint16');
read_cnt = chunk_read_len;
% the two most significant bits in the first word have to be zero
ii = 1;
twomsbitsw1 = bitshift(Acrop(ii),-14);
if twomsbitsw1 ~= 0
    disp(['Warning: Wordshift at ii = ' num2str(ii)]);
    ii = ii +1;
end
twomsbitsw1 = bitshift(Acrop(ii),-14);
if twomsbitsw1 ~= 0
    disp(['Warning: Wordshift 2 at ii = ' num2str(ii)]);
    ii = ii +1;
end

ievent = ones(nch+1,1);
pktcnt = mod(mod(bitshift(Acrop(ii),-8),8)+7,8);
disp('Checking data integrity, getting event counts, building eventtable and timestamps');
curr_chunk_read_len = chunk_read_len;
for ichunk = 1:(ceil(options.nwords/chunk_read_len)+1)
    if ichunk ~= 1
        words_not_read = curr_chunk_read_len - ii +1;
        fseek(fid,-words_not_read*2,0);
        read_cnt = read_cnt - words_not_read;
        curr_chunk_read_len = min(chunk_read_len, options.nwords - read_cnt);
        if curr_chunk_read_len <= 0
            ii = 1;
            break;
        end
        Acrop = fread(fid,curr_chunk_read_len,'uint16=>uint16');
        read_cnt = read_cnt + curr_chunk_read_len;
        disp(['Read ichunk = ' num2str(ichunk) ' / ' num2str(ceil(options.nwords/chunk_read_len)+1)]);
        ii = 1;
    end
    % precompute for speed
    twomsbitsw1 = bitshift(Acrop,-14);
    newpktcnt = bitshift(bitand(Acrop,pktcntmask),-8);
    srcv = bitshift(bitand(Acrop,srcmask),-11) +1;
    evbits = 8:-1:4;
    eventtablev = zeros(length(evbits),length(Acrop));
    for ib = 1:length(evbits)
        eventtablev(ib,:) = bitget(Acrop,evbits(ib))';
    end
    while ii+5 <= length(Acrop)      
        if twomsbitsw1(ii) ~= 0
            disp(['Warning: Two most significant bits in w1 not as expected at ii = ' num2str(ii)]);
            ii = ii +1;
            continue;
        end         
        if ~newpktcnt(ii) == (pktcnt+1) && (~newpktcnt(ii) == 0 || ~pktcnt == 7)
            disp(['Warning: Packet counter not as expected at ii = ' num2str(ii)]);
        end
        pktcnt = newpktcnt(ii);
        src = srcv(ii);
        if src <= 4 % coming from TDC cards
            if twomsbitsw1(ii+1) ~= 1
                disp(['Warning: Two most significant bits in w2 not as expected at ii = ' num2str(ii)]);
                ii = ii +1;
                continue;
            end

            eventtable{src}(:,ievent(src)) = eventtablev(:,ii); %[OvflEvent, StartEvent, ValidStart, StopEvent, ValidEvent]
    %         if ~any(eventtable(ievent,:))
    %             disp(['Warning: No event at ii = ' num2str(ii)]);
    %         end        
            macrotime = double(bitand(Acrop(ii),macromask1))*(2^14) + double(bitand(Acrop(ii+1),macromask2)); 
            timestamps{src}(ievent(src),1) = macrotime;
            if  eventtablev(5,ii)% valid conversion, read third word
                msbitsw3 = bitget(Acrop(ii+2),16)==1;
                if msbitsw3 ~= 1
                    disp(['Warning: Most significant bit in w3 not as expected at ii = ' num2str(ii)]);
                    ii = ii +2;
                    continue;
                end
                microtime = bitand(Acrop(ii+2),micromask);
                timestamps{src}(ievent(src),2) = microtime;
                ii = ii + 3;
            else
                ii = ii + 2;
            end
            ievent(src) = ievent(src) +1;
        elseif src == 5 % coming from ADCs
            if options.keepADC
                macrotime = double(bitand(Acrop(ii),macromask1))*(2^14) + double(bitand(Acrop(ii+1),macromask2));
                w_adc_timestamps_values(ievent(src),1) = macrotime;
                for iadc = 1:nadc
                    w_adc_timestamps_values(ievent(src),iadc+1) = Acrop(ii+iadc+1);
                end
            end
            ii = ii + nadc + 2;
            ievent(src) = ievent(src) +1;
        else
            disp(['Warning: Source not as expected at ii=' num2str(ii)]);
            ii = ii +1;
        end
    end
end
words_not_read = curr_chunk_read_len - ii +1;
fclose(fid);
nevents = zeros(nch,5);
for src = 1:nch
    eventtable{src} = eventtable{src}';
    % calculate number of events
    w_nevents(src,1) = sum(eventtable{src}(:,1));
    w_nevents(src,2) = sum(eventtable{src}(:,2));
    w_nevents(src,3) = sum(eventtable{src}(:,3));
    w_nevents(src,4) = sum(eventtable{src}(:,4));
    w_nevents(src,5) = sum(eventtable{src}(:,5));
    w_nevents(src,6) = ievent(src)-1;
    
    %crop over-allocated parts
    eventtable{src} = eventtable{src}(1:w_nevents(src,6),:);
    timestamps{src} = timestamps{src}(1:w_nevents(src,6),:);
    
    % correct macrotimes for counter wrapping, and remove overflow events
    nevents(src,1:4) = w_nevents(src,2:5); 
    nevents(src,5) = sum(any(eventtable{src}(:,2:5),2));

    disp(['Dewrapping timestamps and removing overflow events isrc=' num2str(src)]);
    for iw = 1:length(eventtable{src})
        if eventtable{src}(iw,1) && timestamps{src}(iw,1) == 0 % macro counter overflow
            wrapcnt(src) = wrapcnt(src) + 2^17;
        end
        timestamps{src}(iw,1) = timestamps{src}(iw,1) + wrapcnt(src);
    end

    sel = any(eventtable{src}(:,2:5),2);
    eventtable{src} = eventtable{src}(sel,2:end);
    timestamps{src} = timestamps{src}(sel,:);
end

% correct ADC macrotimes for counter wrapping
if options.keepADC
    w_adc_timestamps_values = w_adc_timestamps_values(1:(ievent(5)-1),:);
    adc_timestamps_values = zeros(size(w_adc_timestamps_values));
    adc_timestamps_values(:,2:end) = w_adc_timestamps_values(:,2:end);
    disp('Dewrapping ADC timestamps');
    if ADCwrapcnt(2) > w_adc_timestamps_values(1,1) % macro counter overflow
            ADCwrapcnt(1) = ADCwrapcnt(1) + 2^17;
    end
    adc_timestamps_values(1,1) = w_adc_timestamps_values(1,1) + ADCwrapcnt(1);
    for iw = 2:length(w_adc_timestamps_values)
        if w_adc_timestamps_values(iw-1,1) > w_adc_timestamps_values(iw,1) % macro counter overflow
            ADCwrapcnt(1) = ADCwrapcnt(1) + 2^17;
        end
        adc_timestamps_values(iw,1) = w_adc_timestamps_values(iw,1) + ADCwrapcnt(1);
    end
    ADCwrapcnt(2) = w_adc_timestamps_values(end,1); % needed to detect wrap at segment boundary
else
    adc_timestamps_values = [];
end
