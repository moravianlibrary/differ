#ifndef _mse_opencl_h
#define _mse_opencl_h

// MSE class for OpenCL
// inheriting from host_program_openCl and SimilarityMetric
class MSE_openCl : public host_program_openCl, public SimilarityMetric { 

  private:
    int L;
    CvScalar mse;
    CvScalar psnr;

    // source variables to store kernel
    char *source_str;
    size_t source_size_mse;
    cl_kernel kernel_mse;  // Create the OpenCL kernels
    const char *mse_cl;

  public:

    // constructor to initialize values to NULL
    MSE_openCl();

    // get and set functions
    void setL(int value) { L = value; }
    CvScalar getMSE() { return mse; }
    CvScalar getPSNR() { return psnr; }

    // initialization i.e. reading, compiling and setting up kernel
    void Init();
    
    // executing the kernel and returning the result in float *ssim
    void execute_mse (float *A, float *B, float *C, int LIST_SIZE, int LOCAL_SIZE);
    
    // Clean up
    void clean_up_host();

    // calling execute_mse and returning the mean value in CvScalar
    virtual CvScalar compare(IplImage *source1, IplImage *source2, Colorspace space);

};

#endif
