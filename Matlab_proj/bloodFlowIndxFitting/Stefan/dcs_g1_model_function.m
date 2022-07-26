function g1pred = dcs_g1_model_function(model_args,p)

BFi = model_args(1);

k     = sqrt(3*p.musp*p.mua + p.musp^2*p.k0^2*p.alpha*6*BFi*p.tau);
knorm = sqrt(3*p.musp*p.mua); % k at tau=0 for normalization

g1 = exp(-k*p.r1)/p.r1-exp(-k*p.r2)/p.r2;
g1norm = exp(-knorm*p.r1)/p.r1-exp(-knorm*p.r2)/p.r2;

g1pred = g1/g1norm;

