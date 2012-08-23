#ifndef _msssim_h
#define _msssim_h

// MS-SSIM class
// inheriting SimilarityMetric
class calcMSSSIM : public SimilarityMetric {

  private:
    double K1;
    double K2;
    int gaussian_window; // window size
    double gaussian_sigma; // gaussian_sigma value using in filtering
    int level; // no. of levels
    int L;
    IplImage** ms_ssim_map;
    CvScalar ms_ssim_value;

    float *alpha;
    float *beta;
    float *gamma;

  public:

    calcMSSSIM();

    ~calcMSSSIM();

    // get and set functions
    void setK1(double val) { K1 = val; }
    void setK2(double val) { K2 = val; }
    void setGaussian_window(int val) { gaussian_window = val; }
    void setGaussian_sigma(double val) { gaussian_sigma = val; }
    void setLevel(int val) { level = val; }
    void setL(int val) { L = val; }
    void setAlpha(float *val) { alpha = val;}
    void setBeta(float *val) { beta = val;}
    void setGamma(float *val) { gamma = val; }

    CvScalar getMSSSIM() { return ms_ssim_value; }
    IplImage** getMSSSIM_map() { return ms_ssim_map; }

    // release MSSSIM_map
    void releaseMSSSIM_map();

    // Prints all index maps of all the levels into different xml files
    int print_map();
  
    // implementation function to calculate MS-SSIM using OpenCV functions
    virtual CvScalar compare(IplImage *source1, IplImage *source2, Colorspace space);

};

#endif
