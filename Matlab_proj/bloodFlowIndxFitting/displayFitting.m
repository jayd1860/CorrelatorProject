function displayFitting(fignum, BFi, beta, err, tau, measured_g2, g2pred)

figure(fignum); hold off

semilogx(tau, measured_g2, '*');

hold on

semilogx(tau, g2pred);

title(sprintf('BFi = %.3g,  beta = %.3f, Error = %.3f', BFi, beta, err));

pause(.01);
