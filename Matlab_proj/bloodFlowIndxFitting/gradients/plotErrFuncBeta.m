function [beta, err, derr] = plotErrFuncBeta(BFi, g2data, tau, mua, musp, rho, lamda, n, alpha)

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

beta_min = 0;
beta_max = .5;
nbeta      = 1000;
incrBeta  = (beta_max - beta_min) / nbeta;
beta  = beta_min : incrBeta : beta_max; 

B = beta;

err = [];
derr = [];
for ii=1:length(beta)
    err(ii) = errFunc(BFi, B(ii), d, t, c2, c3, c4);
    derr(ii) = dEdB_cpp(BFi, B(ii), d, t, mua, musp, rho, lamda, n, alpha);
end

figure; plot(B, err);
figure; plot(B, derr);




