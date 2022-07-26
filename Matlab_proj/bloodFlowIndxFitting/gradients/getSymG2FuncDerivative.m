function dE = getSymG2FuncDerivative()

[g2, BFi, B] = getSymG2(); 

dG2dBFi = diff(g2, BFi);
dG2dB   = diff(g2, B);

dG2(1)  = dEdBFi;
dG2(2)  = dEdB;

