/*
 * =====================================================================================
 *
 *       Filename:  metric.cpp
 *
 *    Description:  OpenCV Implementation of Image Similarity Metrics
 *                  MSE, PSNR, MSSIM, MS-SSIM, Universal Image Quality
 *
 *        Version:  1.0
 *        Created:  Friday 01 June 2012 03:28:59  IST
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  Dushyant Goyal 
 *        Company:  DIFFER
 *       Comments:  Implement these metrics in OpenCL or CUDA 
 *
 * =====================================================================================
 */

#include <cv.h>
#include <highgui.h>
#include <stdio.h>
#include <iostream>

using namespace cv;
using namespace std;

int main(int argc, char** argv)
{

  IplImage *src1;
  IplImage *src2;
  src1 = cvLoadImage(argv[1]);

  int x = src1->width, y = src1->height;
  int nChan = src1->nChannels;
  //Creating a blurred image of the input image for testing
  IplImage *src3 = cvCreateImage(cvSize(x,y), IPL_DEPTH_8U, nChan);
  cvSmooth(src1, src3, CV_GAUSSIAN, 11 , 11, 2);
  cvSaveImage(argv[2],src3);
  cout<< "x = "<<x<<": y = "<<y<<"\n";
  cvReleaseImage(&src1);
  cvReleaseImage(&src2);
  cvReleaseImage(&src3);

}



