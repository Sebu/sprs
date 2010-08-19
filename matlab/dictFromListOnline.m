function D = dictFromListOnline(filename,s)

F = textread(filename,'%s');
[width] = size(F, 1);

param.K=4096;  % learns a dictionary
param.lambda=0.15;
param.mode=0;
% param.batchsize=1000;
param.iter=200;  % let us see what happens after 100 iterations.
model = [];
tic
for n=1:width
    name=char(F(n))
    IN = double(imread(name)) /255;
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
save(strcat(filename,'.online.dict'), 'D');
O = ecol2im(D, [s s], [s*64 s*64 3]);
imwrite(uint8(O*10*255), strcat(filename,'.online.dict.png'));
end
