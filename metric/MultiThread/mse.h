#ifndef _mse_h
#define _mse_h

// Mean Square Error (MSE) class 
// inheriting from Similarity class 
class calcMSE : public SimilarityMetric {

  private:
    CvScalar mse;

  public:

    calcMSE();
    
    // get functions
    CvScalar getMSE() { return mse; }

    // calculates mse using openCV functions
    virtual CvScalar compare(IplImage *source1, IplImage *source2, Colorspace space);

};

#endif
