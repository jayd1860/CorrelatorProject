function g2 = G2cpp(BFi, beta, tau, mua, musp, rho, lamda, n, alpha)

global r1
global r2
global k0
global g1norm

k = [];
g1 = [];
g2 = [];
for ii=1:length(tau)
    k(ii,1) = sqrt(3*musp*mua + musp^2*k0^2*alpha*6*BFi*tau(ii));
    g1(ii,1) = (exp(-k(ii)*r1)/r1 - exp(-k(ii)*r2)/r2) / g1norm;    
    g2(ii,1) = 1 + beta*(g1(ii)*g1(ii));
end



