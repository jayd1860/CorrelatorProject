function [dg2_1, dg2_2] = calcG2FuncGradientBeta(BFi, tau, mua, musp, rho, lamda, n, alpha)

%
% Example:
%
%   g2data = load('g2data.txt');
%   tau = load('taus.txt');
%   [dE1, dE2] = calcErrMinFuncGradientBFi(1e-8, .45, g2data, tau, .1, 6, 2, 850e-7, 1.37, 1)
%

initCorrDiffusionEqParams(mua, musp, rho, lamda, n, alpha);

for j=1:length(tau)
    dg2_1(j) = dg2dB_matlab(BFi, tau(j));
    dg2_2(j) = dg2dB_cpp(BFi, tau(j));
end
