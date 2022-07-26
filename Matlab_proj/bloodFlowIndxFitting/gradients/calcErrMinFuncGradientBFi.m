function [dE1, dE2] = calcErrMinFuncGradientBFi(BFi, beta, g2data, tau, mua, musp, rho, lamda, n, alpha)

%
% Example:
%
%   g2data = load('g2data.txt');
%   tau = load('taus.txt');
%   [dE1, dE2] = calcErrMinFuncGradientBFi(1e-8, .45, g2data, tau, .1, 6, 2, 850e-7, 1.37, 1)
%

initCorrDiffusionEqParams(mua, musp, rho, lamda, n);

% dE1 = dEdBFi_1(BFi, beta, g2data, tau, mua, musp, rho, lamda, n, alpha);
dE1 = dEdBFi_1_cpp(BFi, beta, g2data, tau, mua, musp, rho, lamda, n, alpha);
dE2 = dEdBFi_2(BFi, beta, g2data, tau, mua, musp, rho, lamda, n, alpha);

