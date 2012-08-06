#include <stdlib.h>
#include <sstream>
#include <ctime>
#include <cv.h>
#include <highgui.h>
#include <stdio.h>
#include <iostream>
#include <getopt.h>    // for getopt_long
#include <string.h>
#include <time.h>
#include <sys/time.h>
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#define MAX_SOURCE_SIZE (0x100000)
using namespace cv;
using namespace std;

//#ifndef DEBUG
//#define DEBUG
//#endif

enum Colorspace {
  GRAYSCALE,
  RGB,
  YCbCr
};

// Some primitive and useful functions

Mat get_gaussian_filter(int filter_size, int sigma) {
  Mat filter1D;
  filter1D = getGaussianKernel(filter_size, sigma, CV_32F);
  Mat filter2D = filter1D * filter1D.t();
  return filter2D;
}

void printCvScalar(CvScalar value, const char *comment)
{
  cout<<comment<<"\t :\t"<<value.val[0]<<" :\t "<<value.val[1]<<" :\t "<<value.val[2]<<" :\t "<<value.val[3]<<"\n";
}

void print_time(clock_t start, int TIMES, const char *s) {
  double diff, time;
  diff = ( std::clock() - start ) / (double)CLOCKS_PER_SEC;
  time = diff/TIMES;
  printf("Time of CPU run using %s (averaged for %d runs): %f milliseconds.\n",s,TIMES, time);
  printf("Time of CPU run using %s (total for %d runs): %f milliseconds.\n",s, TIMES, diff);
}


// Image SimilarityMetric Base Class
class SimilarityMetric {

  public:
    virtual CvScalar compare(IplImage *src1, IplImage *src2, Colorspace space)
    {
    
    }

    IplImage* colorspaceConversion(IplImage *source1, Colorspace space)
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

};

class calcMSE : public SimilarityMetric {

  private:
    CvScalar mse;

  public:
    calcMSE()
    {
      for (int i=0; i < 4; i++)
        mse.val[i] = -1;
    }

    CvScalar getMSE() { return mse; }

    virtual CvScalar compare(IplImage *source1, IplImage *source2, Colorspace space)
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
};


class calcPSNR : public SimilarityMetric {

  private:
    int L;
    CvScalar mse;
    CvScalar PSNR;
  
  public:
   
    calcPSNR()
    {
      L = 255;
      for (int i=0; i < 4; i++)
        mse.val[i] = -1;
      for (int i=0; i < 4; i++)
        PSNR.val[i] = -1;
    }

    void setL(int value) { L = value; }
    void setMSE(CvScalar value) { mse = value; }
    CvScalar getPSNR() { return PSNR; }

    virtual CvScalar compare(IplImage *source1, IplImage *source2, Colorspace space)
    {
      IplImage *src1, *src2;
      src1 = colorspaceConversion(source1, space);
      src2 = colorspaceConversion(source2, space);
      
      //if (mse.val[0]<0)
      if (1)
      {
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
        mse = cvAvg(diff_sq);
      
        //Release images
        cvReleaseImage(&img1);
        cvReleaseImage(&img2);
        cvReleaseImage(&diff);
        cvReleaseImage(&diff_sq);
      }
      
      cvReleaseImage(&src1);
      cvReleaseImage(&src2);
      
      PSNR.val[0] = 10.0*log10((L*L)/mse.val[0]);
      PSNR.val[1] = 10.0*log10((L*L)/mse.val[1]);
      PSNR.val[2] = 10.0*log10((L*L)/mse.val[2]);
      PSNR.val[3] = 10.0*log10((L*L)/mse.val[3]);
      return PSNR;
    }
};

class calcSSIM : public SimilarityMetric {

  private:
    double K1;
    double K2;
    int gaussian_window;
    double gaussian_sigma;
    int L;
    IplImage *ssim_map;
    CvScalar mssim_value;
    IplImage *cs_map;
    CvScalar mean_cs_value; // mean of contrast, structure (part of l,c,s)

  public:

    calcSSIM()
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
    
    /* ~calcSSIM()
    {
      if (ssim_map != NULL)
        cvReleaseImage(&ssim_map);
      if (cs_map != NULL)
        cvReleaseImage(&cs_map);
    }*/

    void setK1(double val) { K1 = val; }
    void setK2(double val) { K2 = val; }
    void setGaussian_window(int val) { gaussian_window = val; }
    void setGaussian_sigma(int val) { gaussian_sigma = val; }
    void setL(int val) { L = val; }

    CvScalar getMSSIM() { return mssim_value; }
    CvScalar getMeanCSvalue() { return mean_cs_value; }
    IplImage* getSSIM_map() { return ssim_map; }
    IplImage* getCS_map() { return cs_map; }

    void releaseSSIM_map() { 
      if (ssim_map != NULL)
        cvReleaseImage(&ssim_map);
      ssim_map = NULL;
    }
    
    void releaseCS_map() { 
      if (cs_map != NULL)
        cvReleaseImage(&cs_map); 
      cs_map = NULL;
    }

    int print_map()
    {
      if (ssim_map == NULL)
      {
        cout<<"Error>> No Index_map_created.\n";
        return 0;
      }
      
      int x = ssim_map->width, y = ssim_map->height;
      int nChan = ssim_map->nChannels, d = IPL_DEPTH_8U;
      CvSize size = cvSize(x, y);
      IplImage *ssim_map_t = cvCreateImage(size, d, nChan);
      cvConvert(ssim_map, ssim_map_t);
      cvSave("output/imgSSIM.xml", ssim_map, NULL, "TESTing Index map");
      cvSaveImage("output/imgSSIM.bmp", ssim_map_t);
     
      //Release Image
      cvReleaseImage(&ssim_map_t);

      return 1;
    }

    virtual CvScalar compare(IplImage *source1, IplImage *source2, Colorspace space)
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

};

class calcMSSSIM : public SimilarityMetric {

  private:
    double K1;
    double K2;
    int gaussian_window;
    double gaussian_sigma;
    int level;
    int L;
    IplImage** ms_ssim_map;
    CvScalar ms_ssim_value;

    float *alpha;
    float *beta;
    float *gamma;

  public:

    calcMSSSIM()
    {
      K1 = 0.01;
      K2 = 0.03;
      gaussian_window = 11;
      gaussian_sigma = 1.5;
      level = 5;
      L = 255;
      ms_ssim_map = NULL;
      for (int i=0; i < 4; i++)
        ms_ssim_value.val[i] = -1; // Initialize with an out of bound value of mssim [0,1]

      float alpha_t[5] = {0.0, 0.0, 0.0, 0.0, 0.1333};
      float beta_t[5] = {0.0448, 0.2856, 0.3001, 0.2363, 0.1333};
      float gamma_t[5] = {0.0448, 0.2856, 0.3001, 0.2363, 0.1333};
      alpha = new float[level];
      beta = new float[level];
      gamma = new float[level];
      memcpy(alpha, alpha_t, sizeof(alpha_t));
      memcpy(beta, beta_t, sizeof(beta_t));
      memcpy(gamma, gamma_t, sizeof(gamma_t));
    }

    ~calcMSSSIM()
    {
      delete[] alpha;
      delete[] beta;
      delete[] gamma;
      int i;
      for (i=0; i < level; i++) {
        if(ms_ssim_map[i] != NULL)
          cvReleaseImage(&ms_ssim_map[i]);
      }
      if(ms_ssim_map !=NULL) 
        free(ms_ssim_map);
    }

    void setK1(double val) { K1 = val; }
    void setK2(double val) { K2 = val; }
    void setGaussian_window(int val) { gaussian_window = val; }
    void setGaussian_sigma(double val) { gaussian_sigma = val; }
    void setLevel(int val) { level = val; }
    void setL(int val) { L = val; }
    void setAlpha(float *val) { alpha = val; }
    void setBeta(float *val) { beta = val; }
    void setGamma(float *val) { gamma = val; }

    CvScalar getMSSSIM() { return ms_ssim_value; }
    IplImage** getMSSSIM_map() { return ms_ssim_map; }

    void releaseMSSSIM_map() {
      int i;
      for (i=0; i < level; i++) {
        if(ms_ssim_map[i] != NULL)
          cvReleaseImage(&ms_ssim_map[i]);
      }
      if(ms_ssim_map != NULL)
        free(ms_ssim_map);
    }

