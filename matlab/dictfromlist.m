function D = =dictfromlist(filename,s)

F = textread(filename,'%s');
[width nouse] = size(F);

param.K=500;  % learns a dictionary with 100 elements
param.lambda=0.15;
param.mode=0;
% param.batchsize=1000;
param.iter=500;  % let us see what happens after 100 iterations.

tic
for n=1:width
    IN = double(imread(F(n))) /255;
    X=eim2col(IN, [s s], s);
    X=X./repmat(sqrt(sum(X.^2)),[size(X,1) 1]);
    tic
    [D model] = mexTrainDL(X, param, model);
    toc
end
toc
save(strcat(filename,'.dict'), 'D');
O = ecol2im(D, [s s], [s*50 s*50 3]);
imwrite(uint8(O*255), strcat(filename,'.dict.jpeg'));
end