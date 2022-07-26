function mse_resid = dcs_g2_separate_beta_cost_function(model_args, measured_g2, p, beta)

g1pred = dcs_g1_model_function(model_args, p);
g2pred = 1 + beta*g1pred.^2;

mse_resid = sum((measured_g2 - g2pred).^2); % sum of the squares of the errors at each correlation time

