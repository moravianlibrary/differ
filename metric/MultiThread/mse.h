#ifndef _mse_h
#define _mse_h

class calcMSE : public SimilarityMetric {

  private:
    CvScalar mse;

  public:

    calcMSE();

    CvScalar getMSE() { return mse; }

    virtual CvScalar compare(IplImage *source1, IplImage *source2, Colorspace space);

};

#endif
