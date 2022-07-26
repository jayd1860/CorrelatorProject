function [BFi, beta, err] = getBFiFromFileData(fname)

[t, g2] = readCorrData(fname);
iD = 40;
taus = t(iD:end); 
g2data = g2(iD:end,1,1);

%[BFi, beta, err] = fit_examples(g2data, taus, .1, 6, 2, 850e-7, 1.37, 1);
[BFi, beta, err] = bestfitBFI(g2data, taus, .1, 6, 2, 850e-7, 1.37, 1);


