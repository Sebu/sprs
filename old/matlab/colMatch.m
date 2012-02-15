function [ Selection ] = colMatch( Reference, Signals )
%COLMATCH Summary of this function goes here
%   Detailed explanation goes here

numCol = size(Reference, 2)
numSig = size(Signals, 2)
Selection = zeros(numCol);

for i=1:numCol
 for j=1:numSig
    Selection(i) = isequal(Signals(:,j), Reference(:,i));
 end
end


