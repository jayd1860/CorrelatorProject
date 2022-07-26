function dEdB = dEdB_cpp(BFi, beta, g2data, tau, mua, musp, rho, lamda, n, alpha)

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

dEdB = 0;
for j=1:length(tau)

    %K = sqrt(c2 + BFi*c3*t(j));
    K = sqrt(c2 + BFi*c3*t(j));
    dEdB = dEdB + ...
            2*(exp(-r1*K)/(c4*r1) - exp(-r2*K)/(c4*r2))^2 * ...
            (B*(exp(-r1*K)/(c4*r1) - exp(-r2*K)/(c4*r2))^2 - d(j) + 1);
end

