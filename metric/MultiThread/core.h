#ifndef _core_h
#define _core_h

//Dependencies
#include <stdlib.h>
#include <sstream>
#include <ctime>
#include <cv.h>
#include <highgui.h>
#include <stdio.h>
#include <iostream>
#include <getopt.h>    // for getopt_long
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <CL/cl.h>

#define MAX_SOURCE_SIZE (0x100000)
using namespace cv;
using namespace std;

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
