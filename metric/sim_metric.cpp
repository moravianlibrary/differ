// OpenCV Implementation of Image Similarity Metrics
// This code calculates the PSNR and MSSIM for the two images to be compared.
// Currently the SSIM implementation is done for gray images, while PSNR calculations works fine for colored images as well.
// Soon, we will be targeting to implement these metrics using CUDA and OpenCL
//
// Author: Dushyant Goyal

#include <cv.h>
#include <highgui.h>
#include <stdio.h>
#include <iostream>

using namespace cv;
using namespace std;

double calcPSNR(IplImage* src1, IplImage* src2, const int L=255);
double calcSSIM(IplImage* src1, IplImage* src2, const double K1 = 0.01, const double K2 = 0.03,	const int gaussian_window=11, const double gaussian_sigma=1.5, const int L = 255);

int main( int argc, char** argv )
{

  IplImage* src1;
  IplImage* src2;
  src1 = cvLoadImage(argv[1]);
  src2 = cvLoadImage(argv[2]);
  int x = src1->width, y = src1->height;
  
  //Converting into Gray images
  IplImage* gray1 = cvCreateImage( cvSize(x,y), IPL_DEPTH_8U, 1);
  IplImage* gray2 = cvCreateImage( cvSize(x,y), IPL_DEPTH_8U, 1);
  
  cvCvtColor(src1, gray1, CV_RGB2GRAY); //color images are BGR!
  cvCvtColor(src2, gray2, CV_RGB2GRAY); //color images are BGR!

  //Calculating the Mean SSIM
  double mean_ssim_value=0;
  mean_ssim_value = calcSSIM(gray1, gray2);
  //mean_ssim_value = calcSSIM(src1, src2);
  cout<<"Mean_ssim_value = "<<mean_ssim_value<<"\n";
  
  //Calculating the PSNR
  double psnr=0;
  psnr = calcPSNR(gray1, gray2);
  //psnr = calcPSNR(src1, src2);
  cout<<"PSNR = "<<psnr<<"\n";
  return 0; 

}

double calcPSNR(IplImage* src1, IplImage* src2,const int L)
{
  int x=src1->width, y=src1->height;
  int nChan=1, d=IPL_DEPTH_32F;
  //size before down sampling
  CvSize size = cvSize(x, y);
  
  //creating diff and difference squares
  IplImage* img1 = cvCreateImage( size, d, nChan);
  IplImage* img2 = cvCreateImage( size, d, nChan);
  IplImage* diff = cvCreateImage( size, d, nChan);
  IplImage* diff_sq = cvCreateImage( size, d, nChan);

  cvConvert(src1, img1);
	cvConvert(src2, img2);
  cvAbsDiff(img1,img2,diff);
  //Squaring the images thus created
  cvPow( diff, diff_sq, 2 );

  CvScalar mse_c = cvAvg( diff_sq );
  double mse = mse_c.val[0];
  double psnr = 10.0*log10((L*L)/mse);

  return psnr;
    
}

