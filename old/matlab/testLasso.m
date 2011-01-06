function OUT = testLasso(filename, s, DICT)

IN = double(imread(filename)) /255;
OUT = reconLasso(IN,s,DICT);
rmse(IN, OUT)
imwrite(uint8(OUT*255), strcat(filename,'.recon.jpeg'));

end