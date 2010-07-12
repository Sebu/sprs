function D = testDict(filename, s);

I = double(imread(filename)) /255;
D = gendict(I,s);

save(strcat(filename,'.dict'), 'D');
