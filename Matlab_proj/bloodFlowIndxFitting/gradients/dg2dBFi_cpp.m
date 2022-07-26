function dg2dBFi = dg2dBFi_cpp(BFi, beta, tj)

global r1
global r2
global c2
global c3
global G1_0

K = sqrt(c2 + BFi*c3*tj);
dg2dBFi = ...
    -(2*beta*(exp(-r1*K)/r1 - exp(-r2*K)/r2) * ...
      ((c3*tj*exp(-r1*K))/(2*K) - ...
       (c3*tj*exp(-r2*K))/(2*K)) ) / (G1_0*G1_0);


