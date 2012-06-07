/*
 * =====================================================================================
 *
 *       Filename:  metric.cpp
 *
 *    Description:  OpenCV Implementation of Image Similarity Metrics
 *                  MSE, PSNR, MSSIM, Uiniversal Image Quality
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

enum Colorspace {
  GRAYSCALE,
  RGB,
  YCbCr
};

// This class will contain some primitive and useful functions
class CoreFunctions {
  
  public:
    void printCvScalar(CvScalar value, Colorspace space, int nChan)
    {
      cout<<nChan<<"\t :\t"<<space<<" :\t"<<value.val[0]<<" :\t "<<value.val[1]<<" :\t "<<value.val[2]<<" :\t "<<value.val[3]<<"\n\n";
    }

};


class SimilarityMetric {

  public:
    virtual CvScalar compare(IplImage* src1, IplImage* src2, Colorspace space)
    {
      CvScalar result;
      for(int i=0; i < 4; i++)
        result.val[i] = 0;
    }

    int colorspaceConversion(IplImage* source1, IplImage* source2, IplImage** src1, IplImage** src2, Colorspace space)
    {
      *src1 = source1;
      *src2 = source2;
      int x = source1->width, y = source1->height;
      switch( space )
      {
        case GRAYSCALE:
          if(source1->nChannels != 1)
          {
            *src1 = cvCreateImage(cvSize(x,y), IPL_DEPTH_8U, 1); 
            *src2 = cvCreateImage(cvSize(x,y), IPL_DEPTH_8U, 1); 
            cvCvtColor(source1, *src1, CV_RGB2GRAY); //color images are BGR in openCV !
            cvCvtColor(source2, *src2, CV_RGB2GRAY); //color images are BGR in openCV !
          }
          break;

        case RGB:
          if(source1->nChannels == 1)
            ;//cout<<"GrayImage\n";
          break;

        case YCbCr:
          if(source1->nChannels != 1)
          {
            *src1 = cvCreateImage(cvSize(x,y), IPL_DEPTH_8U, 3); 
            *src2 = cvCreateImage(cvSize(x,y), IPL_DEPTH_8U, 3); 
            cvCvtColor(source1, *src1, CV_RGB2YCrCb); //color images are YCrCb in openCV !
            cvCvtColor(source2, *src2, CV_RGB2YCrCb); //color images are YCrCb in openCV !
          }
          break;

        default:
          cout<<"INVALID ColorSpace !!\n";
      }
      return 1;
    }

};

class calcMSE : public SimilarityMetric {

  private:
    CvScalar mse;

  public:
    calcMSE()
    {
      for(int i=0; i < 4; i++)
        mse.val[i] = -1;
    }

    CvScalar getMSE() { return mse; }

    virtual CvScalar compare(IplImage* source1, IplImage* source2, Colorspace space)
    {
      IplImage* src1, * src2;
      int status = colorspaceConversion(source1, source2, &src1, &src2, space);
      
      int x = src1->width, y = src1->height;
      int nChan = src1->nChannels, d = IPL_DEPTH_32F;
      //size before down sampling
      CvSize size = cvSize(x, y);
      
      //creating diff and difference squares
      IplImage* img1 = cvCreateImage(size, d, nChan);
      IplImage* img2 = cvCreateImage(size, d, nChan);
      IplImage* diff = cvCreateImage(size, d, nChan);
      IplImage* diff_sq = cvCreateImage(size, d, nChan);
  
      cvConvert(src1, img1);
    	cvConvert(src2, img2);
      cvAbsDiff(img1, img2, diff);
      //Squaring the images thus created
      cvPow(diff, diff_sq, 2);
  
      CvScalar mse = cvAvg(diff_sq);
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
      for(int i=0; i < 4; i++)
        mse.val[i] = -1;
      for(int i=0; i < 4; i++)
        PSNR.val[i] = -1;
    }

    void setL(int value) { L = value; }
    void setMSE(CvScalar value) { mse = value; }
    CvScalar getPSNR() { return PSNR; }

    virtual CvScalar compare(IplImage* source1, IplImage* source2, Colorspace space)
    {
      IplImage* src1, * src2;
      int status = colorspaceConversion(source1, source2, &src1, &src2, space);

      //if(mse.val[0]<0)
      if(1)
      {
        int x = src1->width, y = src1->height;
        int nChan = src1->nChannels, d = IPL_DEPTH_32F;
        //size before down sampling
        CvSize size = cvSize(x, y);
        
        //creating diff and difference squares
        IplImage* img1 = cvCreateImage(size, d, nChan);
        IplImage* img2 = cvCreateImage(size, d, nChan);
        IplImage* diff = cvCreateImage(size, d, nChan);
        IplImage* diff_sq = cvCreateImage(size, d, nChan);
        
        cvConvert(src1, img1);
    	  cvConvert(src2, img2);
        cvAbsDiff(img1, img2, diff);
        //Squaring the images thus created
        cvPow(diff, diff_sq, 2);
        
        mse = cvAvg(diff_sq);
      }
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
    IplImage* ssim_map;
    CvScalar mssim_value;
    IplImage* cs_map;
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
      for(int i=0; i < 4; i++)
        mssim_value.val[i] = -1; // Initialize with an out of bound value of mssim [0,1]
      cs_map = NULL;
      for(int i=0; i < 4; i++)
        mean_cs_value.val[i] = -1; // Initialize with an out of bound value of mssim [0,1]
    }

    void setK1(double val) { K1 = val; }
    void setK2(double val) { K2 = val; }
    void setGaussian_window(int val) { gaussian_window = val; }
    void setGaussian_sigma(int val) { gaussian_sigma = val; }
    void setL(int val) { L = val; }

    CvScalar getMSSIM() { return mssim_value; }
    CvScalar getMeanCSvalue() { return mean_cs_value; }
    void getSSIM_map(IplImage** output_map) { *output_map = ssim_map; }
    void getCS_map(IplImage* output_map) { output_map = cs_map; }
   
    int print_map()
    {
      
      if(ssim_map == NULL)
      {
        cout<<"Error>> No Index_map_created.\n";
        return 0;
      }
      
      int x = ssim_map->width, y = ssim_map->height;
      int nChan = ssim_map->nChannels, d = IPL_DEPTH_8U;
      CvSize size = cvSize(x, y);
      IplImage* ssim_map_t = cvCreateImage(size, d, nChan);
      cvConvert(ssim_map, ssim_map_t);
      cvSave("output/imgSSIM.xml", ssim_map, NULL, "TESTing Index map");
      cvSaveImage("output/imgSSIM.bmp", ssim_map_t);
      return 1;
    
    }

    virtual CvScalar compare(IplImage* source1, IplImage* source2, Colorspace space)
    {
      IplImage* src1, * src2;
      int status = colorspaceConversion(source1, source2, &src1, &src2, space);
      
      int x = source1->width, y = source1->height;
      // default settings
      const double C1 = (K1 * L) * (K1 * L); 
      const double C2 = (K2 * L) * (K2 * L);
 
      int nChan = src1->nChannels;
      int d = IPL_DEPTH_32F;
      CvSize size = cvSize(x, y);
      
      //creating FLOAT type images of src1 and src2 
      IplImage* img1 = cvCreateImage(size, d, nChan);
      IplImage* img2 = cvCreateImage(size, d, nChan);
  
      //Image squares
      IplImage* img1_sq = cvCreateImage(size, d, nChan);
      IplImage* img2_sq = cvCreateImage(size, d, nChan);
      IplImage* img1_img2 = cvCreateImage(size, d, nChan);
  
      cvConvert(src1, img1);
 		  cvConvert(src2, img2);
      
      //Squaring the images thus created
      cvPow(img1, img1_sq, 2);
      cvPow(img2, img2_sq, 2);
      cvMul(img1, img2, img1_img2, 1);
  
      IplImage* mu1 = cvCreateImage(size, d, nChan);
      IplImage* mu2 = cvCreateImage(size, d, nChan);
      IplImage* mu1_sq = cvCreateImage(size, d, nChan);
      IplImage* mu2_sq = cvCreateImage(size, d, nChan);
      IplImage* mu1_mu2 = cvCreateImage(size, d, nChan);
  
      IplImage* sigma1_sq = cvCreateImage(size, d, nChan);
      IplImage* sigma2_sq = cvCreateImage(size, d, nChan);
      IplImage* sigma12 = cvCreateImage(size, d, nChan);
  
      // creating buffers for numerator and denominator and a temp buffer
      IplImage* numerator1 = cvCreateImage(size, d, nChan);
      IplImage* numerator2 = cvCreateImage(size, d, nChan);
      IplImage* numerator = cvCreateImage(size, d, nChan);
      IplImage* denominator1 = cvCreateImage(size, d, nChan);
      IplImage* denominator2 = cvCreateImage(size, d, nChan);
      IplImage* denominator = cvCreateImage(size, d, nChan);
      //ssim map and cs_map
      //IplImage* ssim_map = cvCreateImage(size, d, nChan);
      ssim_map = cvCreateImage(size, d, nChan);
      cs_map = cvCreateImage(size, d, nChan);
  
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
      cvAddWeighted(sigma1_sq, 1, mu1_sq, -1, 0, sigma1_sq);
      //cvSub(sigma1_sq, mu1_sq, sigma1_sq);
  
      cvSmooth(img2_sq, sigma2_sq, CV_GAUSSIAN, gaussian_window, gaussian_window, gaussian_sigma);
      cvAddWeighted(sigma2_sq, 1, mu2_sq, -1, 0, sigma2_sq);
      //cvSub(sigma2_sq, mu2_sq, sigma2_sq);
  
      cvSmooth(img1_img2, sigma12, CV_GAUSSIAN, gaussian_window, gaussian_window, gaussian_sigma);
      cvAddWeighted(sigma12, 1, mu1_mu2, -1, 0, sigma12);
      //cvSub(sigma12, mu1_mu2, sigma12);
      
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
 
      // SSIM_INDEX map 
      // ((2*mu1_mu2 + C1).*(2*sigma12 + C2))./((mu1_sq + mu2_sq + C1).*(sigma1_sq + sigma2_sq + C2))
      cvDiv(numerator, denominator, ssim_map, 1);
      
      // Contrast Structure CS_index map
      // (2*sigma12 + C2)./(sigma1_sq + sigma2_sq + C2)
      cvDiv(numerator2, denominator2, cs_map, 1);
  
      // average is taken for both SSIM_map and CS_map 
  
      mssim_value = cvAvg(ssim_map);
      mean_cs_value = cvAvg(cs_map);
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
      for(int i=0; i < 4; i++)
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

    void setK1(double val) { K1 = val; }
    void setK2(double val) { K2 = val; }
    void setGaussian_window(int val) { gaussian_window = val; }
    void setGaussian_sigma(int val) { gaussian_sigma = val; }
    void setLevel(int val) { level = val; }
    void setL(int val) { L = val; }
    void setAlpha(float *val) { alpha = val; }
    void setBeta(float *val) { beta = val; }
    void setGamma(float *val) { gamma = val; }

    // Prints all index maps of all the levels into different xml files
    int print_map()
    {
      if(ms_ssim_map == NULL)
      {
        cout<<"Error>> No Index_map_created.\n";
        return 0;
      }
      char file_name[50];
      // Printing the MS-SSIM_Map
      for(int i=0; i<level; i++)
      {
        sprintf(file_name,"output/img_MS-SSIM_map_level_%d.xml",i);
        cvSave(file_name, ms_ssim_map[i], NULL, "Testing MS-SSIM Index map");
      }
      return 1;
    }

    CvScalar getMSSSIM() { return ms_ssim_value; }
    void getMSSSIM_map(IplImage** output_map) { output_map = ms_ssim_map; }
   
    virtual CvScalar compare(IplImage* source1, IplImage* source2, Colorspace space)
    {
      int x = source1->width, y = source1->height;
      int nChan = source1->nChannels;
      int d = source1->depth;
      
      //Downsampling of the original images
      IplImage* downsampleSrc1, *downsampleSrc2;

      // creating a object of class calcSSIM 
      // setting the initial parameters values
      calcSSIM ssim;
      ssim.setK1(K1);
      ssim.setK2(K2);
      ssim.setGaussian_window(gaussian_window);
      ssim.setGaussian_sigma(gaussian_sigma);
      ssim.setL(L);
      ms_ssim_map = (IplImage**)(malloc(sizeof(IplImage*)*level));

      //cout<<"\nBeta = "<<beta[0]<<" "<<beta[1]<<" "<<beta[2]<<" "<<beta[3]<<"\n";
      
      for(int i=0; i<level; i++)
      {
        //Downsampling the images
        CvSize downs_size = cvSize((int)(x/pow(2,i)), (int)(y/pow(2,i)));
        downsampleSrc1 = cvCreateImage(downs_size, d, nChan);
        downsampleSrc2 = cvCreateImage(downs_size, d, nChan);
        cvResize(source1, downsampleSrc1, CV_INTER_NN);
        cvResize(source2, downsampleSrc2, CV_INTER_NN);

        //cout<<"Values at level="<<i<<" \n";

        ssim.compare(downsampleSrc1,downsampleSrc2,space);

        CvScalar mssim_t = ssim.getMSSIM();
        CvScalar mcs_t = ssim.getMeanCSvalue();
        ms_ssim_map[i] = cvCreateImage(downs_size, IPL_DEPTH_32F, nChan);
        ssim.getSSIM_map(&ms_ssim_map[i]);
        
        //cout<<"Size of MAP at level = "<<i<<"size = "<<ms_ssim_map[i]->width<<" "<<ms_ssim_map[i]->height<<"\n";

        for(int j=0;j<4;j++)
        {
          if(i==0)
            ms_ssim_value.val[j] = pow((mcs_t.val[j]), (double)(beta[i]));
          else if(i==level-1)
            ms_ssim_value.val[j] = (ms_ssim_value.val[j]) * pow((mssim_t.val[j]), (double)(beta[i]));
          else
            ms_ssim_value.val[j] = (ms_ssim_value.val[j]) * pow((mcs_t.val[j]), (double)(beta[i]));
        
        }

      }

      return ms_ssim_value;
    
    }

};


class calcQualityIndex : public SimilarityMetric {

  private:
    int B;
    IplImage* image_quality_map;
    CvScalar image_quality_value;

  public:

    calcQualityIndex()
    {
      B = 8;
      image_quality_map = NULL;
      for(int i=0; i < 4; i++)
        image_quality_value.val[i] = -2;   // Initialize with an out of bound value for image_quality_value [-1.1]
    }
    
    void setB(int val) { B = val; }

    CvScalar getImageQuailty() { return image_quality_value; }
    void getImageQuality_map(IplImage* output_map) 
    {
      output_map = image_quality_map; 
    }

    int print_map()
    {
      
      if(image_quality_map == NULL)
      {
        cout<<"Error>> No Index_map_created.\n";
        return 0;
      }

      int x = image_quality_map->width, y = image_quality_map->height;
      int nChan = image_quality_map->nChannels, d = IPL_DEPTH_8U;
      CvSize size = cvSize(x, y);
      
      IplImage* image_quality_map_t = cvCreateImage(size, d, nChan);

      cvConvert(image_quality_map, image_quality_map_t);

      cvSave("output/imgQI.xml", image_quality_map, NULL, "TESTing Index map");
      cvSaveImage("output/imgQI.bmp", image_quality_map_t);
      return 1;
    
    }
    
    virtual CvScalar compare(IplImage* source1, IplImage* source2, Colorspace space)
    {
      IplImage* src1,* src2;
      int status = colorspaceConversion(source1, source2, &src1, &src2, space);
      
      int x = src1->width, y = src1->height;
      int nChan = src1->nChannels, d = IPL_DEPTH_32F;
      
      CvSize size = cvSize(x, y);
      
      //creating FLOAT type images of src1 and src2 
      IplImage* img1 = cvCreateImage(size, d, nChan);
      IplImage* img2 = cvCreateImage(size, d, nChan);
    
      //Image squares
      IplImage* img1_sq = cvCreateImage(size, d, nChan);
      IplImage* img2_sq = cvCreateImage(size, d, nChan);
      IplImage* img1_img2 = cvCreateImage(size, d, nChan);
    
      cvConvert(src1, img1);
      cvConvert(src2, img2);
      
      //Squaring the images thus created
      cvPow(img1, img1_sq, 2);
      cvPow(img2, img2_sq, 2);
      cvMul(img1, img2, img1_img2, 1);
    
      IplImage* mu1 = cvCreateImage(size, d, nChan);
      IplImage* mu2 = cvCreateImage(size, d, nChan);
      IplImage* mu1_sq = cvCreateImage(size, d, nChan);
      IplImage* mu2_sq = cvCreateImage(size, d, nChan);
      IplImage* mu1_mu2 = cvCreateImage(size, d, nChan);
    
      IplImage* sigma1_sq = cvCreateImage(size, d, nChan);
      IplImage* sigma2_sq = cvCreateImage(size, d, nChan);
      IplImage* sigma12 = cvCreateImage(size, d, nChan);
    
      // creating buffers for numerator and denominator and a temp buffer
      IplImage* numerator1 = cvCreateImage(size, d, nChan);
      IplImage* numerator = cvCreateImage(size, d, nChan);
      IplImage* denominator1 = cvCreateImage(size, d, nChan);
      IplImage* denominator2 = cvCreateImage(size, d, nChan);
      IplImage* denominator = cvCreateImage(size, d, nChan);
      //image_quality map
      //IplImage* image_quality_map = cvCreateImage(size, d, nChan);
      image_quality_map = cvCreateImage(size, d, nChan);
    
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
      cvAddWeighted(sigma1_sq, 1, mu1_sq, -1, 0, sigma1_sq);
    
      cvSmooth(img2_sq, sigma2_sq, CV_BLUR, B, B);
      cvAddWeighted(sigma2_sq, 1, mu2_sq, -1, 0, sigma2_sq);
    
      cvSmooth(img1_img2, sigma12, CV_BLUR, B, B);
      cvAddWeighted(sigma12, 1, mu1_mu2, -1, 0, sigma12);
      
      // FORMULA to calculate Image Quality Index
    
      // (4*sigma12)
      cvScale(sigma12, numerator1, 4);
    
      // (4*sigma12).*(mu1*mu2)
      cvMul(numerator1, mu1_mu2, numerator, 1);
    
      // (mu1_sq + mu2_sq)
      cvAdd(mu1_sq, mu2_sq, denominator1);
    
      // (sigma1_sq + sigma2_sq)
      cvAdd(sigma1_sq, sigma2_sq, denominator2);
    
      // ((mu1_sq + mu2_sq).*(sigma1_sq + sigma2_sq))
      cvMul(denominator1, denominator2, denominator, 1);
    
      // ((4*sigma12).*(mu1_mu2))./((mu1_sq + mu2_sq).*(sigma1_sq + sigma2_sq))
      cvDiv(numerator, denominator, image_quality_map, 1);
    
      // image_quality_map created in image_quality_map
      // average is taken 
    
      image_quality_value = cvAvg(image_quality_map);
      return image_quality_value;

    }

};
 


int main(int argc, char** argv)
{

  IplImage* src1;
  IplImage* src2;
  src1 = cvLoadImage(argv[1]);
  src2 = cvLoadImage(argv[2]);

  int x = src1->width, y = src1->height;
  int nChan = src1->nChannels;
  //Converting into Gray images
  IplImage* gray1 = cvCreateImage(cvSize(x,y), IPL_DEPTH_8U, 1);
  IplImage* gray2 = cvCreateImage(cvSize(x,y), IPL_DEPTH_8U, 1);
  
  //Creating a blurred image of the input image for testing
  IplImage* src3 = cvCreateImage(cvSize(x,y), IPL_DEPTH_8U, nChan);
  cvSmooth(src1, src3, CV_GAUSSIAN, 11 , 11, 2);
  cvSaveImage("test_images/lena_blur.bmp",src3);
  
  cvCvtColor(src1, gray1, CV_RGB2GRAY); //color images are BGR!
  cvCvtColor(src3, gray2, CV_RGB2GRAY); //color images are BGR!
  
  cout<<"Similarity Metrics \n\n";
  //Calculating the Mean Squared Error
  cout<<"------------------------------------------------------------------------------------\n";
  cout<<"inp nChan :\t space :\t val0 :\t val1 :\t val2 :\t val3 \n";
  cout<<"------------------------------------------------------------------------------------\n";
  
  CoreFunctions F;
  //calcSSIM M1;
  //calcMSE M1;
  
  
  CvScalar tM;
  SimilarityMetric * sM;

  calcMSE M;
  sM=&M;
  cout<<"\nMSE\n";
  tM= sM->compare(gray1,gray2,GRAYSCALE);
  F.printCvScalar(tM,GRAYSCALE,1);
  tM= sM->compare(gray1,gray2,RGB);
  F.printCvScalar(tM,RGB,1);
  tM= sM->compare(gray1,gray2,YCbCr);
  F.printCvScalar(tM,YCbCr,1);
  tM= sM->compare(src1,src3,GRAYSCALE);
  F.printCvScalar(tM,GRAYSCALE,3);
  tM= sM->compare(src1,src3,RGB);
  F.printCvScalar(tM,RGB,3);
  tM= sM->compare(src1,src3,YCbCr);
  F.printCvScalar(tM,YCbCr,3);
  cout<<"...................................................................................\n";

  calcPSNR P;
  sM=&P;
  cout<<"\nPSNR\n";
  tM= sM->compare(gray1,gray2,GRAYSCALE);
  F.printCvScalar(tM,GRAYSCALE,1);
  tM= sM->compare(gray1,gray2,RGB);
  F.printCvScalar(tM,RGB,1);
  tM= sM->compare(gray1,gray2,YCbCr);
  F.printCvScalar(tM,YCbCr,1);
  tM= sM->compare(src1,src3,GRAYSCALE);
  F.printCvScalar(tM,GRAYSCALE,3);
  tM= sM->compare(src1,src3,RGB);
  F.printCvScalar(tM,RGB,3);
  tM= sM->compare(src1,src3,YCbCr);
  F.printCvScalar(tM,YCbCr,3);
  cout<<"...................................................................................\n";

  calcSSIM S;
  sM=&S;
  cout<<"\nSSIM\n";
  tM= sM->compare(gray1,gray2,GRAYSCALE);
  F.printCvScalar(tM,GRAYSCALE,1);
  tM= sM->compare(gray1,gray2,RGB);
  F.printCvScalar(tM,RGB,1);
  tM= sM->compare(gray1,gray2,YCbCr);
  F.printCvScalar(tM,YCbCr,1);
  tM= sM->compare(src1,src3,GRAYSCALE);
  F.printCvScalar(tM,GRAYSCALE,3);
  tM= sM->compare(src1,src3,RGB);
  F.printCvScalar(tM,RGB,3);
  S.print_map(); // Prints the RGB SSIM Index_Map to an XMLFile
  tM= sM->compare(src1,src3,YCbCr);
  F.printCvScalar(tM,YCbCr,3);
  cout<<"...................................................................................\n";

  
  calcMSSSIM MS;
  sM=&MS;
  cout<<"\nMS-SSIM\n";
  tM= sM->compare(gray1,gray2,GRAYSCALE);
  F.printCvScalar(tM,GRAYSCALE,1);
  tM= sM->compare(gray1,gray2,RGB);
  F.printCvScalar(tM,RGB,1);
  tM= sM->compare(gray1,gray2,YCbCr);
  F.printCvScalar(tM,YCbCr,1);
  tM= sM->compare(src1,src3,GRAYSCALE);
  F.printCvScalar(tM,GRAYSCALE,3);
  tM= sM->compare(src1,src3,RGB);
  F.printCvScalar(tM,RGB,3);
  tM= sM->compare(src1,src3,YCbCr);
  MS.print_map();
  F.printCvScalar(tM,YCbCr,3);
  cout<<"...................................................................................\n";


  calcQualityIndex Q;
  sM = &Q;
  cout<<"\nQuality Index\n";
  tM= sM->compare(gray1,gray2,GRAYSCALE);
  F.printCvScalar(tM,GRAYSCALE,1);
  tM= sM->compare(gray1,gray2,RGB);
  F.printCvScalar(tM,RGB,1);
  tM= sM->compare(gray1,gray2,YCbCr);
  F.printCvScalar(tM,YCbCr,1);
  tM= sM->compare(src1,src3,GRAYSCALE);
  F.printCvScalar(tM,GRAYSCALE,3);
  tM= sM->compare(src1,src3,RGB);
  F.printCvScalar(tM,RGB,3);
  Q.print_map(); // Prints the RGB SSIM Index_Map to an XMLFile
  tM= sM->compare(src1,src3,YCbCr);
  F.printCvScalar(tM,YCbCr,3);
  cout<<"...................................................................................\n";


}