    // Prints all index maps of all the levels into different xml files
    int print_map()
    {
      if (ms_ssim_map == NULL)
      {
        cout<<"Error>> No Index_map_created.\n";
        return 0;
      }
      char file_name[50];
      // Printing the MS-SSIM_Map
      for (int i=0; i < level; i++)
      {
        sprintf(file_name, "output/img_MS-SSIM_map_level_%d.xml", i);
        cvSave(file_name, ms_ssim_map[i], NULL, "Testing MS-SSIM Index map");
      }
      return 1;
    }
   
    virtual CvScalar compare(IplImage *source1, IplImage *source2, Colorspace space)
    {
      int x = source1->width, y = source1->height;
      int nChan = source1->nChannels;
      int d = source1->depth;

      // creating a object of class calcSSIM 
      // setting the initial parameters values
      calcSSIM ssim;
      ssim.setK1(K1);
      ssim.setK2(K2);
      ssim.setGaussian_window(gaussian_window);
      ssim.setGaussian_sigma(gaussian_sigma);
      ssim.setL(L);
      //Creating an array of IplImages for ssim_map at various levels
      ms_ssim_map = (IplImage**)(malloc(sizeof(IplImage*)*level));

      #ifdef DEBUG
      cout<<"\nBeta = "<<beta[0]<<" "<<beta[1]<<" "<<beta[2]<<" "<<beta[3]<<"\n";
      #endif

      for (int i=0; i<level; i++)
      {
        //Downsampling of the original images
        IplImage *downsampleSrc1, *downsampleSrc2;
        //Downsampling the images
        CvSize downs_size = cvSize((int)(x/pow(2, i)), (int)(y/pow(2, i)));
        downsampleSrc1 = cvCreateImage(downs_size, d, nChan);
        downsampleSrc2 = cvCreateImage(downs_size, d, nChan);
        cvResize(source1, downsampleSrc1, CV_INTER_NN);
        cvResize(source2, downsampleSrc2, CV_INTER_NN);

        #ifdef DEBUG
        cout<<"Values at level="<<i<<" \n";
        #endif 

        ssim.compare(downsampleSrc1, downsampleSrc2, space);

        CvScalar mssim_t = ssim.getMSSIM();
        CvScalar mcs_t = ssim.getMeanCSvalue();
        ms_ssim_map[i] = ssim.getSSIM_map();
        //releasing the CS_map since not required
        ssim.releaseCS_map(); 
        
        #ifdef DEBUG
        cout<<"Size of MAP at level = "<<i<<"size = "<<ms_ssim_map[i]->width<<" "<<ms_ssim_map[i]->height<<"\n";
        #endif

        for (int j=0; j < 4; j++)
        {
          if (i == 0)
            ms_ssim_value.val[j] = pow((mcs_t.val[j]), (double)(beta[i]));
          else 
            if (i == level-1)
              ms_ssim_value.val[j] = (ms_ssim_value.val[j]) * pow((mssim_t.val[j]), (double)(beta[i]));
            else
              ms_ssim_value.val[j] = (ms_ssim_value.val[j]) * pow((mcs_t.val[j]), (double)(beta[i]));
        }
        //Release images
        cvReleaseImage(&downsampleSrc1);
        cvReleaseImage(&downsampleSrc2);

      }
      return ms_ssim_value;
    }

};


class calcQualityIndex : public SimilarityMetric {

  private:
    int B;
    IplImage *image_quality_map;
    CvScalar image_quality_value;

  public:

    calcQualityIndex()
    {
      B = 8;
      image_quality_map = NULL;
      for (int i=0; i < 4; i++)
        image_quality_value.val[i] = -2;   // Initialize with an out of bound value for image_quality_value [-1.1]
    }
    
    ~calcQualityIndex()
    {
      if(image_quality_map != NULL)
        cvReleaseImage(&image_quality_map);
    }
    
    void setB(int val) { B = val; }

    CvScalar getImageQuailty() { return image_quality_value; }
    IplImage* getImageQuality_map() { return image_quality_map; }

    void releaseImageQuality_map() {
      if(image_quality_map != NULL)
        cvReleaseImage(&image_quality_map);
    }

    int print_map()
    {
      if (image_quality_map == NULL)
      {
        cout<<"Error>> No Index_map_created.\n";
        return 0;
      }
      int x = image_quality_map->width, y = image_quality_map->height;
      int nChan = image_quality_map->nChannels, d = IPL_DEPTH_8U;
      CvSize size = cvSize(x, y);
      
      IplImage *image_quality_map_t = cvCreateImage(size, d, nChan);

      cvConvert(image_quality_map, image_quality_map_t);

      cvSave("output/imgQI.xml", image_quality_map, NULL, "TESTing Index map");
      cvSaveImage("output/imgQI.bmp", image_quality_map_t);
      cvReleaseImage(&image_quality_map_t);
      return 1;
    }
    
    virtual CvScalar compare(IplImage *source1, IplImage *source2, Colorspace space)
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
      
      // creating buffers for numerator and denominator and a temp buffer
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
      
      // ((4*sigma12).*(mu1_mu2))./((mu1_sq + mu2_sq).*(sigma1_sq + sigma2_sq))
      cvDiv(numerator, denominator, image_quality_map, 1);
    
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

};


class host_program_openCl {
  
  public:
      
      size_t source_size;
      // Get platform and device information
      cl_platform_id platform_id;
      cl_device_id device_id;
      cl_uint ret_num_devices;
      cl_uint ret_num_platforms;
      // Create an OpenCL context
      cl_context context;
      // Create a command queue
      cl_command_queue command_queue;
      // Create the OpenCL program
      cl_program program;

      cl_int ret;
    
      char * load_kernel (const char * kernel_name) {
        // Load the kernel source code into the array source_str
        FILE *fp;
        char *source_str;
        #ifdef DEBUG
        printf("%s\n",kernel_name); 
        #endif
        fp = fopen(kernel_name, "r");
        if (!fp) {
            fprintf(stderr, "Failed to load kernel.\n");
        }
        source_str = (char*)malloc(MAX_SOURCE_SIZE);
        source_size = fread( source_str, 1, MAX_SOURCE_SIZE, fp);
        fclose( fp );

        return source_str;
      }
      
      void setup () {
        
        platform_id = NULL;
        device_id = NULL;
        
        // Get platform and device information
        ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
        ret = clGetDeviceIDs( platform_id, CL_DEVICE_TYPE_ALL, 1, &device_id, &ret_num_devices);
       
        // Create an OpenCL context
        context = clCreateContext( NULL, 1, &device_id, NULL, NULL, &ret);
       
        // Create a command queue
        command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
        if( ret != CL_SUCCESS ) 
          printf( "Error : Cannot create command queue.\n" ); 
      }

      cl_kernel create_program (const char *kernel_function_name, const char *source_str, const size_t source_size) {
      
        // Create the OpenCL kernels
        cl_kernel kernel; 
        
        // Create a program from the kernel source
        program = clCreateProgramWithSource(context, 1, (const char **)&source_str, (const size_t *)&source_size, &ret);
       
        // Build the program
        ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
       
        // Create the OpenCL kernel
        kernel = clCreateKernel(program, kernel_function_name, &ret);

        return kernel;
     }

    // Clean up
    void clean_up_host() {
       ret = clFlush(command_queue);
       ret = clFinish(command_queue);
       ret = clReleaseProgram(program);
       ret = clReleaseCommandQueue(command_queue);
       ret = clReleaseContext(context);
    }

};

class MSE_openCl : public host_program_openCl, public SimilarityMetric { 

  private:
    int L;
    CvScalar mse;
    CvScalar psnr;

  public:
    MSE_openCl()
    {
      L = 255;
      for (int i=0; i < 4; i++)
        mse.val[i] = -1;
      for (int i=0; i < 4; i++)
        psnr.val[i] = -1;
    }

    void setL(int value) { L = value; }
    CvScalar getMSE() { return mse; }
    CvScalar getPSNR() { return psnr; }

    char *source_str;
    size_t source_size_mse;
    cl_kernel kernel_mse;  // Create the OpenCL kernels
    const char *mse_cl;

