function TPSF_and_ACF_plotter()


% file conversion
% parseFXdata_chunk_autoestimate('011_2s_ampli2300mA.bin');
% simple processing

load('../../../../../data/004_10s_ampli2300mA_seg1.mat');
y = dataDCS;

time = 10;

% autocorrelation function plot
acfmin = 0;
acfmax = 0;
figure;
for i=4:4
    [acf, t] = pulse_xcorr_2stage(y.arrtimes{i}(:,1), y.arrtimes{i}(:,1), 1.5e6, 1.5e8, 1e3);    
    plot(t,acf);
    hold all;
    set(gca,'ylim',[0.991, 1.02]);
    set(gca,'xscale','log');
    set(gca,'xlim',[min(t), max(t)]);
    drawnow;
end

