#include "core.h"
#include "similarityMetric.h"
#include "iqi.h"

calcQualityIndex :: calcQualityIndex()
{
  B = 8;
  image_quality_map = NULL;
  for (int i=0; i < 4; i++)
    image_quality_value.val[i] = -2;   // Initialize with an out of bound value for image_quality_value [-1.1]
}

calcQualityIndex :: ~calcQualityIndex()
{
  if (image_quality_map != NULL)
    cvReleaseImage(&image_quality_map);
}

void calcQualityIndex :: releaseImageQuality_map() {
  if (image_quality_map != NULL)
    cvReleaseImage(&image_quality_map);
}

int calcQualityIndex :: print_map()
{
  if (image_quality_map == NULL)
  {
    cout<<"Error>> No Index_map_created.\n";
    return 0;
  }
  cvSave("imgQI.xml", image_quality_map, NULL, "TESTing Index map");
  return 1;
}

CvScalar calcQualityIndex :: compare(IplImage *source1, IplImage *source2, Colorspace space)
{
  IplImage *src1,* src2;
  src1 = colorspaceConversion(source1, space);
  src2 = colorspaceConversion(source2, space);
  
  int x = src1->width, y = src1->height;
  int nChan = src1->nChannels, d = IPL_DEPTH_32F;
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
  
  //average smoothing is performed
  cvSmooth(img1, mu1, CV_BLUR, B, B);
  cvSmooth(img2, mu2, CV_BLUR, B, B);

  //gettting mu, mu_sq, mu1_mu2
  cvPow(mu1, mu1_sq, 2);
  cvPow(mu2, mu2_sq, 2);
  cvMul(mu1, mu2, mu1_mu2, 1);

  //calculating sigma1, sigma2, sigma12
  cvSmooth(img1_sq, sigma1_sq, CV_BLUR, B, B);
  cvSub(sigma1_sq, mu1_sq, sigma1_sq);

  cvSmooth(img2_sq, sigma2_sq, CV_BLUR, B, B);
  cvSub(sigma2_sq, mu2_sq, sigma2_sq);

  cvSmooth(img1_img2, sigma12, CV_BLUR, B, B);
  cvSub(sigma12, mu1_mu2, sigma12);

  //Releasing unused images 
  cvReleaseImage(&img1);
  cvReleaseImage(&img2);
  cvReleaseImage(&img1_sq);
  cvReleaseImage(&img2_sq);
  cvReleaseImage(&img1_img2);
  
  // creating buffers for numerator and denominator
  IplImage *numerator1 = cvCreateImage(size, d, nChan);
  IplImage *numerator = cvCreateImage(size, d, nChan);
  IplImage *denominator1 = cvCreateImage(size, d, nChan);
  IplImage *denominator2 = cvCreateImage(size, d, nChan);
  IplImage *denominator = cvCreateImage(size, d, nChan);

  // FORMULA to calculate Image Quality Index

  // (4*sigma12)
  cvScale(sigma12, numerator1, 4);

  // (4*sigma12).*(mu1*mu2)
  cvMul(numerator1, mu1_mu2, numerator, 1);

  // (mu1_sq + mu2_sq)
  cvAdd(mu1_sq, mu2_sq, denominator1);

  // (sigma1_sq + sigma2_sq)
  cvAdd(sigma1_sq, sigma2_sq, denominator2);
  
  //Release images
  cvReleaseImage(&mu1);
  cvReleaseImage(&mu2);
  cvReleaseImage(&mu1_sq);
  cvReleaseImage(&mu2_sq);
  cvReleaseImage(&mu1_mu2);
  cvReleaseImage(&sigma1_sq);
  cvReleaseImage(&sigma2_sq);
  cvReleaseImage(&sigma12);
  cvReleaseImage(&numerator1);

  // ((mu1_sq + mu2_sq).*(sigma1_sq + sigma2_sq))
  cvMul(denominator1, denominator2, denominator, 1);

  //image_quality map
  image_quality_map = cvCreateImage(size, d, nChan);
  float *num, *den, *res;
  num = (float*)(numerator->imageData);
  den = (float*)(denominator->imageData);
  res = (float*)(image_quality_map->imageData);
 
  // dividing by hand
  // ((4*sigma12).*(mu1_mu2))./((mu1_sq + mu2_sq).*(sigma1_sq + sigma2_sq))
  for (int i=0; i<(x*y*nChan); i++) {
    if (den[i] == 0)
    {
      num[i] = (float)(1.0);
      den[i] = (float)(1.0);
    }
    res[i] = 1.0*(num[i]/den[i]);
  }
 
  // cvDiv doesnt work 
  //cvDiv(numerator, denominator, image_quality_map, 1);

  // image_quality_map created in image_quality_map
  // average is taken 
  image_quality_value = cvAvg(image_quality_map);

  //Release images
  cvReleaseImage(&numerator);
  cvReleaseImage(&denominator);
  cvReleaseImage(&denominator1);
  cvReleaseImage(&denominator2);
  cvReleaseImage(&src1);
  cvReleaseImage(&src2);

  return image_quality_value;
}


