function dg2dBFi = dg2dBFi_matlab(BFi, beta, tj)

global r1
global r2
global c2
global c3
global G1_0

B = beta;

dg2dBFi = ...
    -(2*beta*(exp(-r1*(c2 + BFi*c3*tj)^(1/2))/r1 - exp(-r2*(c2 + BFi*c3*tj)^(1/2))/r2) * ...
      ((c3*tj*exp(-r1*(c2 + BFi*c3*tj)^(1/2)))/(2*(c2 + BFi*c3*tj)^(1/2)) - ...
       (c3*tj*exp(-r2*(c2 + BFi*c3*tj)^(1/2)))/(2*(c2 + BFi*c3*tj)^(1/2))) ) / G1_0^2;
