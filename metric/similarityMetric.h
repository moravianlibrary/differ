#ifndef _similarityMetric_h
#define _similarityMetric_h

// Image SimilarityMetric Base Class
class SimilarityMetric {

  public:

    // virtual functions compare, returns CvScalar value and has separate implementation for each of the algorithms
    virtual CvScalar compare(IplImage *src1, IplImage *src2, Colorspace space)
    {
    
    }

    // function to convert image into different colorspaces based upon the enum Colorspace
    IplImage* colorspaceConversion(IplImage *source1, Colorspace space);

};


#endif
