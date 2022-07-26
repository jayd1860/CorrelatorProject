function dEdBFi = dEdBFi_1(BFi, beta, g2data, tau, mua, musp, rho, lamda, n, alpha)

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

dEdBFi = 0;
for j=1:length(tau)

    dEdBFi = dEdBFi + ...
              -((4*B*(exp(-r1*(c2 + BFi*c3*t(j))^(1/2))/r1 - exp(-r2*(c2 + BFi*c3*t(j))^(1/2))/r2) * ...
               ((c3*t(j)*exp(-r1*(c2 + BFi*c3*t(j))^(1/2)))/(2*(c2 + BFi*c3*t(j))^(1/2)) - ...
                (c3*t(j)*exp(-r2*(c2 + BFi*c3*t(j))^(1/2)))/(2*(c2 + BFi*c3*t(j))^(1/2))) * ...
               (((B*(exp(-r1*(c2 + BFi*c3*t(j))^(1/2))/r1 - exp(-r2*(c2 + BFi*c3*t(j))^(1/2))/r2)^2)/c4)^2 - d(j) + 1))/c4)^2;
end
