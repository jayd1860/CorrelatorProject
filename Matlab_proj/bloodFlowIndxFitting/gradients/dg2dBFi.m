function dg2dBFi = dg2dBFi_1(BFi, beta, g2data, tau, mua, musp, rho, lamda, n, alpha)

global r1
global r2
global k0

initCorrDiffusionEqParams(mua, musp, rho, lamda, n);

B = beta;
c2 = 3*mua*musp;
c3 = k0*k0*6*alpha*musp*musp;
c4 = exp(-r1*c2^(1/2))/r1 - exp(-r2*c2^(1/2))/r2;

t = tau;
d = g2data;

dg2dBFi = 0;
for j=1:length(tau)

    dg2dBFi(j) = ...
        -(2*B*(exp(-r1*(c2 + BFi*c3*tj)^(1/2))/r1 - exp(-r2*(c2 + BFi*c3*tj)^(1/2))/r2) * ...
         ((c3*tj*exp(-r1*(c2 + BFi*c3*tj)^(1/2)))/(2*(c2 + BFi*c3*tj)^(1/2)) - ...
          (c3*tj*exp(-r2*(c2 + BFi*c3*tj)^(1/2)))/(2*(c2 + BFi*c3*tj)^(1/2))) ) / c4^2;
end
