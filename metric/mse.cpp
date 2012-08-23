#include "core.h"
#include "similarityMetric.h"
#include "mse.h"    

calcMSE :: calcMSE()
{
  for (int i=0; i < 4; i++)
    mse.val[i] = -1;
}

CvScalar calcMSE :: compare(IplImage *source1, IplImage *source2, Colorspace space)
{
  IplImage *src1, *src2;
  src1 = colorspaceConversion(source1, space);
  src2 = colorspaceConversion(source2, space);
  
  int x = src1->width, y = src1->height;
  int nChan = src1->nChannels, d = IPL_DEPTH_32F;
  //size before down sampling
  CvSize size = cvSize(x, y);
  
  //creating diff and difference squares
  IplImage *img1 = cvCreateImage(size, d, nChan);
  IplImage *img2 = cvCreateImage(size, d, nChan);
  IplImage *diff = cvCreateImage(size, d, nChan);
  IplImage *diff_sq = cvCreateImage(size, d, nChan);

  cvConvert(src1, img1);
	cvConvert(src2, img2);
  cvAbsDiff(img1, img2, diff);
  //Squaring the images thus created
  cvPow(diff, diff_sq, 2);

  CvScalar mse = cvAvg(diff_sq);

  //Release images
  cvReleaseImage(&img1);
  cvReleaseImage(&img2);
  cvReleaseImage(&diff);
  cvReleaseImage(&diff_sq);
  cvReleaseImage(&src1);
  cvReleaseImage(&src2);

  return mse;
}

