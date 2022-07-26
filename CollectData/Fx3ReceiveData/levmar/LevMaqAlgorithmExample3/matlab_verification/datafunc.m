function y = datafunc(x)

global a
global b
global c
global d
global e

for ii=1:length(x)
    r = .002*(rand()-.5);
    y(ii) = a*x(ii)^4 - b*x(ii)^3 + c*x(ii)^2 - d*x(ii) + e + r;
end
