function [BFi, err, derr] = plotErrFuncBFi(beta, g2data, tau, mua, musp, rho, lamda, n, alpha)

% Example:
%
%    plotErrFuncBFi(.45, g2data, tau, .1, 6, 2, 850e-7, 1.37, 1)
%  
%
%

global r1
global r2
global k0

initCorrDiffusionEqParams(mua, musp, rho, lamda, n);

c2 = 3*mua*musp;
c3 = k0*k0*6*alpha*musp*musp;
c4 = exp(-r1*sqrt(c2))/r1 - exp(-r2*sqrt(c2))/r2;

t = tau;
d = g2data;

BFi_min = 5e-11;
BFi_max = 3e-8;
nBFi      = 1000;
incrBFi  = (BFi_max - BFi_min) / nBFi;
BFi  = BFi_min : incrBFi : BFi_max; 

B = beta;

err = [];
derr = [];
for ii=1:length(BFi)
    err(ii) = errFunc(BFi(ii), B, d, t, c2, c3, c4);
    derr(ii) = dEdBFi_1_cpp(BFi(ii), B, d, t, mua, musp, rho, lamda, n, alpha);
end

figure; plot(BFi, err);
figure; plot(BFi, derr);

