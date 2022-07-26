function [E, BFi, B] = getSymErrFunc()

BFi = sym('BFi');
B   = sym('B');
dj  = sym('dj');
tj  = sym('tj');
r1  = sym('r1');
r2  = sym('r2');
c2  = sym('c2');
c3  = sym('c3');
c4  = sym('c4');

E(BFi, B) = (dj - (1 + B*((1/(c4*r1)) * exp(-r1*(c2+c3*tj*BFi)^(1/2)) - (1/(c4*r2)) * exp(-r2*(c2+c3*tj*BFi)^(1/2)))^2) )^2;
%E(BFi, B) = (dj - (1 + B*( ((exp(-r1*(c2+c3*tj*BFi)^(1/2))/r1 - exp(-r2*(c2+c3*tj*BFi)^(1/2))/r2) / c4) ^ 2) ))^2;