double calcSSIM(IplImage* src1, IplImage* src2, const double K1, const double K2, const int gaussian_window, const double gaussian_sigma, const int L)
{
    // default settings
    const double C1 = (K1 * L) * (K1 * L); 
    const double C2 = (K2 * L) * (K2 * L);

  	int x=src1->width, y=src1->height;
  	int nChan=1, d=IPL_DEPTH_32F;
  	//size before down sampling
  	CvSize size = cvSize(x, y);
  	
    //creating FLOAT type images of src1 and src2 
  	IplImage* img1 = cvCreateImage( size, d, nChan);
  	IplImage* img2 = cvCreateImage( size, d, nChan);

  	//Image squares
  	IplImage* img1_sq = cvCreateImage( size, d, nChan);
  	IplImage* img2_sq = cvCreateImage( size, d, nChan);
  	IplImage* img1_img2 = cvCreateImage( size, d, nChan);

    cvConvert(src1, img1);
	  cvConvert(src2, img2);
  
    //Squaring the images thus created
  	cvPow( img1, img1_sq, 2 );
  	cvPow( img2, img2_sq, 2 );
  	cvMul( img1, img2, img1_img2, 1 );

  	IplImage* mu1 = cvCreateImage( size, d, nChan);
  	IplImage* mu2 = cvCreateImage( size, d, nChan);
  	IplImage* mu1_sq = cvCreateImage( size, d, nChan);
  	IplImage* mu2_sq = cvCreateImage( size, d, nChan);
  	IplImage* mu1_mu2 = cvCreateImage( size, d, nChan);

  	IplImage* sigma1_sq = cvCreateImage( size, d, nChan);
  	IplImage* sigma2_sq = cvCreateImage( size, d, nChan);
  	IplImage* sigma12 = cvCreateImage( size, d, nChan);

  	// creating buffers for numerator and denominator and a temp buffer
  	IplImage* numerator1 = cvCreateImage( size, d, nChan);
  	IplImage* numerator2 = cvCreateImage( size, d, nChan);
  	IplImage* numerator = cvCreateImage( size, d, nChan);
  	IplImage* denominator1 = cvCreateImage( size, d, nChan);
  	IplImage* denominator2 = cvCreateImage( size, d, nChan);
  	IplImage* denominator = cvCreateImage( size, d, nChan);
  	//ssim map
  	IplImage* ssim_map = cvCreateImage( size, d, nChan);

  	//PRELIMINARY COMPUTING

  	//gaussian smoothing is performed
  	cvSmooth( img1, mu1, CV_GAUSSIAN, gaussian_window, gaussian_window, gaussian_sigma );
  	cvSmooth( img2, mu2, CV_GAUSSIAN, gaussian_window, gaussian_window, gaussian_sigma );

  	//gettting mu, mu_sq, mu1_mu2
  	cvPow( mu1, mu1_sq, 2 );
  	cvPow( mu2, mu2_sq, 2 );
  	cvMul( mu1, mu2, mu1_mu2, 1 );

  	//calculating sigma1, sigma2, sigma12
  	cvSmooth( img1_sq, sigma1_sq, CV_GAUSSIAN, gaussian_window, gaussian_window, gaussian_sigma );
  	cvAddWeighted( sigma1_sq, 1, mu1_sq, -1, 0, sigma1_sq );
    //cvSub(sigma1_sq, mu1_sq, sigma1_sq);

    cvSmooth( img2_sq, sigma2_sq, CV_GAUSSIAN, gaussian_window, gaussian_window, gaussian_sigma);
  	cvAddWeighted( sigma2_sq, 1, mu2_sq, -1, 0, sigma2_sq );
    //cvSub(sigma2_sq, mu2_sq, sigma2_sq);

  	cvSmooth( img1_img2, sigma12, CV_GAUSSIAN, gaussian_window, gaussian_window, gaussian_sigma );
  	cvAddWeighted( sigma12, 1, mu1_mu2, -1, 0, sigma12 );
    //cvSub(sigma12, mu1_mu2, sigma12);
  	
    //////////////////////////////////////////////////////////////////////////
  	// FORMULA to calculate SSIM

  	// (2*mu1_mu2 + C1)
  	cvScale( mu1_mu2, numerator1, 2 );
  	cvAddS( numerator1, cvScalarAll(C1), numerator1 );

  	// (2*sigma12 + C2)
  	cvScale( sigma12, numerator2, 2 );
  	cvAddS( numerator2, cvScalarAll(C2), numerator2 );

  	// ((2*mu1_mu2 + C1).*(2*sigma12 + C2))
  	cvMul( numerator1, numerator2, numerator, 1 );

  	// (mu1_sq + mu2_sq + C1)
  	cvAdd( mu1_sq, mu2_sq, denominator1 );
  	cvAddS( denominator1, cvScalarAll(C1), denominator1 );

  	// (sigma1_sq + sigma2_sq + C2)
  	cvAdd( sigma1_sq, sigma2_sq, denominator2 );
  	cvAddS( denominator2, cvScalarAll(C2),denominator2 );

  	// ((mu1_sq + mu2_sq + C1).*(sigma1_sq + sigma2_sq + C2))
  	cvMul( denominator1, denominator2, denominator, 1 );

  	// ((2*mu1_mu2 + C1).*(2*sigma12 + C2))./((mu1_sq + mu2_sq + C1).*(sigma1_sq + sigma2_sq + C2))
  	cvDiv( numerator, denominator, ssim_map, 1 );

    // SSIM map created in ssim_map 
    // average is taken 

  	CvScalar mssim_value = cvAvg( ssim_map );
  	return mssim_value.val[0];

}