    void Init() {
      mse_cl = "mse.cl";
      setup();
      source_str = load_kernel(mse_cl);
      source_size_mse = source_size;
      kernel_mse = create_program("vector_mse", source_str, source_size_mse);
    }
   
    void execute_mse (float *A, float *B, float *C, int LIST_SIZE, int LOCAL_SIZE) {
  
       size_t global_item_size = LIST_SIZE;
       size_t local_item_size = LOCAL_SIZE;
       cl_event event[1];
       // Create memory buffers on the device for each vector
       cl_mem a_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);
       cl_mem b_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);
       cl_mem c_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);
   
       // Copy the lists A and B to their respective memory buffers
       ret = clEnqueueWriteBuffer(command_queue, a_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(float), A, 0, NULL, NULL);
       ret = clEnqueueWriteBuffer(command_queue, b_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(float), B, 0, NULL, NULL);
   
       // Set the arguments of the kernel
       ret = clSetKernelArg(kernel_mse, 0, sizeof(cl_mem), (void *)&a_mem_obj);
       ret = clSetKernelArg(kernel_mse, 1, sizeof(cl_mem), (void *)&b_mem_obj);
       ret = clSetKernelArg(kernel_mse, 2, sizeof(cl_mem), (void *)&c_mem_obj);
       
       // Execute the OpenCL kernel on the list
       ret = clEnqueueNDRangeKernel(command_queue, kernel_mse, 1, NULL, &global_item_size, &local_item_size, 0, NULL, &event[0]);
       if (ret!=CL_SUCCESS) {
         printf("Error: Kernel could not be executed\n"); 
         cout<<ret;
       }
       clWaitForEvents(1, &event[0]);
   
       // Read the memory buffer C on the device to the local variable C
       ret = clEnqueueReadBuffer(command_queue, c_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(float), C, 0, NULL, NULL);
       // Clean up
       ret = clReleaseMemObject(a_mem_obj);
       ret = clReleaseMemObject(b_mem_obj);
       ret = clReleaseMemObject(c_mem_obj);
    }
    
    // Clean up
    void clean_up_host() {
       ret = clReleaseKernel(kernel_mse);
    }

    virtual CvScalar compare(IplImage *source1, IplImage *source2, Colorspace space)
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
      IplImage *mse_index = cvCreateImage(size, d, nChan);
      cvConvert(src1, img1);
    	cvConvert(src2, img2);

      execute_mse((float*)(img1->imageData), (float*)(img2->imageData), (float*)(mse_index->imageData), x*y*nChan, x);
  
      mse = cvAvg(mse_index);

      psnr.val[0] = 10.0*log10((L*L)/mse.val[0]);
      psnr.val[1] = 10.0*log10((L*L)/mse.val[1]);
      psnr.val[2] = 10.0*log10((L*L)/mse.val[2]);
      psnr.val[3] = 10.0*log10((L*L)/mse.val[3]);
      
      //Release images
      cvReleaseImage(&img1);
      cvReleaseImage(&img2);
      cvReleaseImage(&mse_index);
      cvReleaseImage(&src1);
      cvReleaseImage(&src2);

      return mse;
    }

};

class SSIM_openCl : public host_program_openCl, public SimilarityMetric { 

  private:
    double K1;
    double K2;
    int gaussian_window;
    double gaussian_sigma;
    int L;
    IplImage *ssim_map;
    CvScalar mssim_value;
    IplImage *cs_map;
    CvScalar mean_cs_value; // mean of contrast, structure (part of l,c,s)

  public:

    SSIM_openCl()
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

    void setK1(double val) { K1 = val; }
    void setK2(double val) { K2 = val; }
    void setGaussian_window(int val) { gaussian_window = val; }
    void setGaussian_sigma(double val) { gaussian_sigma = val; }
    void setL(int val) { L = val; }

    CvScalar getMSSIM() { return mssim_value; }
    CvScalar getMeanCSvalue() { return mean_cs_value; }
    IplImage* getSSIM_map() { return ssim_map; }

    IplImage* getCS_map() { return cs_map; }

    void releaseSSIM_map() { 
      if (ssim_map != NULL)
        cvReleaseImage(&ssim_map);
      ssim_map = NULL;
    }
    
    void releaseCS_map() { 
      if (cs_map != NULL)
        cvReleaseImage(&cs_map); 
      cs_map = NULL;
    }

    int print_map()
    {
      if (ssim_map == NULL)
      {
        cout<<"Error>> No Index_map_created.\n";
        return 0;
      }
      int x = ssim_map->width, y = ssim_map->height;
      int nChan = ssim_map->nChannels, d = IPL_DEPTH_8U;
      CvSize size = cvSize(x, y);
      IplImage *ssim_map_t = cvCreateImage(size, d, nChan);
      cvConvert(ssim_map, ssim_map_t);
      cvSave("output/imgSSIM.xml", ssim_map, NULL, "TESTing Index map");
      cvSaveImage("output/imgSSIM.bmp", ssim_map_t);
      //Release Image
      cvReleaseImage(&ssim_map_t);
      return 1;
    }

    char *source_str_ssim1;
    char *source_str_ssim2;
    size_t source_size_ssim1;
    size_t source_size_ssim2;
    // Create the OpenCL kernels
    cl_kernel kernel_ssim1, kernel_ssim2;
    const char *ssim1;
    const char *ssim2;

    void Init() {
      
      ssim1 = "ssim_part1.cl";
      ssim2 = "ssim_part2.cl";
      setup();
      source_str_ssim1 = load_kernel(ssim1);
      source_size_ssim1 = source_size;
      kernel_ssim1 = create_program("ssim_A", source_str_ssim1, source_size_ssim1);
      source_str_ssim2 = load_kernel(ssim2);
      source_size_ssim2 = source_size;
      kernel_ssim2 = create_program("ssim_B", source_str_ssim2, source_size_ssim2);
    }
    
    void execute_ssim (float *src1, float *src2, float *filter, float *ssim, int LIST_SIZE, int LOCAL_SIZE, int x, int y, int nChan, int filter_size, float C1, float C2) {
  
       size_t global_item_size[] = {x,y};
       size_t local_item_size[] = {1,1};
       cl_event event[2];
       
       #ifdef DEBUG
       cout<<"Creating the memory buffers-\n";
       #endif
       // Create memory buffers on the device for each vector
       cl_mem src1_mem_obj      = clCreateBuffer(context, CL_MEM_READ_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);
       cl_mem src2_mem_obj      = clCreateBuffer(context, CL_MEM_READ_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);
       cl_mem filter_mem_obj    = clCreateBuffer(context, CL_MEM_READ_ONLY, filter_size*filter_size* sizeof(float), NULL, &ret);
       cl_mem img1_sq_mem_obj   = clCreateBuffer(context, CL_MEM_READ_WRITE, LIST_SIZE * sizeof(float), NULL, &ret);
       cl_mem img2_sq_mem_obj   = clCreateBuffer(context, CL_MEM_READ_WRITE, LIST_SIZE * sizeof(float), NULL, &ret);
       cl_mem img1_img2_mem_obj = clCreateBuffer(context, CL_MEM_READ_WRITE, LIST_SIZE * sizeof(float), NULL, &ret);
       cl_mem mu1_mem_obj       = clCreateBuffer(context, CL_MEM_READ_WRITE, LIST_SIZE * sizeof(float), NULL, &ret);
       cl_mem mu2_mem_obj       = clCreateBuffer(context, CL_MEM_READ_WRITE, LIST_SIZE * sizeof(float), NULL, &ret);
       cl_mem ssim_index_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);
     
       if (!src1_mem_obj || !src2_mem_obj || !filter_mem_obj || !img1_sq_mem_obj || !img2_sq_mem_obj || !img1_img2_mem_obj || 
           !mu1_mem_obj || !mu2_mem_obj || !ssim_index_mem_obj)
           cout<<"Failed to allocate device memory!\n"; 
        
