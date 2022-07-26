function dE = getSymErrFuncDerivative()

[E, BFi, B] = getSymErrFunc(); 

dEdBFi = diff(E, BFi);
dEdB   = diff(E, B);

dE(1)  = dEdBFi;
dE(2)  = dEdB;

