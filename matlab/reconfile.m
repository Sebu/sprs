function I = reconfile(filename, s, D)

IN = double(imread(filename)) /255;
param.L=20; % not more than 20 non-zeros coefficients (default: min(size(D,1),size(D,2)))
param.eps=0.015;
param.lambda=0.15; % not more than 20 non-zeros coefficients
param.mode=2;       % penalized formulation
tic;
alpha=mexOMP(eim2col(IN, [s s], s),D,param);
toc
I = ecol2im(D*alpha, [s s], size(IN));
imwrite(uint8(I*255), strcat(filename,'.recon.jpeg'));
