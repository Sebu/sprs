clear all;

s_ = 12;

I1=double(imread('~/Bilder/factorimage_data/bild5.jpg')) /255;
X1 = eim2col(I1, [s_ s_], s_);

I2=double(imread('~/Bilder/factorimage_data/epitomes/bild5.jpg.epi.png'))/255;
X2= eim2col(I2, [s_ s_], s_/4);
X2=X2./repmat(sqrt(sum(X2.^2)),[size(X2,1) 1]);


param.K=100;  % learns a dictionary with 100 elements
param.lambda=0.15;
% param.batchsize=1000;

param.iter=500;  % let us see what happens after 100 iterations.

tic
 D = mexTrainDL(X2,param);
t=toc;
fprintf('time of computation for Dictionary Learning: %f\n',t);
% ImD=displayPatches(D);
% parameter of the optimization procedure are chosen
% param.L=1; % not more than 20 non-zeros coefficients (default: min(size(D,1),size(D,2)))
param.eps=0.0;
param.lambda=0.15; % not more than 20 non-zeros coefficients
param.mode=2;       % penalized formulation

tic
alpha=mexLasso(X1,D,param);
t=toc;
toc
fprintf('%f signals processed per second\n',size(X1,2)/t);

X3 = D*alpha;

I3 = ecol2im(X3, [s_ s_], size(I1));


imwrite(uint8(I3*255), '~/Bilder/factorimage_data/epitomes/fig1_orig.png.recon_sparse.jpeg');
