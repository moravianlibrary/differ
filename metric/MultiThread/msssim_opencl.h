#ifndef _MS_SSIM_openCl_h
#define _MS_SSIM_openCl_h

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
 
    char *source_str_ms_ssim1;
    char *source_str_ms_ssim2;
    size_t source_size_ms_ssim1;
    size_t source_size_ms_ssim2;
    // Create the OpenCL kernels
    cl_kernel kernel_ms_ssim1, kernel_ms_ssim2;
    const char *ms_ssim1;
    const char *ms_ssim2;
 
  public:

    MS_SSIM_openCl();
    ~MS_SSIM_openCl();

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

    void releaseMSSSIM_map();

    // Prints all index maps of all the levels into different xml files
    int print_map();

    void Init();
   
    void execute_ssim_temp (float *src1, float *src2, float *filter, float *ms_ssim, float *cs_map, int LIST_SIZE, int LOCAL_SIZE, int x, int y, int nChan, int filter_size, float C1, float C2);
    
    // Clean up
    void clean_up_host();
    
    CvScalar execute_ms_ssim (IplImage *src1, IplImage *src2, float *filter, int LIST_SIZE, int LOCAL_SIZE, int x, int y, int nChan, int filter_size, float C1, float C2, int level);

    virtual CvScalar compare(IplImage *source1, IplImage *source2, Colorspace space);

};

#endif
