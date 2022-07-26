function dg2dB = dg2dB_cpp(BFi, tj)

global r1
global r2
global c2
global c3
global G1_0

K = sqrt(c2 + BFi*c3*tj);
dg2dB = (exp(-r1*K)/r1 - exp(-r2*K)/r2)^2/(G1_0*G1_0);


