#include "core.h"

// Some primitive and useful functions are implemented here

Mat get_gaussian_filter(int filter_size, int sigma) {
  Mat filter1D;
  filter1D = getGaussianKernel(filter_size, sigma, CV_32F); // this function creates a 1-D gaussian array
  Mat filter2D = filter1D * filter1D.t(); // converting into 2-D
  return filter2D;
}

void print_time(clock_t start, int TIMES, const char *s) {
  double diff, time;
  diff = ( std::clock() - start ) / (double)CLOCKS_PER_SEC;
  time = diff/TIMES;
  printf("Time of CPU run using %s (averaged for %d runs): %f milliseconds.\n",s,TIMES, time);
  printf("Time of CPU run using %s (total for %d runs): %f milliseconds.\n",s, TIMES, diff);
}

void print_help_menu() {
  printf("\nUsage: metric [OPTIONS] [arguments] ...\n");
  printf("\nOptions with Mandatory arguments.\n");
  printf("  --image1                  input image 1 name \n");
  printf("  --image2                  input image 2 name\n");
  printf("  --out                     output format (.xml, [Default stdout])\n");
  printf("  --algorithm               algorithm to use \n");
  printf("                            mse - Mean Square Error\n");
  printf("                            psnr - Peak Signal to Noise Ratio \n");
  printf("                            ssim - Structural Similarity Index Metric\n");
  printf("                            msssim - Multi-scale Structural Similarity Index Metric\n");
  printf("                            iqi - Image Quality Index \n");
  printf("                            all - All of the above metrics\n");
  printf("  --colorspace              colorspace \n");
  printf("                            0 - GRAYSCALE\n");
  printf("                            1 - RGB\n");
  printf("                            2 - YCbCr\n");
  printf("  --L                       L value for PSNR \n");
  printf("  --K1                      K1 value for SSIM \n");
  printf("  --K2                      K2 value for SSIM \n");
  printf("  --gaussian_window_size    Gaussian window value for SSIM \n");
  printf("  --sigma                   Gaussian sigma value \n");
  printf("  --level                   No. of levels MSSSIM \n");
  printf("  --alpha                   Alpha - comma separated vector (size of level)  \n");
  printf("  --beta                    Beta  - comma separated vector (size of level)  \n");
  printf("  --gamma                   Gamma - comma separated vector (size of level)  \n");
  printf("                            For example - --alpha 0.3,0.5,0.1,0.1 (for level = 4) \n");
  printf("  --B                       B is Block size value for smoothing in IQI \n");
  printf("  Options with no arguments. \n");
  printf("  --opencl                  Use OpenCL \n");
  printf("  --index_map               Print Index_map to xml file \n");
  printf("  --help                    Displays help menu \n");
}

void writeXML( CvFileStorage *fs, CvScalar result, const char * algo ) {
  cvWriteString( fs, "Algorithm", algo);
  cvStartWriteStruct( fs, "Values", CV_NODE_SEQ);
  cvWriteReal( fs, 0, 1.0*result.val[0]);
  cvWriteReal( fs, 0, 1.0*result.val[1]);
  cvWriteReal( fs, 0, 1.0*result.val[2]);
  cvEndWriteStruct(fs);
}

void printCvScalar(CvFileStorage *fs, CvScalar value, const char *comment, int out_status)
{
  if (out_status == 1)
    writeXML(fs, value, comment);
  else
    cout<<comment<<" : "<<value.val[0]<<" , "<<value.val[1]<<" , "<<value.val[2]<<"\n";
}

void printError(CvFileStorage *fs, const char *comment, int out_status)
{
  if (out_status == 1)
    cvWriteString( fs, "Error", comment);
  else
    printf("%s \n", comment);
}



