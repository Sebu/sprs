function OUT = reconLasso(IN, s, DICT)

param.L=20; % not more than 20 non-zeros coefficients (default: min(size(D,1),size(D,2)))
param.lambda=0.15; % not more than 20 non-zeros coefficients
param.mode=2;       % penalized formulation
tic;
alpha=mexLasso(eim2col(IN, [s s], s), DICT, param);
toc
OUT = ecol2im(DICT*alpha, [s s], size(IN));

end