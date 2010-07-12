function D = gendict(IN, s)

X=eim2col(IN, [s s], s);
X=X./repmat(sqrt(sum(X.^2)),[size(X,1) 1]);

param.K=100;  % learns a dictionary with 100 elements
param.lambda=0.15;
param.mode=0;
% param.batchsize=1000;
param.iter=500;  % let us see what happens after 100 iterations.
tic
D = mexTrainDL(X,param);
t=toc;

fprintf('time of computation for Dictionary Learning: %f\n',t);
end