       #ifdef DEBUG
       cout<<"Allocating memory buffers - \n"; 
       #endif
       // Copy the lists Image Data src1, src2 to their respective memory buffers
       ret = clEnqueueWriteBuffer(command_queue, src1_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(float), src1, 0, NULL, NULL);
       ret = clEnqueueWriteBuffer(command_queue, src2_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(float), src2, 0, NULL, NULL);
       if (ret != CL_SUCCESS)
          printf("Error: Failed to write to source array!\n");
       ret = clEnqueueWriteBuffer(command_queue, filter_mem_obj, CL_TRUE, 0, filter_size*filter_size* sizeof(float), filter, 0, NULL, NULL);
        
       #ifdef DEBUG
       cout<<"Setting arguments of kernel1 ssim- \n"; 
       #endif
       // Set the arguments of the kernel ssim1
       ret = clSetKernelArg(kernel_ssim1, 0, sizeof(cl_mem), (void *)&src1_mem_obj);
       ret = clSetKernelArg(kernel_ssim1, 1, sizeof(cl_mem), (void *)&src2_mem_obj);
       ret = clSetKernelArg(kernel_ssim1, 2, sizeof(cl_mem), (void *)&filter_mem_obj   );
       ret = clSetKernelArg(kernel_ssim1, 3, sizeof(cl_mem), (void *)&img1_sq_mem_obj  );
       ret = clSetKernelArg(kernel_ssim1, 4, sizeof(cl_mem), (void *)&img2_sq_mem_obj  );
       ret = clSetKernelArg(kernel_ssim1, 5, sizeof(cl_mem), (void *)&img1_img2_mem_obj);
       ret = clSetKernelArg(kernel_ssim1, 6, sizeof(cl_mem), (void *)&mu1_mem_obj);
       ret = clSetKernelArg(kernel_ssim1, 7, sizeof(cl_mem), (void *)&mu2_mem_obj);
       ret = clSetKernelArg(kernel_ssim1, 8, sizeof(int), &x);
       ret = clSetKernelArg(kernel_ssim1, 9, sizeof(int), &y);
       ret = clSetKernelArg(kernel_ssim1, 10, sizeof(int), &nChan);
       ret = clSetKernelArg(kernel_ssim1, 11, sizeof(int), &filter_size);
       
       // Execute the OpenCL kernel on the list
       #ifdef DEBUG
       cout<<"Executing ssim kernel1 - \n";
       #endif
      
       ret = clEnqueueNDRangeKernel(command_queue, kernel_ssim1, 2, NULL, global_item_size, local_item_size, 0, NULL, &event[0]);
       if (ret!=CL_SUCCESS) {
         printf("Error: Kernel could not be executed\n"); 
         cout<<ret;
       }
       clWaitForEvents(1, &event[0]);
       
       #ifdef DEBUG
       cout<<"Setting arguments of kernel2 ssim- \n"; 
       #endif
       // Set the arguments of the kernel ssim2
       ret = clSetKernelArg(kernel_ssim2, 0, sizeof(cl_mem), (void *)&mu1_mem_obj);
       ret = clSetKernelArg(kernel_ssim2, 1, sizeof(cl_mem), (void *)&mu2_mem_obj);
       ret = clSetKernelArg(kernel_ssim2, 2, sizeof(cl_mem), (void *)&filter_mem_obj   );
       ret = clSetKernelArg(kernel_ssim2, 3, sizeof(cl_mem), (void *)&img1_sq_mem_obj  );
       ret = clSetKernelArg(kernel_ssim2, 4, sizeof(cl_mem), (void *)&img2_sq_mem_obj  );
       ret = clSetKernelArg(kernel_ssim2, 5, sizeof(cl_mem), (void *)&img1_img2_mem_obj);
       ret = clSetKernelArg(kernel_ssim2, 6, sizeof(cl_mem), (void *)&ssim_index_mem_obj);
       ret = clSetKernelArg(kernel_ssim2, 7, sizeof(int), &x);
       ret = clSetKernelArg(kernel_ssim2, 8, sizeof(int), &y);
       ret = clSetKernelArg(kernel_ssim2, 9, sizeof(int), &nChan);
       ret = clSetKernelArg(kernel_ssim2, 10, sizeof(int), &filter_size);
       ret = clSetKernelArg(kernel_ssim2, 11, sizeof(float), &C1);
       ret = clSetKernelArg(kernel_ssim2, 12, sizeof(float), &C2);
  
       #ifdef DEBUG
       cout<<"Executing ssim kernel2 - \n";
       #endif
       // Execute the OpenCL kernel on the list
       ret = clEnqueueNDRangeKernel(command_queue, kernel_ssim2, 2, NULL, global_item_size, local_item_size, 0, NULL, &event[1]);
       clWaitForEvents(1, &event[1]);
       
       // Read the memory buffer C on the device to the local variable C
       ret = clEnqueueReadBuffer(command_queue, ssim_index_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(float), ssim, 0, NULL, NULL);
    
       #ifdef DEBUG
       cout<<"Cleaning up the memory bufffer\n"; 
       #endif
       // Clean up
       ret = clReleaseMemObject(src1_mem_obj      );
       ret = clReleaseMemObject(src2_mem_obj      );
       ret = clReleaseMemObject(filter_mem_obj    );
       ret = clReleaseMemObject(img1_sq_mem_obj   );
       ret = clReleaseMemObject(img2_sq_mem_obj   );
       ret = clReleaseMemObject(img1_img2_mem_obj );
       ret = clReleaseMemObject(mu1_mem_obj       );
       ret = clReleaseMemObject(mu2_mem_obj       );
       ret = clReleaseMemObject(ssim_index_mem_obj);
    }
    
    // Clean up
    void clean_up_host() {
       ret = clReleaseKernel(kernel_ssim1);
       ret = clReleaseKernel(kernel_ssim2);
    }

    virtual CvScalar compare(IplImage *source1, IplImage *source2, Colorspace space)
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
      IplImage *ssim_index = cvCreateImage(size, d, nChan);
      cvConvert(src1, img1);
    	cvConvert(src2, img2);
    
      const float C1 = (K1 * L) * (K1 * L); 
      const float C2 = (K2 * L) * (K2 * L);
  
      Mat tempo = get_gaussian_filter(gaussian_window,gaussian_sigma);
      float *filter = (float*)tempo.data;

      execute_ssim((float*)(img1->imageData), (float*)(img2->imageData), filter, (float*)(ssim_index->imageData), x*y*nChan, x, x, y, nChan, gaussian_window, C1, C2);
  
      mssim_value = cvAvg(ssim_index);

      //Release images
      cvReleaseImage(&img1);
      cvReleaseImage(&img2);
      cvReleaseImage(&ssim_index);
      cvReleaseImage(&src1);
      cvReleaseImage(&src2);

      return mssim_value;
    }

};

class MS_SSIM_openCl : public SSIM_openCl {

  private:
    double K1;
    double K2;
    int gaussian_window;
    double gaussian_sigma;
    int level;
    int L;
    IplImage** ms_ssim_map;
    CvScalar ms_ssim_value;

    float *alpha;
    float *beta;
    float *gamma;

  public:

    MS_SSIM_openCl()
    {
      K1 = 0.01;
      K2 = 0.03;
      gaussian_window = 11;
      gaussian_sigma = 1.5;
      level = 5;
      L = 255;
      ms_ssim_map = NULL;
      for (int i=0; i < 4; i++)
        ms_ssim_value.val[i] = -1; // Initialize with an out of bound value of mssim [0,1]

      float alpha_t[5] = {0.0, 0.0, 0.0, 0.0, 0.1333};
      float beta_t[5] = {0.0448, 0.2856, 0.3001, 0.2363, 0.1333};
      float gamma_t[5] = {0.0448, 0.2856, 0.3001, 0.2363, 0.1333};
      alpha = new float[level];
      beta = new float[level];
      gamma = new float[level];
      memcpy(alpha, alpha_t, sizeof(alpha_t));
      memcpy(beta, beta_t, sizeof(beta_t));
      memcpy(gamma, gamma_t, sizeof(gamma_t));
    }

    ~MS_SSIM_openCl()
    {
      delete[] alpha;
      delete[] beta;
      delete[] gamma;
      int i;
      for (i=0; i < level; i++) {
        if(ms_ssim_map[i] != NULL)
          cvReleaseImage(&ms_ssim_map[i]);
      }
      if(ms_ssim_map !=NULL) 
        free(ms_ssim_map);
    }

