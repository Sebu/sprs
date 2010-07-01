clear all;

s_ = 16;

I1=double(imread('~/Bilder/factorimage_data/fig20c_orig.png')) /255;
[height, width, channels] = size(I1);
X = [];
for c=1:channels
    T =  I1(:,:,c);
    T2 = im2col(T, [s_  s_], 'distinct');
    if isempty(X)
        X = T2;
    else
        X = cat(1,X,T2);
    end
end
I2=double(imread('~/Bilder/factorimage_data/fig20c_epitome.png'))/255;
[height, width, channels] = size(I2);
X2 = [];
for c=1:channels
    T =  I2(:,:,c);
    T2 = im2col(T, [s_  s_], 'sliding');
    if isempty(X2)
        X2 = T2;
    else
        X2 = cat(1,X2,T2);
    end
end
X2=X2./repmat(sqrt(sum(X2.^2)),[size(X2,1) 1]);


param.K=500;  % learns a dictionary with 100 elements
param.lambda=0.15;
% param.numThreads=4; % number of threads
%param.batchsize=1000;

param.iter=100;  % let us see what happens after 100 iterations.


%%%%%%%%%% FIRST EXPERIMENT %%%%%%%%%%%
tic
D = mexTrainDL(X2,param);
t=toc;
fprintf('time of computation for Dictionary Learning: %f\n',t);
ImD=displayPatches(D);
% parameter of the optimization procedure are chosen
param.L=12; % not more than 20 non-zeros coefficients (default: min(size(D,1),size(D,2)))
% param.eps=0.0;
param.lambda=0.15; % not more than 20 non-zeros coefficients
% param.numThreads=8; % number of processors/cores to use; the default choice is -1
                    % and uses all the cores of the machine
param.mode=2;       % penalized formulation

tic
alpha=mexLasso(X,D,param);
t=toc;
toc

fprintf('%f signals processed per second\n',size(X,2)/t);

X3 = D*alpha;
[height, width, channels] = size(I1)
I3 = zeros(size(I1));
for c=1:channels
    start = (c-1) * s_ *s_
    ende = c * s_ *s_
    T = (col2im(X3(start+1:ende,:), [s_  s_], [height width], 'distinct')) * 255;
    I3(:,:,c) = T;
end


imwrite(uint8(I3), '~/Bilder/factorimage_data/epitomes/fig1_orig.png.recon_sparse.jpeg');
