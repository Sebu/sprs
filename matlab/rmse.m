function error = rmse( D, E )
%rmse calc RMSE of Data and Estimate

%  Felix Hebeler http://www.mathworks.com/matlabcentral/fileexchange/21383-rmse
error = sqrt( sum( (D(:)-E(:)).^2) / numel(D) );

end

