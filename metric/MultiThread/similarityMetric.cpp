#include "core.h"
#include "similarityMetric.h"

IplImage* SimilarityMetric :: colorspaceConversion(IplImage *source1, Colorspace space)
{
  IplImage *src1;
  #ifdef DEBUG
  cout<<"Within Color space conversion\n";
  #endif
  int x = source1->width, y = source1->height;
  switch (space)
  {
    case GRAYSCALE:
      if (source1->nChannels != 1)
      {
        src1 = cvCreateImage(cvSize(x,y), IPL_DEPTH_8U, 1); 
        cvCvtColor(source1, src1, CV_RGB2GRAY); //color images are BGR in openCV !
      }
      else
        src1 = cvCloneImage(source1);
      break;

    case RGB:
      src1 = cvCloneImage(source1);
      break;

    case YCbCr:
      if (source1->nChannels != 1)
      {
        src1 = cvCreateImage(cvSize(x,y), IPL_DEPTH_8U, 3); 
        cvCvtColor(source1, src1, CV_RGB2YCrCb); //color images are YCrCb in openCV !
      }
      else
        src1 = cvCloneImage(source1);
      break;

    default:
      cout<<"INVALID ColorSpace !!\n";
  }
  #ifdef DEBUG
  cout<<"Returned from Color space conversion\n";
  #endif
  return src1;
}

