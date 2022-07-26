function [vset, vset_idx, start, start_idx] = create_vset2(eventtable, timestamps)

nch = length(eventtable);

for isrc = 1:nch
    disp(['finding start stop valid_conversion triplets isrc=' num2str(isrc)]);
    nvsets = sum(eventtable{isrc}(:,4));
    vset_idx{isrc} = zeros(nvsets,3);
    vset{isrc} = zeros(nvsets,4);
    start{isrc} = zeros(nvsets,1);
    start_idx{isrc} = zeros(nvsets,1);
    
    if nvsets < 5
        % channel was probably not active
        continue
    end
    vset_idx{isrc}(:,3) = find(eventtable{isrc}(:,4));
    for iv = 1:nvsets
        vstopidx = vset_idx{isrc}(iv,3);
        while vstopidx > 0 && (eventtable{isrc}(vstopidx,3) == false || ((timestamps{isrc}(vset_idx{isrc}(iv,3),1)-timestamps{isrc}(vstopidx,1))<20))
            vstopidx = vstopidx -1;
%             if (eventtable{isrc}(stopidx,2) || eventtable{isrc}(stopidx,4)) && eventtable{isrc}(stopidx,3) == false
%                 disp(['Warning: VStart, VStop, Valid out of order. isrc=' num2str(isrc) ' iv=' num2str(iv)]);
%             end
        end
        if vstopidx > 0
            vset_idx{isrc}(iv,2) = vstopidx;
        else
            vset_idx{isrc}(iv,2) = 1;
            vstopidx = 1;
        end
        vstartidx = vstopidx;
        while vstartidx > 0 && eventtable{isrc}(vstartidx,2) == false %|| ((timestamps{isrc}(vset_idx(iv,3),1)-timestamps{isrc}(stopidx,1))<10))
            vstartidx = vstartidx -1;
%             if (eventtable{isrc}(startidx,3) || eventtable{isrc}(startidx,4)) && eventtable{isrc}(startidx,2) == false
%                 disp(['Warning2: VStart, VStop, Valid out of order. isrc=' num2str(isrc) ' iv=' num2str(iv)]);
%             end
        end
        if vstartidx > 0
            vset_idx{isrc}(iv,1) = vstartidx;
        else
            vset_idx{isrc}(iv,1) = 1;
            vstartidx = 1;
        end
        
        startidx = vstartidx;
        while startidx > 0 && eventtable{isrc}(startidx,1) == false %|| ((timestamps{isrc}(vset_idx(iv,3),1)-timestamps{isrc}(stopidx,1))<10))
            startidx = startidx -1;
        end
        if startidx > 0
            start_idx{isrc}(iv,1) = startidx;
        else
            start_idx{isrc}(iv,1) = 1;
            startidx = 1;
        end
        
        
%         if abs((timestamps{isrc}(stopidx,1)-timestamps{isrc}(startidx,1))-(timestamps{isrc}(vset_idx{isrc}(iv,3),2)/682.6667)) > 1.2
%             dt = abs((timestamps{isrc}(stopidx,1)-timestamps{isrc}(startidx,1))-(timestamps{isrc}(vset_idx{isrc}(iv,3),2)/682.6667));
%             disp(['Warning: Macro and Micro time mismatch by ' num2str(dt,3) ' macroperiods at isrc=' num2str(isrc) ' iv=' num2str(iv)]);
%         end
        
    end
    
    vset{isrc}(:,1) = timestamps{isrc}(vset_idx{isrc}(:,1),1);
    vset{isrc}(:,2) = timestamps{isrc}(vset_idx{isrc}(:,2),1);
    vset{isrc}(:,3) = timestamps{isrc}(vset_idx{isrc}(:,3),1);
    vset{isrc}(:,4) = timestamps{isrc}(vset_idx{isrc}(:,3),2); 
    
    start{isrc}(:,1) = timestamps{isrc}(start_idx{isrc}(:,1),1);
end