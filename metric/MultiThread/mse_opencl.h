#ifndef _SSIM_openCl_h
#define _SSIM_openCl_h

class MSE_openCl : public host_program_openCl, public SimilarityMetric { 

  private:
    int L;
    CvScalar mse;
    CvScalar psnr;

    char *source_str;
    size_t source_size_mse;
    cl_kernel kernel_mse;  // Create the OpenCL kernels
    const char *mse_cl;

  public:
    MSE_openCl();

    void setL(int value) { L = value; }
    CvScalar getMSE() { return mse; }
    CvScalar getPSNR() { return psnr; }

    void Init();
   
    void execute_mse (float *A, float *B, float *C, int LIST_SIZE, int LOCAL_SIZE);
    
    // Clean up
    void clean_up_host();

    virtual CvScalar compare(IplImage *source1, IplImage *source2, Colorspace space);

};

#endif
