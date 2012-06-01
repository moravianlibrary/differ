function PSNR = psnr(f1, f2,k)
% to calculate the psnr 

%k=no. of levels
%k = 8;    
fmax = 2.^k - 1;
a = fmax.^2;
e = double(f1) - double(f2);
[m, n] = size(e);
b = sum(e(:).^2);
PSNR = 10*log10(m*n*a/b);
end
