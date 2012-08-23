#ifndef _iqi_h
#define _iqi_h

// Image Quality Index class
// inheriting SimilarityMetric
class calcQualityIndex : public SimilarityMetric {

  private:
    int B; // block size value for filtering
    IplImage *image_quality_map; // index_map
    CvScalar image_quality_value; // image_quality_index value

  public:

    calcQualityIndex();
    
    ~calcQualityIndex();

    // get and setfunctions    
    void setB(int val) { B = val; }

    CvScalar getImageQuailty() { return image_quality_value; }
    IplImage* getImageQuality_map() { return image_quality_map; }

    void releaseImageQuality_map();

    // prints the index_map
    int print_map();
    
    // returns the iqi_value using openCV functions
    virtual CvScalar compare(IplImage *source1, IplImage *source2, Colorspace space);

};

#endif
