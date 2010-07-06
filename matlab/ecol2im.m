function X = ecol2im(I, S, D)




m = S(1);
n = S(2);
height = D(1);
width = D(2);
channels = D(3);
mm = ceil(height / m);
nn = ceil(width / n);
X = zeros([mm*m nn*n channels]);
for c=1:channels
    start = (c-1) * m * n;
    ende = c * m * n;
    
    M = zeros(height, width);
    k = 1;
    for y=0:mm-1
        for x=0:nn-1
            indexy = y*m;
            indexx = x*n;
            C = I(start+1:ende, k);
            M(indexy+1:indexy+m,indexx+1:indexx+n) = reshape(C,m,n);
            k=k+1;
        end;
    end;
    X(:,:,c) = M;
end
X = X(1:height, 1:width, :);
