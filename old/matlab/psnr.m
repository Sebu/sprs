function error = psnr( D, E )
%calc PSNR of Data in dB

error = 20*log10( 1 / rmse(D,E));

end
