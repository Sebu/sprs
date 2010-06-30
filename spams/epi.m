clear all;

randn('seed',0);

I1=double(imread('~/Bilder/factorimage_data/fig1_orig2.png'))/255;
[height, width] = size(I1);
X=eim2col(I1, 16, 16);
norm = repmat(sqrt(sum(X.^2)),[size(X,1) 1]);
X=X./ norm;
I2=double(imread('~/Bilder/factorimage_data/epitomes/fig1_orig.png.epi2.jpeg'))/255;
D=eim2col(I2, 16, 16);
D=D./repmat(sqrt(sum(D.^2)),[size(D,1) 1]);

% parameter of the optimization procedure are chosen
param.L=8; % not more than 20 non-zeros coefficients (default: min(size(D,1),size(D,2)))
param.lambda=0.15; % not more than 20 non-zeros coefficients
param.numThreads=8; % number of processors/cores to use; the default choice is -1
                    % and uses all the cores of the machine
param.mode=2;       % penalized formulation

tic
alpha=mexLasso(X,D,param);
t=toc;
toc

fprintf('%f signals processed per second\n',size(X,2)/t);

X2 = D*alpha;
I3 = (ecol2im(X2.* norm, 16, width)  ) * 255;
imwrite(uint8(I3), '~/Bilder/factorimage_data/epitomes/fig1_orig.png.recon_sparse.jpeg');
