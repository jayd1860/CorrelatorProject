function [BFi, beta, err] = bestfitBFI(g2data, tau, mua, musp, rho, lamda, n, alpha)

%
% Example:
%
%   g2data = load('g2data.txt');
%   taus = load('taus.txt');
%   [BFi, beta, err_curr] = bestfitBFI(g2data, taus, .1, 6, 2, 850e-7, 1.37, 1)
%

BFi  = []; 
beta = []; 
err  = [];

BFi_min = 5e-11;
BFi_max = 3e-8;
beta_min = 0;
beta_max = .5;

nBFi      = 100;
nbeta     = 100;

incrBFi  = (BFi_max - BFi_min) / nBFi;
incrBeta = (beta_max - beta_min) / nbeta;

BFi(1) = BFi_min;
ii=2;
while ii<=nBFi
    BFi(ii) = BFi(ii-1) + incrBFi;
    if BFi(ii) > BFi_max
        break;
    end
    ii=ii+1;
end

beta(1) = beta_min;
ii=2;
while ii<=nbeta
    beta(ii) = beta(ii-1) + incrBeta;
    if beta(ii) > beta_max
        break;
    end
    ii=ii+1;
end

err = 9999;
indx1 = 1;
indx2 = 1;

initCorrDiffusionEqParams(mua, musp, rho, lamda, n);

b1 = length(BFi);
b2 = length(beta);
N = b1 * b2;

for idx=0:N-1
    ii = floor(idx/b2) + 1;
    jj = mod(idx,b2) + 1;
    
    g2pred_curr = G2cpp(BFi(ii), beta(jj), tau, mua, musp, rho, lamda, n, alpha);
      
    err_curr = 0;
    for kk=1:length(g2data)
        err_curr = err_curr + (g2pred_curr(kk) - g2data(kk)) * (g2pred_curr(kk) - g2data(kk));
    end
    if err_curr < err
        indx1 = ii;
        indx2 = jj;
        err = err_curr;
        g2pred = g2pred_curr;
        displayFitting(2, BFi(ii), beta(jj), err, tau, g2data, g2pred);
    end
    
    if err<1
        dbg=1;
    end
       
end    

displayFitting(2, BFi(indx1), beta(indx2), err, tau, g2data, g2pred);
BFi  = BFi(indx1);
beta = beta(indx2);



