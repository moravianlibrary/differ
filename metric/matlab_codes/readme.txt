There are 3 image similarity metrics implementations in matlab
1. PSNR - psnr.m
2. MSSIM - ssim_index.m
3. Image Quality Index - img_qi.m

create_noisy_images.m generates the noisy images of three types of the 3 test images.
1. salt-pepper noise (5%)
2. guassian noise (mean=0, varaince=0.05)
3. blurr (mask=3)

Test Images -
1. Lena.png
2. Barbara.png
3. Peppers.png

Ouput format-
1. For each image there are 3 lines of ouput corresponding to 
   Salt-pepper
   Gaussian-noise
   Blurr          
2. Each line of output contains the PSNR, MSSIM, Img_quality_index in that order.


References-
1. https://ece.uwaterloo.ca/~z70wang/research/ssim/ssim_index.m
2. https://ece.uwaterloo.ca/~z70wang/research/quality_index/img_qi.m

