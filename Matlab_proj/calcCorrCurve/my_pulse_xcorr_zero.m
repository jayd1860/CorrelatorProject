function my_pulse_xcorr_zero(y, y_shift)

global maxlag;
global g;

g(:) = 0;
n1 = length(y);
n2 = length(y_shift);
c1 = (y(n1)-y(1)) / (n1 * n2);


if isempty(y)
    return;
end

h = waitbar(0,'Please wait...');
waitbar_update_interval = 1000;
 
for ii = 1:n1
   
   if mod(ii, waitbar_update_interval)==0
        waitbar(ii/length(y), h, sprintf('Completed %d of %d', ii, length(y)));
   end
   
   for jj = ii+1:n2
       k = y_shift(jj) - y(ii);
       
       if (k <= maxlag) && (k > 0)
           
           g(k+1) = g(k+1) + 1;
                                
       else
           
           break;
           
       end
   end
   

end

%g = ((y(end)-y(1)) / (n1*n2)) .* g;
for ii=1:maxlag
    g(ii) = c1 * g(ii);
end

close(h);