    void setK1(double val) { K1 = val; }
    void setK2(double val) { K2 = val; }
    void setGaussian_window(int val) { gaussian_window = val; }
    void setGaussian_sigma(double val) { gaussian_sigma = val; }
    void setLevel(int val) { level = val; }
    void setL(int val) { L = val; }
    void setAlpha(float *val) { alpha = val; }
    void setBeta(float *val) { beta = val; }
    void setGamma(float *val) { gamma = val; }

    CvScalar getMSSSIM() { return ms_ssim_value; }
    IplImage** getMSSSIM_map() { return ms_ssim_map; }

    void releaseMSSSIM_map() {
      int i;
      for (i=0; i < level; i++) {
        if(ms_ssim_map[i] != NULL)
          cvReleaseImage(&ms_ssim_map[i]);
      }
      if(ms_ssim_map != NULL)
        free(ms_ssim_map);
    }

    // Prints all index maps of all the levels into different xml files
    int print_map()
    {
      if (ms_ssim_map == NULL)
      {
        cout<<"Error>> No Index_map_created.\n";
        return 0;
      }
      char file_name[50];
      // Printing the MS-SSIM_Map
      for (int i=0; i < level; i++)
      {
        sprintf(file_name, "output/img_MS-SSIM_map_level_%d.xml", i);
        cvSave(file_name, ms_ssim_map[i], NULL, "Testing MS-SSIM Index map");
      }
      return 1;
    }
 
    char *source_str_ms_ssim1;
    char *source_str_ms_ssim2;
    size_t source_size_ms_ssim1;
    size_t source_size_ms_ssim2;
    // Create the OpenCL kernels
    cl_kernel kernel_ms_ssim1, kernel_ms_ssim2;
    const char *ms_ssim1;
    const char *ms_ssim2;

    void Init() {
      
      ms_ssim1 = "ms_ssim_part1.cl";
      ms_ssim2 = "ms_ssim_part2.cl";
      setup();
      source_str_ms_ssim1 = load_kernel(ms_ssim1);
      source_size_ms_ssim1 = source_size;
      kernel_ms_ssim1 = create_program("ms_ssim_A", source_str_ms_ssim1, source_size_ms_ssim1);
      source_str_ms_ssim2 = load_kernel(ms_ssim2);
      source_size_ms_ssim2 = source_size;
      kernel_ms_ssim2 = create_program("ms_ssim_B", source_str_ms_ssim2, source_size_ms_ssim2);
    }
    
