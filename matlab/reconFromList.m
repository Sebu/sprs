function reconFromList(filename, dictfile, s)

F = textread(filename,'%s');
[width] = size(F, 1);

load(dictfile,'-mat');
tic
for n=1:width
    name=char(F(n));
    IN = double(imread(name)) /255;
    param.L=20; % not more than 20 non-zeros coefficients (default: min(size(D,1),size(D,2)))
    param.eps=0.8;
    % param.lambda=0.03; % not more than 20 non-zeros coefficients
    % param.mode=0;       % penalized formulation
    tic
    INC = eim2col(IN, [s s], s);
    alpha=mexOMP(INC, D, param);
    toc
    OUTC = D*alpha;
    error = rmse(INC, OUTC)
    db = psnr(INC,OUTC)
    save(strcat(name,'.stat.txt'),'error','db','-ascii');
    save(strcat(name,'.alpha'),'alpha');
    OUT = ecol2im(OUTC, [s s], size(IN));
%    save(strcat(filename,'.alpha'), 'alpha');
    imwrite(uint8(OUT*255), strcat(name,'.recon.png'));
end
toc
end
