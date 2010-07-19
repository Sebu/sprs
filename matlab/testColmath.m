
s = 12;

I = double(imread('~/Bilder/factorimage_data/epitomes/fig25_orig.png.epi.png')) /255;
X=eim2col(I, [s s], s);
X=X./repmat(sqrt(sum(X.^2)),[size(X,1) 1]);

param.K=130;  % learns a dictionary with 100 elements
param.lambda=0.15;
param.mode=2;
% param.batchsize=1000;
param.iter=-10;  % let us see what happens after 100 iterations.
tic
D = mexTrainDL(X,param);

% param.L=20; % not more than 20 non-zeros coefficients (default: min(size(D,1),size(D,2)))
param.lambda=0.15; % not more than 20 non-zeros coefficients
param.mode=2;       % penalized formulation
alpha=mexLasso(X, D, param);

U=D*alpha;
colMatch(X,D);