    void execute_ssim_temp (float *src1, float *src2, float *filter, float *ms_ssim, float *cs_map, int LIST_SIZE, int LOCAL_SIZE, int x, int y, int nChan, int filter_size, float C1, float C2) {
  
       size_t global_item_size[] = {x,y};
       size_t local_item_size[] = {1,1};
       cl_event event[2];
       
       #ifdef DEBUG
       cout<<"Creating the memory buffers-\n";
       #endif
       // Create memory buffers on the device for each vector
       cl_mem src1_mem_obj      = clCreateBuffer(context, CL_MEM_READ_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);
       cl_mem src2_mem_obj      = clCreateBuffer(context, CL_MEM_READ_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);
       cl_mem filter_mem_obj    = clCreateBuffer(context, CL_MEM_READ_ONLY, filter_size*filter_size* sizeof(float), NULL, &ret);
       cl_mem img1_sq_mem_obj   = clCreateBuffer(context, CL_MEM_READ_WRITE, LIST_SIZE * sizeof(float), NULL, &ret);
       cl_mem img2_sq_mem_obj   = clCreateBuffer(context, CL_MEM_READ_WRITE, LIST_SIZE * sizeof(float), NULL, &ret);
       cl_mem img1_img2_mem_obj = clCreateBuffer(context, CL_MEM_READ_WRITE, LIST_SIZE * sizeof(float), NULL, &ret);
       cl_mem mu1_mem_obj       = clCreateBuffer(context, CL_MEM_READ_WRITE, LIST_SIZE * sizeof(float), NULL, &ret);
       cl_mem mu2_mem_obj       = clCreateBuffer(context, CL_MEM_READ_WRITE, LIST_SIZE * sizeof(float), NULL, &ret);
       cl_mem ms_ssim_index_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);
       cl_mem cs_index_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);
     
       if (!src1_mem_obj || !src2_mem_obj || !filter_mem_obj || !img1_sq_mem_obj || !img2_sq_mem_obj || !img1_img2_mem_obj || 
           !mu1_mem_obj || !mu2_mem_obj || !ms_ssim_index_mem_obj || !cs_index_mem_obj)
           cout<<"Failed to allocate device memory!\n"; 
        
       #ifdef DEBUG
       cout<<"Allocating memory buffers - \n"; 
       #endif
       // Copy the lists Image Data src1, src2 to their respective memory buffers
       ret = clEnqueueWriteBuffer(command_queue, src1_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(float), src1, 0, NULL, NULL);
       ret = clEnqueueWriteBuffer(command_queue, src2_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(float), src2, 0, NULL, NULL);
       if (ret != CL_SUCCESS)
          printf("Error: Failed to write to source array!\n");
       ret = clEnqueueWriteBuffer(command_queue, filter_mem_obj, CL_TRUE, 0, filter_size*filter_size* sizeof(float), filter, 0, NULL, NULL);
        
       #ifdef DEBUG
       cout<<"Setting arguments of kernel1 ms_ssim- \n"; 
       #endif
       // Set the arguments of the kernel ms_ssim1
       ret = clSetKernelArg(kernel_ms_ssim1, 0, sizeof(cl_mem), (void *)&src1_mem_obj);
       ret = clSetKernelArg(kernel_ms_ssim1, 1, sizeof(cl_mem), (void *)&src2_mem_obj);
       ret = clSetKernelArg(kernel_ms_ssim1, 2, sizeof(cl_mem), (void *)&filter_mem_obj   );
       ret = clSetKernelArg(kernel_ms_ssim1, 3, sizeof(cl_mem), (void *)&img1_sq_mem_obj  );
       ret = clSetKernelArg(kernel_ms_ssim1, 4, sizeof(cl_mem), (void *)&img2_sq_mem_obj  );
       ret = clSetKernelArg(kernel_ms_ssim1, 5, sizeof(cl_mem), (void *)&img1_img2_mem_obj);
       ret = clSetKernelArg(kernel_ms_ssim1, 6, sizeof(cl_mem), (void *)&mu1_mem_obj);
       ret = clSetKernelArg(kernel_ms_ssim1, 7, sizeof(cl_mem), (void *)&mu2_mem_obj);
       ret = clSetKernelArg(kernel_ms_ssim1, 8, sizeof(int), &x);
       ret = clSetKernelArg(kernel_ms_ssim1, 9, sizeof(int), &y);
       ret = clSetKernelArg(kernel_ms_ssim1, 10, sizeof(int), &nChan);
       ret = clSetKernelArg(kernel_ms_ssim1, 11, sizeof(int), &filter_size);
       
       // Execute the OpenCL kernel on the list
       #ifdef DEBUG
       cout<<"Executing ms_ssim kernel1 - \n";
       #endif
       //for(int l=1; l <100;l++)
        ret = clEnqueueNDRangeKernel(command_queue, kernel_ms_ssim1, 2, NULL, global_item_size, local_item_size, 0, NULL, &event[0]);
       if (ret!=CL_SUCCESS) {
         printf("Error: Kernel could not be executed\n"); 
         cout<<ret;
       }
       clWaitForEvents(1, &event[0]);
       
       //ret = clEnqueueReadBuffer(command_queue, mu1_mem_obj, CL_TRUE, 0, LIST_SIZE*sizeof(float), ms_ssim, 0, NULL, NULL);
       #ifdef DEBUG
       cout<<"Setting arguments of kernel2 ms_ssim- \n"; 
       #endif
       // Set the arguments of the kernel ms_ssim2
       ret = clSetKernelArg(kernel_ms_ssim2, 0, sizeof(cl_mem), (void *)&mu1_mem_obj);
       ret = clSetKernelArg(kernel_ms_ssim2, 1, sizeof(cl_mem), (void *)&mu2_mem_obj);
       ret = clSetKernelArg(kernel_ms_ssim2, 2, sizeof(cl_mem), (void *)&filter_mem_obj   );
       ret = clSetKernelArg(kernel_ms_ssim2, 3, sizeof(cl_mem), (void *)&img1_sq_mem_obj  );
       ret = clSetKernelArg(kernel_ms_ssim2, 4, sizeof(cl_mem), (void *)&img2_sq_mem_obj  );
       ret = clSetKernelArg(kernel_ms_ssim2, 5, sizeof(cl_mem), (void *)&img1_img2_mem_obj);
       ret = clSetKernelArg(kernel_ms_ssim2, 6, sizeof(cl_mem), (void *)&ms_ssim_index_mem_obj);
       ret = clSetKernelArg(kernel_ms_ssim2, 7, sizeof(cl_mem), (void *)&cs_index_mem_obj);
       ret = clSetKernelArg(kernel_ms_ssim2, 8, sizeof(int), &x);
       ret = clSetKernelArg(kernel_ms_ssim2, 9, sizeof(int), &y);
       ret = clSetKernelArg(kernel_ms_ssim2, 10, sizeof(int), &nChan);
       ret = clSetKernelArg(kernel_ms_ssim2, 11, sizeof(int), &filter_size);
       ret = clSetKernelArg(kernel_ms_ssim2, 12, sizeof(float), &C1);
       ret = clSetKernelArg(kernel_ms_ssim2, 13, sizeof(float), &C2);
  
       #ifdef DEBUG
       cout<<"Executing ms_ssim kernel2 - \n";
       #endif
       // Execute the OpenCL kernel on the list
       ret = clEnqueueNDRangeKernel(command_queue, kernel_ms_ssim2, 2, NULL, global_item_size, local_item_size, 0, NULL, &event[1]);
       clWaitForEvents(1, &event[1]);
       
       // Read the memory buffer C on the device to the local variable C
       ret = clEnqueueReadBuffer(command_queue, ms_ssim_index_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(float), ms_ssim, 0, NULL, NULL);
       ret = clEnqueueReadBuffer(command_queue, cs_index_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(float), cs_map, 0, NULL, NULL);
    
       #ifdef DEBUG
       cout<<"Cleaning up the memory bufffer\n"; 
       #endif
       // Clean up
       ret = clReleaseMemObject(src1_mem_obj      );
       ret = clReleaseMemObject(src2_mem_obj      );
       ret = clReleaseMemObject(filter_mem_obj    );
       ret = clReleaseMemObject(img1_sq_mem_obj   );
       ret = clReleaseMemObject(img2_sq_mem_obj   );
       ret = clReleaseMemObject(img1_img2_mem_obj );
       ret = clReleaseMemObject(mu1_mem_obj       );
       ret = clReleaseMemObject(mu2_mem_obj       );
       ret = clReleaseMemObject(ms_ssim_index_mem_obj);
       ret = clReleaseMemObject(cs_index_mem_obj);
    }
    
    // Clean up
    void clean_up_host() {
       ret = clReleaseKernel(kernel_ms_ssim1);
       ret = clReleaseKernel(kernel_ms_ssim2);
    }
  
    CvScalar execute_ms_ssim (IplImage *src1, IplImage *src2, float *filter, int LIST_SIZE, int LOCAL_SIZE, int x, int y, int nChan, int filter_size, float C1, float C2, int level) {
   
      ms_ssim_map = (IplImage**)(malloc(sizeof(IplImage*)*level));
      int d = IPL_DEPTH_32F;
      #ifdef DEBUG
      cout<<"\nBeta = "<<beta[0]<<" "<<beta[1]<<" "<<beta[2]<<" "<<beta[3]<<"\n";
      #endif

      for (int i=0; i<level; i++)
      {
        //creating a cs_map image
        IplImage *cs_map;
        //Downsampling of the original images
        IplImage *downsampleSrc1, *downsampleSrc2;
        //Downsampling the images
        CvSize downs_size = cvSize((int)(x/pow(2, i)), (int)(y/pow(2, i)));
        downsampleSrc1 = cvCreateImage(downs_size, d, nChan);
        downsampleSrc2 = cvCreateImage(downs_size, d, nChan);
        cvResize(src1, downsampleSrc1, CV_INTER_NN);
        cvResize(src2, downsampleSrc2, CV_INTER_NN);
        ms_ssim_map[i] = cvCreateImage(downs_size, d, nChan);
        cs_map = cvCreateImage(downs_size, d, nChan);
        CvScalar test_avg = cvAvg(downsampleSrc1);
        #ifdef DEBUG
        cout<<"Checking for average value - "<<test_avg.val[0]<<" "<<test_avg.val[1]<<" "<<test_avg.val[2]<<"\n";
        #endif

        int x1 = downsampleSrc1->height;
        int y1 = downsampleSrc1->width;
        LIST_SIZE = x1*y1*nChan;
        LOCAL_SIZE = 1 ;
        #ifdef DEBUG
        cout<<"Values at level="<<i<<" \n";
        #endif 

        execute_ssim_temp((float*)(downsampleSrc1->imageData), (float*)(downsampleSrc2->imageData), filter, (float*)(ms_ssim_map[i]->imageData), (float*)(cs_map->imageData), LIST_SIZE,LOCAL_SIZE,x1,y1,nChan,11, C1, C2);

        CvScalar mssim_t = cvAvg(ms_ssim_map[i]);
        CvScalar mcs_t = cvAvg(cs_map);
        
        #ifdef DEBUG
        cout<<"Size of MAP at level = "<<i<<" size = "<<ms_ssim_map[i]->width<<" "<<ms_ssim_map[i]->height<<"\n";
        cout<<"Some values for testing - "<<mssim_t.val[0]<<" "<<mssim_t.val[1]<<" "<<mssim_t.val[2]<<"\n";
        #endif

        for (int j=0; j < 4; j++)
        {
          if (i == 0)
            ms_ssim_value.val[j] = pow((mcs_t.val[j]), (double)(beta[i]));
          else 
            if (i == level-1)
              ms_ssim_value.val[j] = (ms_ssim_value.val[j]) * pow((mssim_t.val[j]), (double)(beta[i]));
            else
              ms_ssim_value.val[j] = (ms_ssim_value.val[j]) * pow((mcs_t.val[j]), (double)(beta[i]));
        }
        //Release images
        cvReleaseImage(&downsampleSrc1);
        cvReleaseImage(&downsampleSrc2);
        cvReleaseImage(&cs_map);

      }
      return ms_ssim_value;
    }

    virtual CvScalar compare(IplImage *source1, IplImage *source2, Colorspace space)
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
      IplImage *ms_ssim_map = cvCreateImage(size, d, nChan);
      cvConvert(src1, img1);
    	cvConvert(src2, img2);
    
      const float C1 = (K1 * L) * (K1 * L); 
      const float C2 = (K2 * L) * (K2 * L);
  
      Mat tempo = get_gaussian_filter(gaussian_window,gaussian_sigma);
      float *filter = (float*)tempo.data;

      ms_ssim_value  = execute_ms_ssim(img1, img2, filter, x*y*nChan, x,x,y,nChan,11, C1, C2, level);

      //Release images
      cvReleaseImage(&img1);
      cvReleaseImage(&img2);
      //I have tto release ms_ssim
      cvReleaseImage(&src1);
      cvReleaseImage(&src2);

      return ms_ssim_value;
    }

};

class ImageQuI_openCl : public host_program_openCl, public SimilarityMetric { 

  private:
    int B;
    IplImage *image_quality_map;
    CvScalar image_quality_value;

  public:

    ImageQuI_openCl()
    {
      B = 8;
      image_quality_map = NULL;
      for (int i=0; i < 4; i++)
        image_quality_value.val[i] = -2;   // Initialize with an out of bound value for image_quality_value [-1.1]
    }
    
    ~ImageQuI_openCl()
    {
      if(image_quality_map != NULL)
        cvReleaseImage(&image_quality_map);
    }
    
    void setB(int val) { B = val; }

    CvScalar getImageQuailty() { return image_quality_value; }
    IplImage* getImageQuality_map() { return image_quality_map; }

    void releaseImageQuality_map() {
      if(image_quality_map != NULL)
        cvReleaseImage(&image_quality_map);
    }

