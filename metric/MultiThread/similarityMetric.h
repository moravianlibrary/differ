#ifndef _similarityMetric_h
#define _similarityMetric_h

// Image SimilarityMetric Base Class
class SimilarityMetric {

  public:
    virtual CvScalar compare(IplImage *src1, IplImage *src2, Colorspace space)
    {
    
    }

    IplImage* colorspaceConversion(IplImage *source1, Colorspace space);

};


#endif
