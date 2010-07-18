function X = eim2col(O, D, step)
%eim2col extended image to column (with multi channel and step support)

[height, width, channels] = size(O);
m = D(1);
n = D(2);
mm = ceil(height / step);
nn = ceil(width / step);
I = zeros([mm*step nn*step channels]);
I(1:height, 1:width,:) = O;

X = [];
for c=1:channels
    T =  I(:,:,c);
    maxm = mm-(m/step);
    maxn = nn-(n/step);
    M = []; %ones([m*n, maxm*maxn]);
    k = 1;
    for y=0:maxm
        for x=0:maxn
            indexy = y*step;
            indexx = x*step;
            C = T(indexy+1:indexy+m,indexx+1:indexx+n);
%            if sum(A(:)==0)== 
            M(:,k) = C(:);
            k=k+1;
%            end
        end;
    end;
    if isempty(X)
        X = M;
    else
        X = cat(1,X,M);
    end
end

end