    int print_map()
    {
      if (image_quality_map == NULL)
      {
        cout<<"Error>> No Index_map_created.\n";
        return 0;
      }
      int x = image_quality_map->width, y = image_quality_map->height;
      int nChan = image_quality_map->nChannels, d = IPL_DEPTH_8U;
      CvSize size = cvSize(x, y);
      
      IplImage *image_quality_map_t = cvCreateImage(size, d, nChan);

      cvConvert(image_quality_map, image_quality_map_t);

      cvSave("output/imgQI.xml", image_quality_map, NULL, "TESTing Index map");
      cvSaveImage("output/imgQI.bmp", image_quality_map_t);
      cvReleaseImage(&image_quality_map_t);
      return 1;
    }
 
    char *source_str_iqi1;
    char *source_str_iqi2;
    size_t source_size_iqi1;
    size_t source_size_iqi2;
    // Create the OpenCL kernels
    cl_kernel kernel_iqi1, kernel_iqi2;
    const char *iqi1;
    const char *iqi2;

    void Init() {
      
      iqi1 = "img_qi1.cl";
      iqi2 = "img_qi2.cl";
      setup();
      source_str_iqi1 = load_kernel(iqi1);
      source_size_iqi1 = source_size;
      kernel_iqi1 = create_program("img_qi_A", source_str_iqi1, source_size_iqi1);
      source_str_iqi2 = load_kernel(iqi2);
      source_size_iqi2 = source_size;
      kernel_iqi2 = create_program("img_qi_B", source_str_iqi2, source_size_iqi2);
    }
    
    void execute_iqi (float *src1, float *src2, float *iqi, int LIST_SIZE, int LOCAL_SIZE, int x, int y, int nChan, int filter_size) {
  
       size_t global_item_size[] = {x,y};
       size_t local_item_size[] = {1,1};
       cl_event event[2];
       
       #ifdef DEBUG
       cout<<"Creating the memory buffers-\n";
       #endif
       // Create memory buffers on the device for each vector
       cl_mem src1_mem_obj      = clCreateBuffer(context, CL_MEM_READ_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);
       cl_mem src2_mem_obj      = clCreateBuffer(context, CL_MEM_READ_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);
       cl_mem img1_sq_mem_obj   = clCreateBuffer(context, CL_MEM_READ_WRITE, LIST_SIZE * sizeof(float), NULL, &ret);
       cl_mem img2_sq_mem_obj   = clCreateBuffer(context, CL_MEM_READ_WRITE, LIST_SIZE * sizeof(float), NULL, &ret);
       cl_mem img1_img2_mem_obj = clCreateBuffer(context, CL_MEM_READ_WRITE, LIST_SIZE * sizeof(float), NULL, &ret);
       cl_mem mu1_mem_obj       = clCreateBuffer(context, CL_MEM_READ_WRITE, LIST_SIZE * sizeof(float), NULL, &ret);
       cl_mem mu2_mem_obj       = clCreateBuffer(context, CL_MEM_READ_WRITE, LIST_SIZE * sizeof(float), NULL, &ret);
       cl_mem iqi_index_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);
     
       if (!src1_mem_obj || !src2_mem_obj || !img1_sq_mem_obj || !img2_sq_mem_obj || !img1_img2_mem_obj || 
           !mu1_mem_obj || !mu2_mem_obj || !iqi_index_mem_obj)
           cout<<"Failed to allocate device memory!\n"; 
        
       #ifdef DEBUG
       cout<<"Allocating memory buffers - \n"; 
       #endif
       // Copy the lists Image Data src1, src2 to their respective memory buffers
       ret = clEnqueueWriteBuffer(command_queue, src1_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(float), src1, 0, NULL, NULL);
       ret = clEnqueueWriteBuffer(command_queue, src2_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(float), src2, 0, NULL, NULL);
       if (ret != CL_SUCCESS)
          printf("Error: Failed to write to source array!\n");
        
       #ifdef DEBUG
       cout<<"Setting arguments of kernel1 iqi- \n"; 
       #endif
       // Set the arguments of the kernel iqi1
       ret = clSetKernelArg(kernel_iqi1, 0, sizeof(cl_mem), (void *)&src1_mem_obj);
       ret = clSetKernelArg(kernel_iqi1, 1, sizeof(cl_mem), (void *)&src2_mem_obj);
       ret = clSetKernelArg(kernel_iqi1, 2, sizeof(cl_mem), (void *)&img1_sq_mem_obj  );
       ret = clSetKernelArg(kernel_iqi1, 3, sizeof(cl_mem), (void *)&img2_sq_mem_obj  );
       ret = clSetKernelArg(kernel_iqi1, 4, sizeof(cl_mem), (void *)&img1_img2_mem_obj);
       ret = clSetKernelArg(kernel_iqi1, 5, sizeof(cl_mem), (void *)&mu1_mem_obj);
       ret = clSetKernelArg(kernel_iqi1, 6, sizeof(cl_mem), (void *)&mu2_mem_obj);
       ret = clSetKernelArg(kernel_iqi1, 7, sizeof(int), &x);
       ret = clSetKernelArg(kernel_iqi1, 8, sizeof(int), &y);
       ret = clSetKernelArg(kernel_iqi1, 9, sizeof(int), &nChan);
       ret = clSetKernelArg(kernel_iqi1, 10, sizeof(int), &filter_size);
       
       // Execute the OpenCL kernel on the list
       #ifdef DEBUG
       cout<<"Executing iqi kernel1 - \n";
       #endif
       //for(int l=1; l <100;l++)
        ret = clEnqueueNDRangeKernel(command_queue, kernel_iqi1, 2, NULL, global_item_size, local_item_size, 0, NULL, &event[0]);
       if (ret!=CL_SUCCESS) {
         printf("Error: Kernel could not be executed\n"); 
         cout<<ret;
       }
       clWaitForEvents(1, &event[0]);
       
       #ifdef DEBUG
       cout<<"Setting arguments of kernel2 iqi- \n"; 
       #endif
       // Set the arguments of the kernel iqi2
       ret = clSetKernelArg(kernel_iqi2, 0, sizeof(cl_mem), (void *)&mu1_mem_obj);
       ret = clSetKernelArg(kernel_iqi2, 1, sizeof(cl_mem), (void *)&mu2_mem_obj);
       ret = clSetKernelArg(kernel_iqi2, 2, sizeof(cl_mem), (void *)&img1_sq_mem_obj  );
       ret = clSetKernelArg(kernel_iqi2, 3, sizeof(cl_mem), (void *)&img2_sq_mem_obj  );
       ret = clSetKernelArg(kernel_iqi2, 4, sizeof(cl_mem), (void *)&img1_img2_mem_obj);
       ret = clSetKernelArg(kernel_iqi2, 5, sizeof(cl_mem), (void *)&iqi_index_mem_obj);
       ret = clSetKernelArg(kernel_iqi2, 6, sizeof(int), &x);
       ret = clSetKernelArg(kernel_iqi2, 7, sizeof(int), &y);
       ret = clSetKernelArg(kernel_iqi2, 8, sizeof(int), &nChan);
       ret = clSetKernelArg(kernel_iqi2, 9, sizeof(int), &filter_size);
  
       #ifdef DEBUG
       cout<<"Executing iqi kernel2 - \n";
       #endif
       // Execute the OpenCL kernel on the list
       ret = clEnqueueNDRangeKernel(command_queue, kernel_iqi2, 2, NULL, global_item_size, local_item_size, 0, NULL, &event[1]);
       clWaitForEvents(1, &event[1]);
       
       // Read the memory buffer C on the device to the local variable C
       ret = clEnqueueReadBuffer(command_queue, iqi_index_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(float), iqi, 0, NULL, NULL);
    
