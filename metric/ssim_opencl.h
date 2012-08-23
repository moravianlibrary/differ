#ifndef _SSIM_opencl_h
#define _SSIM_opencl_h

// SSIM class for OpenCL
// inheriting from host_program_openCl and SimilarityMetric
class SSIM_openCl : public host_program_openCl, public SimilarityMetric { 

  private:
    double K1;
    double K2;
    int gaussian_window;
    double gaussian_sigma;
    int L;
    IplImage *ssim_map;
    CvScalar mssim_value;

    // source variables to store kernels
    char *source_str_ssim1;
    char *source_str_ssim2;
    size_t source_size_ssim1;
    size_t source_size_ssim2;
    // Create the OpenCL kernels
    cl_kernel kernel_ssim1, kernel_ssim2;
    const char *ssim1;
    const char *ssim2;

  public:

    // constructor to set values to NULL
    SSIM_openCl();
    ~SSIM_openCl();
    
    // get and set functions
    void setK1(double val) { K1 = val; }
    void setK2(double val) { K2 = val; }
    void setGaussian_window(int val) { gaussian_window = val; }
    void setGaussian_sigma(double val) { gaussian_sigma = val; }
    void setL(int val) { L = val; }

    CvScalar getMSSIM() { return mssim_value; }
    IplImage* getSSIM_map() { return ssim_map; }
   
    // prints index map 
    int print_map();

    // initialization i.e. reading, compiling and setting up kernel
    void Init();
    
    // executing the kernel and returning the result in float *ssim
    void execute_ssim (float *src1, float *src2, float *filter, float *ssim, int LIST_SIZE, int LOCAL_SIZE, int x, int y, int nChan, int filter_size, float C1, float C2);
    // Clean up
    void clean_up_host();

    // calls execute_ssim and return the average value
    virtual CvScalar compare(IplImage *source1, IplImage *source2, Colorspace space);

};

#endif
