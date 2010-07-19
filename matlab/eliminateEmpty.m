function [ O ] = eliminateEmpty( I )
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here
size(I)
k=1;
s=(size(I,1)/3) * 2
O=[];
for n=1:size(I, 2)
    if (sum(I(:,n)==1) ~= s)
        O(:,k) = I(:,n);
        k=k+1;
    end
end
size(O)

end