       #ifdef DEBUG
       cout<<"Cleaning up the memory bufffer\n"; 
       #endif
       // Clean up
       ret = clReleaseMemObject(src1_mem_obj      );
       ret = clReleaseMemObject(src2_mem_obj      );
       ret = clReleaseMemObject(img1_sq_mem_obj   );
       ret = clReleaseMemObject(img2_sq_mem_obj   );
       ret = clReleaseMemObject(img1_img2_mem_obj );
       ret = clReleaseMemObject(mu1_mem_obj       );
       ret = clReleaseMemObject(mu2_mem_obj       );
       ret = clReleaseMemObject(iqi_index_mem_obj );
    }
    
    // Clean up
    void clean_up_host() {
       ret = clReleaseKernel(kernel_iqi1);
       ret = clReleaseKernel(kernel_iqi2);
    }

    virtual CvScalar compare(IplImage *source1, IplImage *source2, Colorspace space)
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
      IplImage *image_quality_map = cvCreateImage(size, d, nChan);
      cvConvert(src1, img1);
    	cvConvert(src2, img2);
  
      execute_iqi((float*)(img1->imageData), (float*)(img2->imageData), (float*)(image_quality_map->imageData),x*y*nChan,x,x,y,nChan,B);
  
      image_quality_value = cvAvg(image_quality_map);

      //Release images
      cvReleaseImage(&img1);
      cvReleaseImage(&img2);
      cvReleaseImage(&src1);
      cvReleaseImage(&src2);

      return image_quality_value;
    }

};


int main (int argc, char **argv) {

  struct timeval start_time;
  struct timeval end_time;
  double start_dtime, end_dtime, diff;
  gettimeofday(&start_time,NULL);
 
  // Reading Images
  IplImage *src1;
  IplImage *src2;

  // Result will be stored in res
  CvScalar res;
  // Creating Objects of Metrics - Normal
  calcMSE mse;
  calcSSIM ssim;
  calcPSNR psnr;
  calcMSSSIM msssim;
  calcQualityIndex iqi;

  // Creating Objects of Metrics - OpenCl
  MSE_openCl M;
  SSIM_openCl S;
  MS_SSIM_openCl MS;
  ImageQuI_openCl I;

  // Initializing the OpenCl Objects - compile kernels
  M.Init();
  S.Init();
  MS.Init();
  I.Init();
  cout<<"Finished Initialization\n";
  
  // Setting up the options
  int c;
  int algo = 1; // default algo MSE
  Colorspace space;
  space = GRAYSCALE; // default color space
  bool opencl= false; // default no opencl
  char output_file[50];
  char img_name1[50], img_name2[50];
  static struct option long_options[] = {
      {"algorithm", 1, 0, 'a'},
      {"colorspace", 1, 0, 'c'},
      {"opencl", 0, 0, 'p'},
      {"L", 1, 0, 'L'},
      {"K1", 1, 0, '1'},
      {"K2", 1, 0, '2'},
      {"gaussian_window_size", 1, 0, 'w'},
      {"sigma", 1, 0, 's'},
      {"level", 1, 0, 'l'},
      {"B", 1, 0, 'B'},
      {"out", 1, 0, 'o'},
      {"image1", 1, 0, 1},
      {"image2", 1, 0, 2},
      {NULL, 0, NULL, 0}
  };
  int option_index = 0;
  
  while ((c = getopt_long(argc, argv, "a:pc:L:1:2:s:l:B:o:", long_options, &option_index)) != -1) {
      
    int this_option_optind = optind ? optind : 1;
    switch (c) {
      
      case 'a':
        char algorithm[20];
        sscanf(optarg, "%s", algorithm);
          printf("Algorithm - %s \n",algorithm);
          if(strcmp(algorithm,"mse")==0)
              algo=1;
          if(strcmp(algorithm,"psnr")==0)
              algo=2;
          if(strcmp(algorithm,"ssim")==0)
              algo=3;
          if(strcmp(algorithm,"msssim")==0)
              algo=4;
          if(strcmp(algorithm,"iqi")==0)
              algo=5;
          break;
      
      case 'c':
          int color_code;
          cout<<"Option colorspace\n";
          sscanf(optarg, "%d", &color_code);
          switch (color_code) {
            case 0:
              space = GRAYSCALE;
              break;
            case 1:
              space = RGB;
              break;
            case 2:
              space = YCbCr;
              break;
            default:
              space = GRAYSCALE;
              break;
          }
          cout<<"Space = "<<space<<"\n";
          break;

      case 'p':
          opencl = true;
          break;
      
      case 'o':
          sscanf(optarg, "%s",output_file );
          printf("output_file - %s \n",output_file);
          break;

      case 1:
        sscanf(optarg, "%s", img_name1);
    		printf("Image1 : %s\n", img_name1);
        break;
      
      case 2:
        sscanf(optarg, "%s", img_name2);
    		printf("Image2 : %s\n", img_name2);
        break;
      
      case 'L':
        int L;
        sscanf(optarg, "%d", &L);
        printf("Setting L value = %d\n", L);
        psnr.setL(L);
        ssim.setL(L);
        msssim.setL(L);
        break;
      
      case '1':
        double K1;
        sscanf(optarg, "%lf", &K1);
        printf("Setting K1 value = %lf\n", K1);
        ssim.setK1(K1);
        msssim.setK1(K1);
        break;

      case '2':
        double K2;
        sscanf(optarg, "%lf", &K2);
        printf("Setting K2 value = %lf\n", K2);
        ssim.setK2(K2);
        msssim.setK2(K2);
        break;
      
      case 'w':
        int w;
        sscanf(optarg, "%d", &w);
        printf("Setting gaussian window = %d\n", w);
        if(w%2==0)
          w++;
        ssim.setGaussian_window(w);
        msssim.setGaussian_window(w);
        break;
      
      case 's':
        double sigma;
        sscanf(optarg, "%lf", &sigma);
        printf("Setting gaussian sigma = %lf\n", sigma);
        ssim.setGaussian_sigma(sigma);
        msssim.setGaussian_sigma(sigma);
        break;
      
      case 'l':
        int level;
        sscanf(optarg, "%d", &level);
        printf("Setting level = %d\n", level);
        msssim.setLevel(level);
        break;
      
      case 'B':
        int B;
        sscanf(optarg, "%d", &B);
        printf("Setting B = %d\n", B);
        iqi.setB(B);
        break;
      
      case '?':
          break;
  
      default:
          printf ("?? getopt returned character code 0%o ??\n", c);

    }
  }
 
  // Finished with get_opt_long 
  cout<<"Finished with get_opt_long\n";
  src1 = cvLoadImage(img_name1);
  src2 = cvLoadImage(img_name2);

  switch(algo)
  {
    case 1:
      if(opencl==false) {
        res = mse.compare(src1,src2,space);
        printCvScalar(res,"MSE");
      } 
      else {
        res = M.compare(src1,src2,space);
        printCvScalar(res,"MSE_opencl");
      }
      break;
    case 2:
      if(opencl==false) {
        res = psnr.compare(src1,src2,space);
        printCvScalar(res,"PSNR");
      }
      else {
        res = M.compare(src1,src2,space);
        res = M.getPSNR();
        printCvScalar(res,"PSNR_openCl");
      }
      break;
    case 3:
      if(opencl==false) {
        res = ssim.compare(src1,src2,space);
        printCvScalar(res,"SSIM");
      }
      else {
        res = S.compare(src1,src2,space);
        printCvScalar(res,"SSIM_opencl");
      }
      break;
    case 4:
      if(opencl==false) {
        res = msssim.compare(src1,src2,space);
        printCvScalar(res,"MSSSIM");
      }
      else {
        res = MS.compare(src1,src2,space);
        printCvScalar(res,"MSSSIM_opencl");
      }
      break;
    case 5:
      if(opencl==false) {
        res = iqi.compare(src1,src2,space);
        printCvScalar(res,"IQI");
      }
      else {
        res = I.compare(src1,src2,space);
        printCvScalar(res,"IQI_opencl");
      }
      break;
    default:
      cout<<"Invalid choice\n";
  }

  //Release images
  cvReleaseImage(&src1);
  cvReleaseImage(&src2);

  //Will be used to calculate time
  /* 
  gettimeofday(&end_time,NULL);
  start_dtime=(double)start_time.tv_sec+(double)start_time.tv_usec/1000000.0;
  end_dtime=(double)end_time.tv_sec+(double)end_time.tv_usec/1000000.0; 
  diff=end_dtime-start_dtime;
  printf("Reading Buffer - %ld %ld %f %ld %ld %f %f\n",start_time.tv_sec,start_time.tv_usec,start_dtime,end_time.tv_sec,end_time.tv_usec,end_dtime,diff); 
  */
 
  exit(0);

}