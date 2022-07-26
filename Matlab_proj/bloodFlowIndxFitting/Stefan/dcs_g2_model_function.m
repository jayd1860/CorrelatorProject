function g2pred = dcs_g2_model_function(model_args, p)

Db     = model_args(1);
beta   = model_args(2);

k      = sqrt(3*p.musp*p.mua + p.musp^2*p.k0^2*p.alpha*6*Db*p.tau);
knorm  = sqrt(3*p.musp*p.mua); % k at tau=0 for normalization

G1     = exp(-k*p.r1)/p.r1 - exp(-k*p.r2)/p.r2;
G1norm = exp(-knorm*p.r1)/p.r1 - exp(-knorm*p.r2)/p.r2;

g1     = G1/G1norm;

g2pred = 1+beta*(g1.^2);