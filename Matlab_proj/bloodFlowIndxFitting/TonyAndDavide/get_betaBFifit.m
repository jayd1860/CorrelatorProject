function [beta, BFi, fit_curve, t] = get_betaBFifit(acf, t, mua, musp, rho, lamda, n, varargin)
%
switch nargin
    case 8
        max_acf = max(acf); min_acf = min(acf);
        ratio_low = varargin{1};
        thres_low = min_acf + (max_acf-min_acf)*ratio_low;
        for i = length(acf):-1:1
            if acf(i) >= thres_low
                t = t(1:i);
                acf = acf(1:i);
                break;
            end
        end
    case 9
        max_acf = max(acf); min_acf = min(acf);
        ratio_low = varargin{1};
        ratio_high = varargin{2};
        thres_low = min_acf + (max_acf-min_acf)*ratio_low;
        thres_high = min_acf + (max_acf-min_acf)*ratio_high;
        for i = length(acf):-1:1
            if acf(i) >= thres_low
                t = t(1:i);
                acf = acf(1:i);
                break;
            end
        end
        for i = 1:length(acf)
            if acf(i) <= thres_high
                t = t(i:end);
                acf = acf(i:end);
                break;
            end
        end
    otherwise
end

%~~~~~~~~~~~~~~~~~~~!~~~~~~~~~~~~~~~~~~~~!
k0 = 2*pi/lamda; % Free-space wavenumber
k = k0*n; % Wavenumber in tissue
ltr = 1/(mua + musp); % Transport mean-free path
Reff = -1.44*n^-2+0.71/n+0.00636*n+0.668; % Effective reflection coefficient
z0 = ltr;
zb = 2/3*ltr*(1+Reff)/(1-Reff);
r1 = sqrt(rho^2+z0^2);
r2 = sqrt(rho^2 + (z0+2*zb)^2);

%~~~~~~~~~~~~~~~~~~~!~~~~~~~~~~~~~~~~~~~~!
options = optimoptions('lsqnonlin','Display','off');
%
x0 = [0.5,3.2];
tau0 = 1/150e6;%t(1);%
K = @(tau,BFi)sqrt(3*mua*musp + musp^2*k^2 * 6*BFi*tau);
g1 = @(tau,BFi)(exp(-K(tau,BFi)*r1)/r1 - exp(-K(tau,BFi)*r2)/r2) / (exp(-K(tau0,BFi)*r1)/r1 - exp(-K(tau0,BFi)*r2)/r2);

fun0 = @(x)(1+x(1).*(abs(g1(t,x(2).*1e-9)).^2)-acf);
[fixed_x,~] = lsqnonlin(fun0,x0,[],[],options);
beta = fixed_x(1);
BFi = fixed_x(2)*1e-9;
fit_curve = fun0(fixed_x) + acf;

