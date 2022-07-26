function [BFi, beta, err] = fit_examples(g2data, taus, mua, musp, rho, lamda, n, alpha)
%
% Example:
%    nDiscard = 40;
%    [tau, g2data] = readCorrData('c:/users/public/DCS/CollectData_Jay_run1.bin.corr');
%    [BFi, beta] = fit_examples(g2data(nDiscard:end,2,2), tau(nDiscard:end), .1, 6, 2, 850e-7, 1.37, 1);
%

PLOT = 0;
err = 0;

%%% set fitting parameters

% set-up model parameters that are assumed known
% parameters depending on length scales must use consistent units
% i.e. separation in cm, wavelength in cm, BFi in cm^2/s

param.mua    = mua;     % absorption coefficient (cm^-1)
param.musp   = musp;    % reduced scattering coefficient (cm^-1)
param.rho    = rho;     % source-detector separation (cm)
param.lambda = lamda;   % wavelength in cm, i.e. 850 nm = 850 x 1e-9 m = 850 x 1e-7 cm
param.n      = n;       % index of refraction
param.alpha  = alpha;   % probability of scattering from a moving scatterer, standard assumption 100%

% the following are calculated from the ones above

param.z0 = 1/(param.mua + param.musp);

R        = -1.440./param.n^2 + 0.710/param.n + 0.668 + 0.0636*param.n; % Effective reflection coefficient
param.zb = 2/3*(1+R)/(1-R)/param.musp;       % Extrapolated boundary is given by ze divided by musp

param.r1 = sqrt(param.rho^2 + param.z0^2);
param.r2 = sqrt(param.rho^2 + (param.z0 + 2*param.zb)^2);
param.k0 = 2*pi*param.n/param.lambda;

% pack correlation times into parameters as well for convenience

param.tau = taus;

%%% full g2 fit, i.e. BFi and beta

BFi_initial_guess  = 1e-8;
beta_initial_guess = 0.45;
fit_result = fminsearch(@(x) dcs_g2_cost_function(x, taus, g2data, param), [BFi_initial_guess, beta_initial_guess]);
BFi  = fit_result(1);
beta = fit_result(2);

if PLOT
    figure;
    semilogx(taus, g2data,'*');
    hold on
    semilogx(taus, dcs_g2_model_function(fit_result,param), 'r');
    title(sprintf('BFi = %.3g,  beta = %.3f', BFi, beta));
end

