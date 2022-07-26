function [ydata, y] = plotpolfunc()

initCoeff();

x = -1:.01:1;
y = polfunc(x);
ydata = datafunc(x);

plot(x,ydata,'*b');
hold on
plot(x,y, '-r');

meas = [x', ydata']; 
save('ydata.txt', '-ascii', 'meas');

