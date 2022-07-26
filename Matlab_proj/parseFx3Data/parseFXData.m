function [photcount, adccount, wrapcount, kk] = parseFXData(dataraw, wrapcount)

global dataDCS;
global dataADC;

nCh_dcs = dataDCS.nCh;
nCh_adc = dataADC.nCh;

photcount = dataDCS.count;
adccount = dataADC.count;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Definitions common to DCS and ADC
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
BIT_SHIFT_11 = uint16(2^11);
SRCMASK    = uint16(bin2dec('1111100000000000'));
PKTCNTMASK = uint16(bin2dec('0000011100000000'));
EVENTMASK  = uint16(bin2dec('0000000011111000'));
MACROMASK1 = uint16(bin2dec('0000000000000111'));
MACROMASK2 = uint16(bin2dec('0011111111111111'));
MICROMASK  = uint16(bin2dec('0111111111111111'));
BIT_SHIFT_14 = uint64(2^14);
MACRO_COUNTER_SIZE = uint64(2^17-1);

%%%%%%%%%%%%%%%%%%%%%%%%%%
% DCS definitions 
%%%%%%%%%%%%%%%%%%%%%%%%%%
MACROOVERFLOWMASK   = uint16(bin2dec('0000000010000000'));
STARTMASK           = uint16(bin2dec('0000000001000000'));
VALIDSTARTMASK      = uint16(bin2dec('0000000000100000'));
VALIDSTOPMASK       = uint16(bin2dec('0000000000010000'));
VALIDCONVERSIONMASK = uint16(bin2dec('0000000000001000'));

macrotimeDCS        = uint64(0);

%%%%%%%%%%%%%%%%%%%%%%%%%
% ADC definitions 
%%%%%%%%%%%%%%%%%%%%%%%%%
macrotimeADC     = uint64(0);
macrotimeADCPrev = uint64(0);


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Main data loop for parsing a chunk
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
word    = uint16(zeros(6,1));
kk      = 1;
while kk <= length(dataraw)
    
    word(1) = dataraw(kk);
    
    % src = bitshift(bitand(word(1), SRCMASK), -11); 
    % is a lot slower than this:
    src = bitand(word(1), SRCMASK) / BIT_SHIFT_11;

    if src<4
        
        %%%% Packet from DCS
       
        if kk+1>length(dataraw)
            break;
        end
              
        word(2) = dataraw(kk+1);
        
        % validconversion tells us if the packet is fast DCS or
        % time domain DCS
        validconversion = bitand(word(1), VALIDCONVERSIONMASK);
        if validconversion == VALIDCONVERSIONMASK
            if kk+2>length(dataraw)
                break;
            end
            word(3) = dataraw(kk+2);
        end
        
        % Get macro count
        macrotimeDCS = bitor( uint64(bitand(word(1), MACROMASK1)) * BIT_SHIFT_14,  uint64(bitand(word(2), MACROMASK2)) );
        
        % Check for counter overflow event
        if (bitand(word(1),MACROOVERFLOWMASK) == MACROOVERFLOWMASK) & (macrotimeDCS == 0)
            wrapcount(src+1) = wrapcount(src+1)+1;
        end

        % Check for photon arrival event           
        if bitand(word(1), STARTMASK) == STARTMASK
            
            % Get macro count
            dataDCS.arrtimes{src+1}(photcount(src+1)+1,1) = (MACRO_COUNTER_SIZE * wrapcount(src+1) + wrapcount(src+1)) + macrotimeDCS;
            
            % Get micro count
            if validconversion == 2^3
                dataDCS.arrtimes{src+1}(photcount(src+1)+1,2) = bitand(word(3), MICROMASK);
            end
            
            % Increment number photon packets received
            photcount(src+1) = photcount(src+1)+1;
            
        end
                
        % Skip to next packet
        if validconversion == VALIDCONVERSIONMASK
            kk=kk+3;
        elseif validconversion == 0
            kk=kk+2;
        end
      
    elseif src==4

        %%%% Packet from ADC
        if kk+6>length(dataraw)
            break;
        end        
        
        if bitand(dataraw(kk+1),(2^14))==0
            kk=kk+1;
            continue;
        end

        word(2) = dataraw(kk+1);
        word(3) = dataraw(kk+2);
        word(4) = dataraw(kk+3);
        word(5) = dataraw(kk+4);
        word(6) = dataraw(kk+5);
        
        macrotimeADCPrev = macrotimeADC;
        
        % Get macro count
        macrotimeADC = bitor( uint64(bitand(word(1), MACROMASK1)) * BIT_SHIFT_14,  uint64(bitand(word(2), MACROMASK2)) );
        if macrotimeADC < macrotimeADCPrev
            wrapcount(src+1) = wrapcount(src+1)+1;
        end
        dataADC.t(adccount+1,1) = (MACRO_COUNTER_SIZE * wrapcount(src+1) + wrapcount(src+1)) + macrotimeADC;

        % Get ADC data and increment number ADC packets received
        for hh=1:nCh_adc
            dataADC.amp(adccount+1,hh) = word(dataADC.chIdxOrder(hh)+1);
        end
              
        % Skip to next packet
        kk=kk+6;
        
		adccount = adccount+1;

    else
        
        %%%% Unknown packet type
       
        % Skip to next packet
        kk=kk+1;
            
    end
end

for ii=1:nCh_dcs
    photcount(ii) = photcount(ii)-dataDCS.count(ii);
    dataDCS.count(ii) = dataDCS.count(ii)+photcount(ii);
end
adccount = adccount-dataADC.count;
dataADC.count = dataADC.count+adccount;

