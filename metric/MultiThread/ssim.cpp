#include "core.h"
#include "similarityMetric.h"
#include "ssim.h"

calcSSIM :: calcSSIM()
{
  K1 = 0.01;
  K2 = 0.03;
  gaussian_window = 11;
  gaussian_sigma = 1.5;
  L = 255;
  ssim_map = NULL;
  for (int i=0; i < 4; i++)
    mssim_value.val[i] = -1; // Initialize with an out of bound value of mssim [0,1]
  cs_map = NULL;
  for (int i=0; i < 4; i++)
    mean_cs_value.val[i] = -1; // Initialize with an out of bound value of mssim [0,1]
}

void calcSSIM :: releaseSSIM_map() { 
  if (ssim_map != NULL)
    cvReleaseImage(&ssim_map);
  ssim_map = NULL;
}

void calcSSIM :: releaseCS_map() { 
  if (cs_map != NULL)
    cvReleaseImage(&cs_map); 
  cs_map = NULL;
}

int calcSSIM :: print_map()
{
  if (ssim_map == NULL)
  {
    cout<<"Error>> No Index_map_created.\n";
    return 0;
  }
  cvSave("imgSSIM.xml", ssim_map, NULL, "TESTing Index map");
  return 1;
}

CvScalar calcSSIM :: compare(IplImage *source1, IplImage *source2, Colorspace space)
{
  IplImage *src1, *src2;
  src1 = colorspaceConversion(source1, space);
  src2 = colorspaceConversion(source2, space);
  int x = source1->width, y = source1->height;
  // default settings
  const double C1 = (K1 * L) * (K1 * L); 
  const double C2 = (K2 * L) * (K2 * L);

  int nChan = src1->nChannels;
  int d = IPL_DEPTH_32F;
  CvSize size = cvSize(x, y);
  
  //creating FLOAT type images of src1 and src2 
  IplImage *img1 = cvCreateImage(size, d, nChan);
  IplImage *img2 = cvCreateImage(size, d, nChan);

  //Image squares
  IplImage *img1_sq = cvCreateImage(size, d, nChan);
  IplImage *img2_sq = cvCreateImage(size, d, nChan);
  IplImage *img1_img2 = cvCreateImage(size, d, nChan);
  
  cvConvert(src1, img1);
  cvConvert(src2, img2);
  
  //Squaring the images thus created
  cvPow(img1, img1_sq, 2);
  cvPow(img2, img2_sq, 2);
  cvMul(img1, img2, img1_img2, 1);

  IplImage *mu1 = cvCreateImage(size, d, nChan);
  IplImage *mu2 = cvCreateImage(size, d, nChan);
  IplImage *mu1_sq = cvCreateImage(size, d, nChan);
  IplImage *mu2_sq = cvCreateImage(size, d, nChan);
  IplImage *mu1_mu2 = cvCreateImage(size, d, nChan);

  IplImage *sigma1_sq = cvCreateImage(size, d, nChan);
  IplImage *sigma2_sq = cvCreateImage(size, d, nChan);
  IplImage *sigma12 = cvCreateImage(size, d, nChan);

  //PRELIMINARY COMPUTING
  //gaussian smoothing is performed
  cvSmooth(img1, mu1, CV_GAUSSIAN, gaussian_window, gaussian_window, gaussian_sigma);
  cvSmooth(img2, mu2, CV_GAUSSIAN, gaussian_window, gaussian_window, gaussian_sigma);

  //gettting mu, mu_sq, mu1_mu2
  cvPow(mu1, mu1_sq, 2);
  cvPow(mu2, mu2_sq, 2);
  cvMul(mu1, mu2, mu1_mu2, 1);

  //calculating sigma1, sigma2, sigma12
  cvSmooth(img1_sq, sigma1_sq, CV_GAUSSIAN, gaussian_window, gaussian_window, gaussian_sigma);
  cvSub(sigma1_sq, mu1_sq, sigma1_sq);

  cvSmooth(img2_sq, sigma2_sq, CV_GAUSSIAN, gaussian_window, gaussian_window, gaussian_sigma);
  cvSub(sigma2_sq, mu2_sq, sigma2_sq);

  cvSmooth(img1_img2, sigma12, CV_GAUSSIAN, gaussian_window, gaussian_window, gaussian_sigma);
  cvSub(sigma12, mu1_mu2, sigma12);
  
  //releasing some junk buffers
  cvReleaseImage(&img1);
  cvReleaseImage(&img2);
  cvReleaseImage(&img1_sq);
  cvReleaseImage(&img2_sq);
  cvReleaseImage(&img1_img2);
  cvReleaseImage(&mu1);
  cvReleaseImage(&mu2);
  
  // creating buffers for numerator and denominator 
  IplImage *numerator1 = cvCreateImage(size, d, nChan);
  IplImage *numerator2 = cvCreateImage(size, d, nChan);
  IplImage *numerator = cvCreateImage(size, d, nChan);
  IplImage *denominator1 = cvCreateImage(size, d, nChan);
  IplImage *denominator2 = cvCreateImage(size, d, nChan);
  IplImage *denominator = cvCreateImage(size, d, nChan);

  // FORMULA to calculate SSIM
  // (2*mu1_mu2 + C1)
  cvScale(mu1_mu2, numerator1, 2);
  cvAddS(numerator1, cvScalarAll(C1), numerator1);
  // (2*sigma12 + C2) 
  cvScale(sigma12, numerator2, 2);
  cvAddS(numerator2, cvScalarAll(C2), numerator2);
  // ((2*mu1_mu2 + C1).*(2*sigma12 + C2))
  cvMul(numerator1, numerator2, numerator, 1);

  // (mu1_sq + mu2_sq + C1)
  cvAdd(mu1_sq, mu2_sq, denominator1);
  cvAddS(denominator1, cvScalarAll(C1), denominator1);
  // (sigma1_sq + sigma2_sq + C2) >>>
  cvAdd(sigma1_sq, sigma2_sq, denominator2);
  cvAddS(denominator2, cvScalarAll(C2),denominator2);
  // ((mu1_sq + mu2_sq + C1).*(sigma1_sq + sigma2_sq + C2))
  cvMul(denominator1, denominator2, denominator, 1);

  //Release some junk buffers 
  cvReleaseImage(&numerator1);
  cvReleaseImage(&denominator1);
  cvReleaseImage(&mu1_sq);
  cvReleaseImage(&mu2_sq);
  cvReleaseImage(&mu1_mu2);
  cvReleaseImage(&sigma1_sq);
  cvReleaseImage(&sigma2_sq);
  cvReleaseImage(&sigma12);

  //ssim map and cs_map
  ssim_map = cvCreateImage(size, d, nChan);
  cs_map = cvCreateImage(size, d, nChan);
  // SSIM_INDEX map 
  // ((2*mu1_mu2 + C1).*(2*sigma12 + C2))./((mu1_sq + mu2_sq + C1).*(sigma1_sq + sigma2_sq + C2))
  cvDiv(numerator, denominator, ssim_map, 1);
  // Contrast Structure CS_index map
  // (2*sigma12 + C2)./(sigma1_sq + sigma2_sq + C2)
  cvDiv(numerator2, denominator2, cs_map, 1);

  // average is taken for both SSIM_map and CS_map 
  mssim_value = cvAvg(ssim_map);
  mean_cs_value = cvAvg(cs_map);

  //Release images
  cvReleaseImage(&numerator);
  cvReleaseImage(&denominator);
  cvReleaseImage(&numerator2);
  cvReleaseImage(&denominator2);
  cvReleaseImage(&src1);
  cvReleaseImage(&src2);

  return mssim_value;
}


