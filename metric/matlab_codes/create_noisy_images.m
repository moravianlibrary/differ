clear;
clc;
img1 = imread('test_images/lena.png');
img2 = imread('test_images/barbara.png');
img3 = imread('test_images/peppers.png');

%% Salt Pepper Noise
n1 = imnoise(img1,'salt & pepper',0.05);
sm_lena(1,1) = psnr(img1,n1,8);
[mssim ssim_map] = ssim_index(img1,n1);
sm_lena(1,2) = mssim;
[qi qi_map] = img_qi(img1,n1);
sm_lena(1,3) = qi;
imwrite(n1,'test_images/lena_sp.png');

n2 = imnoise(img2,'salt & pepper',0.05);
sm_barb(1,1) = psnr(img2,n2,8);
[mssim ssim_map] = ssim_index(img2,n2);
sm_barb(1,2) = mssim;
[qi qi_map] = img_qi(img2,n2);
sm_barb(1,3) = qi;
imwrite(n2,'test_images/barbara_sp.png');

n3 = imnoise(img3,'salt & pepper',0.05);
sm_pep(1,1) = psnr(img3,n3,8);
[mssim ssim_map] = ssim_index(img3,n3);
sm_pep(1,2) = mssim;
[qi qi_map] = img_qi(img3,n3);
sm_pep(1,3) = qi;
imwrite(n3,'test_images/peppers_sp.png');


%% Gaussian Noise
n1 = imnoise(img1,'gaussian');
sm_lena(2,1) = psnr(img1,n1,8);
[mssim ssim_map] = ssim_index(img1,n1);
sm_lena(2,2) = mssim;
[qi qi_map] = img_qi(img1,n1);
sm_lena(2,3) = qi;
imwrite(n1,'test_images/lena_g.png');

n2 = imnoise(img2,'gaussian');
sm_barb(2,1) = psnr(img2,n2,8);
[mssim ssim_map] = ssim_index(img2,n2);
sm_barb(2,2) = mssim;
[qi qi_map] = img_qi(img2,n2);
sm_barb(2,3) = qi;
imwrite(n2,'test_images/barbara_g.png');

n3 = imnoise(img3,'gaussian');
sm_pep(2,1) = psnr(img3,n3,8);
[mssim ssim_map] = ssim_index(img3,n3);
sm_pep(2,2) = mssim;
[qi qi_map] = img_qi(img3,n3);
sm_pep(2,3) = qi;
imwrite(n3,'test_images/peppers_g.png');


%% Blurr
H = fspecial('disk',3);
n1 = imfilter(img1,H,'replicate');
sm_lena(3,1) = psnr(img1,n1,8);
[mssim ssim_map] = ssim_index(img1,n1);
sm_lena(3,2) = mssim;
[qi qi_map] = img_qi(img1,n1);
sm_lena(3,3) = qi;
imwrite(n1,'test_images/lena_blur.png');


n2 = imfilter(img2,H,'replicate');
sm_barb(3,1) = psnr(img2,n2,8);
[mssim ssim_map] = ssim_index(img2,n2);
sm_barb(3,2) = mssim;
[qi qi_map] = img_qi(img2,n2);
sm_barb(3,3) = qi;
imwrite(n2,'test_images/barbara_blur.png');

n3 = imfilter(img3,H,'replicate');
sm_pep(3,1) = psnr(img3,n3,8);
[mssim ssim_map] = ssim_index(img3,n3);
sm_pep(3,2) = mssim;
[qi qi_map] = img_qi(img3,n3);
sm_pep(3,3) = qi;
imwrite(n3,'test_images/peppers_blur.png');


%% Displaying the values 
display('Output for lena :');
sm_lena
display('Output for barbara :');
sm_barb
display('Output for peppers :');
sm_pep
