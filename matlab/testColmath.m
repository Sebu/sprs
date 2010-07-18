
s = 12;

I = double(imread('~/Bilder/factorimage_data/epitomes/fig25_orig.png.epi.png')) /255;
X=eim2col(I, [s s], s);
X=X./repmat(sqrt(sum(X.^2)),[size(X,1) 1]);

param.K=225;  % learns a dictionary with 100 elements
param.lambda=0.15;
param.mode=0;
% param.batchsize=1000;
param.iter=-1;  % let us see what happens after 100 iterations.
tic
D = mexTrainDL(X,param);
t=toc;

D=D./repmat(sqrt(sum(D.^2)),[size(D,1) 1]);
colMatch(X,D);