function g2 = G2(BFi, beta, tau, mua, musp, rho, lamda, n, alpha)

R     = -1.440./n^2 + 0.710/n + 0.668 + 0.0636*n; % Effective reflection coefficient
z0    = 1/(mua + musp);
zb    = 2/3*(1+R)/(1-R)/musp;       % Extrapolated boundary is given by ze divided by musp
r1    = sqrt(rho^2 + z0^2);
r2    = sqrt(rho^2 + (z0 + 2*zb)^2);
k0    = 2*pi*n/lamda;
k     = sqrt(3*musp*mua + musp^2*k0^2*alpha*6*BFi*tau);

knorm = sqrt(3*musp*mua); % k at tau=0 for normalization

g1 = exp(-k*r1)/r1-exp(-k*r2)/r2;
g1norm = exp(-knorm*r1)/r1-exp(-knorm*r2)/r2;

g1 = g1/g1norm;
g2 = 1+beta*(g1.^2);

