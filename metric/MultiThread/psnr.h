#ifndef _psnr_h
#define _psnr_h

class calcPSNR : public SimilarityMetric {

  private:
    int L;
    CvScalar mse;
    CvScalar PSNR;
  
  public:
   
    calcPSNR();

    void setL(int value) { L = value; }
    void setMSE(CvScalar value) { mse = value; }
    CvScalar getPSNR() { return PSNR; }

    virtual CvScalar compare(IplImage *source1, IplImage *source2, Colorspace space);

};

#endif
