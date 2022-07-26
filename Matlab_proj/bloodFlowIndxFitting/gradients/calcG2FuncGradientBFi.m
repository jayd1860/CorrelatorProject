function [dg2_1, dg2_2] = calcG2FuncGradientBFi(BFi, beta, tau, mua, musp, rho, lamda, n, alpha)

%
% Example:
%
%   g2data = load('g2data.txt');
%   tau = load('taus.txt');
%   [dE1, dE2] = calcErrMinFuncGradientBFi(1e-8, .45, g2data, tau, .1, 6, 2, 850e-7, 1.37, 1)
%

initCorrDiffusionEqParams(mua, musp, rho, lamda, n, alpha);

% dE1 = dEdBFi_1(BFi, beta, g2data, tau, mua, musp, rho, lamda, n, alpha);

for j=1:length(tau)
    dg2_1(j) = dg2dBFi_matlab(BFi, beta, tau(j));
    dg2_2(j) = dg2dBFi_cpp(BFi, beta, tau(j));
end
