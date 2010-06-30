function M = eim2col(I, m, n)

[rows, cols] = size(I);
mm = rows / m;
nn = cols / n;
M = zeros(m*n,mm*nn);

k = 1;
for y=0:mm-1
    for x=0:nn-1
        indexy = y*m;
        indexx = x*n;
        C = I(indexy+1:indexy+m,indexx+1:indexx+n);
        M(:,k) = C(:);
        k=k+1;
    end;
end;