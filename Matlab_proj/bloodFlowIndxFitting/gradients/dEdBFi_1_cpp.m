function dEdBFi = dEdBFi_1_cpp(BFi, beta, g2data, tau, mua, musp, rho, lamda, n, alpha)

global r1
global r2
global k0

initCorrDiffusionEqParams(mua, musp, rho, lamda, n);

B = beta;
c2 = 3*mua*musp;
c3 = k0*k0*6*alpha*musp*musp;
c4 = exp(-r1*sqrt(c2))/r1 - exp(-r2*sqrt(c2))/r2;

t = tau;
d = g2data;

dEdBFi = 0;
for j=1:length(tau)

    K = sqrt(c2 + BFi*c3*t(j));
    dEdBFi = dEdBFi + ...
              -(4*B*(exp(-r1*K)/r1 - exp(-r2*K)/r2) * ...
               ((c3*t(j)*exp(-r1*K))/(2*K) - ...
                (c3*t(j)*exp(-r2*K))/(2*K)) * ...
               ((B*( exp(-r1*K)/r1 - exp(-r2*K)/r2 ) * ( exp(-r1*K)/r1 - exp(-r2*K)/r2 ))/(c4*c4) - d(j) + 1))/(c4*c4);
end

