#ifndef _core_h
#define _core_h
// Basic functions implemented here
// core.h included by all source files

// Dependencies
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

// Colorspace variable
enum Colorspace {
  GRAYSCALE,
  RGB,
  YCbCr
};

// function to create a gaussian kernel given kernel_size and gaussian_sigma
Mat get_gaussian_filter(int filter_size, int sigma);

// function to calculate and print time
void print_time(clock_t start, int TIMES, const char *s);

// function to print the help menu
void print_help_menu();

// function to write output in an XML format
void writeXML( CvFileStorage *fs, CvScalar result, const char * algo );

// function to write output on stdout or xml file depending upon out_status
void printCvScalar(CvFileStorage *fs, CvScalar value, const char *comment, int out_status);

// function to print error on stdout or xml depending upon out_status
void printError(CvFileStorage *fs, const char *comment, int out_status);

#endif
