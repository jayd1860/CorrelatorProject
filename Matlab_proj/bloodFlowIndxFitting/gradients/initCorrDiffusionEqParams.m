function initCorrDiffusionEqParams(mua, musp, rho, lamda, n)

global r1
global r2
global k0
global g1norm

R     = -1.440/(n*n) + 0.710/n + 0.668 + 0.0636*n; % Effective reflection coefficient
z0    = 1/(mua + musp);
zb    = 2/3*(1+R)/(1-R)/musp;       % Extrapolated boundary is given by ze divided by musp
r1    = sqrt(rho*rho + z0*z0);
r2    = sqrt(rho*rho + (z0 + 2*zb)*(z0 + 2*zb));
k0    = 2*pi*n/lamda;
knorm = sqrt(3*musp*mua); % k at tau=0 for normalization
g1norm = exp(-knorm*r1)/r1 - exp(-knorm*r2)/r2;

