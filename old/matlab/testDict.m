function D = testDict(filename, s);

I = double(imread(filename)) /255;
D = gendict(I,s);

save(strcat(filename,'.dict'), 'D');
O = ecol2im(D, [s s], [s*10 s*10 3]);
imwrite(uint8(O*255), strcat(filename,'.dict.jpeg'));
