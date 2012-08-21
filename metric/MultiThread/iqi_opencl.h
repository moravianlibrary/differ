#ifndef _ImageQuI_openCl_h
#define _ImageQuI_openCl_h

class ImageQuI_openCl : public host_program_openCl, public SimilarityMetric { 

  private:
    int B;
    IplImage *image_quality_map;
    CvScalar image_quality_value;
 
    char *source_str_iqi1;
    char *source_str_iqi2;
    size_t source_size_iqi1;
    size_t source_size_iqi2;
    // Create the OpenCL kernels
    cl_kernel kernel_iqi1, kernel_iqi2;
    const char *iqi1;
    const char *iqi2;

  public:

    ImageQuI_openCl();
    ~ImageQuI_openCl();
    
    void setB(int val) { B = val; }

    CvScalar getImageQuailty() { return image_quality_value; }
    IplImage* getImageQuality_map() { return image_quality_map; }

    void releaseImageQuality_map();

    int print_map();
    void Init();
    
    void execute_iqi (float *src1, float *src2, float *iqi, int LIST_SIZE, int LOCAL_SIZE, int x, int y, int nChan, int filter_size);
    
    // Clean up
    void clean_up_host();

    virtual CvScalar compare(IplImage *source1, IplImage *source2, Colorspace space);

};

#endif
