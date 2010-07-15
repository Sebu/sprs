function D = dictfromlist(filename,s)

F = textread(filename,'%s');
[width nouse] = size(F);

param.K=768;  % learns a dictionary
param.lambda=0.15;
param.mode=0;
% param.batchsize=1000;
param.iter=400;  % let us see what happens after 100 iterations.
model = [];
tic
for n=1:width
    n=char(F(n))
    IN = double(imread(n)) /255;
    X=eim2col(IN, [s s], s);
    X=X./repmat(sqrt(sum(X.^2)),[size(X,1) 1]);
    tic
    if isempty(model)
      [D model] = mexTrainDL(X, param);
    else
      param.D=D;
      [D model] = mexTrainDL(X, param, model);
    end

    toc
end
toc
save(strcat(filename,'.dict'), 'D');
O = ecol2im(D, [s s], [s*16 s*16 3]);
imwrite(uint8(O*10*255), strcat(filename,'.dict.jpeg'));
end
