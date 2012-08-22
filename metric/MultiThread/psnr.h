#ifndef _psnr_h
#define _psnr_h

// PSNR class
// inheriting from SimilarityMetric class
class calcPSNR : public SimilarityMetric {

  private:
    int L;
    CvScalar mse;
    CvScalar PSNR;
  
  public:
   
    calcPSNR();

    // get and set functions
    void setL(int value) { L = value; }
    void setMSE(CvScalar value) { mse = value; }
    CvScalar getPSNR() { return PSNR; }

    // calculates PSNR using OpenCV functions
    virtual CvScalar compare(IplImage *source1, IplImage *source2, Colorspace space);

};

#endif
