function D = dictFromList(filename,s)

F = textread(filename,'%s');
[width] = size(F, 1);

param.K=16384;  % learns a dictionary
param.lambda=0.15;
param.mode=0;
param.batchsize=1000;
param.iter=300;  % let us see what happens after 100 iterations.
X = [];
tic
for n=1:width
    name=char(F(n))
    IN = double(imread(name)) /255;
    Xtmp=eim2col(IN, [s s], s);
    Xtmp=eliminateEmpty(Xtmp);
    Xtmp=Xtmp./repmat(sqrt(sum(Xtmp.^2)),[size(Xtmp,1) 1]);
    if isempty(X)
        X = Xtmp;
    else
        X = cat(2,X,Xtmp);
    end
end
toc
size(X)
D = mexTrainDL(X, param);

save(strcat(filename,'.dict'), 'D');
O = ecol2im(D, [s s], [s*128 s*128 3]);
imwrite(uint8(O*10*255), strcat(filename,'.dict.8.png'));
end
