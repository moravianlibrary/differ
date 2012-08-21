#ifndef _iqi_h
#define _iqi_h

class calcQualityIndex : public SimilarityMetric {

  private:
    int B;
    IplImage *image_quality_map;
    CvScalar image_quality_value;

  public:

    calcQualityIndex();
    
    ~calcQualityIndex();
    
    void setB(int val) { B = val; }

    CvScalar getImageQuailty() { return image_quality_value; }
    IplImage* getImageQuality_map() { return image_quality_map; }

    void releaseImageQuality_map();

    int print_map();
    
    virtual CvScalar compare(IplImage *source1, IplImage *source2, Colorspace space);

};

#endif
