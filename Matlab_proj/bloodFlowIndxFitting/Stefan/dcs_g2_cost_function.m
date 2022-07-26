function err = dcs_g2_cost_function(model_args, tau, measured_g2, p)

g2pred = dcs_g2_model_function(model_args, p);

err = sum((measured_g2 - g2pred).^2); % sum of the squares of the errors at each correlation time

% Display intermediate fitting
PLOT = 1;
if PLOT
	BFi  = model_args(1);
	beta = model_args(2);
	displayFitting(1, BFi, beta, err, tau, measured_g2, g2pred);
end


