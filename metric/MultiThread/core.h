#ifndef _core_h
#define _core_h

enum Colorspace {
  GRAYSCALE,
  RGB,
  YCbCr
};

Mat get_gaussian_filter(int filter_size, int sigma);
void print_time(clock_t start, int TIMES, const char *s);
void print_help_menu();
void writeXML( CvFileStorage *fs, CvScalar result, const char * algo );
void printCvScalar(CvFileStorage *fs, CvScalar value, const char *comment, int out_status);
void printError(CvFileStorage *fs, const char *comment, int out_status);

#endif
