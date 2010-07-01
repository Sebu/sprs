function M = ecol2im(I, n, width)

[rows, cols] = size(I);
m = rows / n
nn = width / n;
mm = cols / nn;
M = zeros(mm*m,nn*n);
k = 1;
for y=0:mm-1
    for x=0:nn-1
        indexy = y*m;
        indexx = x*n;
        C = I(:,k);
        M(indexy+1:indexy+m,indexx+1:indexx+n) = reshape(C,m,n);
        k=k+1;
    end;
end;