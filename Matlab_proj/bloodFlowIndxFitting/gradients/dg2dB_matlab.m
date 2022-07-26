function dg2dB = dg2dB_matlab(BFi, tj)

global r1
global r2
global c2
global c3
global G1_0

dg2dB = (exp(-r1*(c2 + BFi*c3*tj)^(1/2))/r1 - exp(-r2*(c2 + BFi*c3*tj)^(1/2))/r2)^2/G1_0^2;


