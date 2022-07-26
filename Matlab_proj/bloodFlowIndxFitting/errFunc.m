function err = errFunc(BFi, B, d, t, c2, c3, c4)

global r1
global r2
global k0

err = 0;
for j=1:length(t)
    err = err + (d(j) - (1 + B*((1/(c4*r1)) * exp(-r1*(c2+c3*t(j)*BFi)^(1/2)) - (1/(c4*r2)) * exp(-r2*(c2+c3*t(j)*BFi)^(1/2)))^2) )^2;
end

dbg = 1;

