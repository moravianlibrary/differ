#ifndef _ssim_h
#define _ssim_h

// SSIM class
// inheriting SimilarityMetric
class calcSSIM : public SimilarityMetric {

  private:

    // parameters
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

    calcSSIM();

    /* ~calcSSIM()
    {
      if (ssim_map != NULL)
        cvReleaseImage(&ssim_map);
      if (cs_map != NULL)
        cvReleaseImage(&cs_map);
    }*/

    // get and set functions
    void setK1(double val) { K1 = val; }
    void setK2(double val) { K2 = val; }
    void setGaussian_window(int val) { gaussian_window = val; }
    void setGaussian_sigma(int val) { gaussian_sigma = val; }
    void setL(int val) { L = val; }

    CvScalar getMSSIM() { return mssim_value; }
    CvScalar getMeanCSvalue() { return mean_cs_value; }
    IplImage* getSSIM_map() { return ssim_map; }
    IplImage* getCS_map() { return cs_map; }

    // release SSIM_map
    void releaseSSIM_map();
    
    // release CS_map
    void releaseCS_map();

    // print_map
    int print_map();

    // calculating mean SSIM value using openCV functions
    virtual CvScalar compare(IplImage *source1, IplImage *source2, Colorspace space);

};

#endif


