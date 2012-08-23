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

// Including core.h
#include "core.h"

// Including the Similarity Metric Header files
#include "similarityMetric.h"
#include "mse.h"
#include "psnr.h"
#include "ssim.h"
#include "msssim.h"
#include "iqi.h"

#include "host_program_opencl.h"
#include "mse_opencl.h"
#include "ssim_opencl.h"
#include "msssim_opencl.h"
#include "iqi_opencl.h"


//#ifndef DEBUG
//#define DEBUG
//#endif
/*
#ifndef DEBUG
#define DEBUG
#endif
*/

int main (int argc, char **argv) {

  struct timeval start_time;
  struct timeval end_time;
  double start_dtime, end_dtime, diff;
  gettimeofday(&start_time,NULL);
 
  // Reading Images
  IplImage *src1;
  IplImage *src2;

  // Result will be stored in res
  CvScalar res;
  // Creating Objects of Metrics - Normal
  calcMSE mse;
  calcSSIM ssim;
  calcPSNR psnr;
  calcMSSSIM msssim;
  calcQualityIndex iqi;

  // Printing device Information
  host_program_openCl H;
  H.print_Device_Info();

  // Creating Objects of Metrics - OpenCl
  MSE_openCl M;
  SSIM_openCl S;
  MS_SSIM_openCl MS;
  ImageQuI_openCl I;

  // Initializing the OpenCl Objects - compile kernels
  M.Init();
  S.Init();
  MS.Init();
  I.Init();
  #ifdef DEBUG
  cout<<"Finished Initialization\n";
  #endif
 

  // Creating ouput file storage
  CvFileStorage* fs;
  char output_file[50];
  int err;
  int out_status = 0;
  fs = NULL;
  if (out_status == 1)
    fs = cvOpenFileStorage(output_file, 0, CV_STORAGE_WRITE);
  char error[100];

  // Setting up the options
  int c;
  int algo = 0;
  Colorspace space;
  space = GRAYSCALE; // default color space
  bool opencl= false; // default no opencl
  char img_name1[50], img_name2[50];
  int image1_status = 0, image2_status = 0;
  int opt_mse = 1, opt_psnr = 2, opt_ssim = 4, opt_msssim = 8, opt_iqi = 16;
  static struct option long_options[] = {
      {"algorithm", 1, 0, 'm'},
      {"colorspace", 1, 0, 'c'},
      {"opencl", 0, 0, 'p'},
      {"L", 1, 0, 'L'},
      {"K1", 1, 0, '1'},
      {"K2", 1, 0, '2'},
      {"gaussian_window_size", 1, 0, 'w'},
      {"sigma", 1, 0, 's'},
      {"level", 1, 0, 'l'},
      {"alpha", 1, 0, 'a'},
      {"beta", 1, 0, 'b'},
      {"gamma", 1, 0, 'g'},
      {"B", 1, 0, 'B'},
      {"out", 1, 0, 'o'},
      {"image1", 1, 0, 1},
      {"image2", 1, 0, 2},
      {"help", 0, 0, 'h'},
      {"index_map", 0, 0, 'i'}, 
      {NULL, 0, NULL, 0}
  };
  int option_index = 0;
  int index_map = 0;
  int ind = 0;
  char * cut;


  if (argc<7) {
    cout<<"Error : Not enough input arguments\n";
    print_help_menu();
    exit(0);
  }

  while ((c = getopt_long(argc, argv, "m:pc:L:1:2:s:l:a:b:g:B:o:hi", long_options, &option_index)) != -1) {
      
    int this_option_optind = optind ? optind : 1;
    switch (c) {
      
      case 'm':
        char algorithm[20];
        sscanf(optarg, "%s", algorithm);
          #ifdef DEBUG
          printf("Algorithm - %s \n", algorithm);
          #endif
          if (strcmp(algorithm,"mse") == 0)
              algo = algo | opt_mse;
          if (strcmp(algorithm,"psnr") == 0)
              algo = algo | opt_psnr;
          if (strcmp(algorithm,"ssim") == 0)
              algo = algo | opt_ssim;
          if (strcmp(algorithm,"msssim") == 0)
              algo = algo | opt_msssim;
          if (strcmp(algorithm,"iqi") == 0)
              algo = algo | opt_iqi;
          if (strcmp(algorithm,"all") == 0)
              algo = opt_mse | opt_psnr | opt_ssim | opt_msssim | opt_iqi;
          if (!algo) {
              sprintf(error, "%s%s", "Invalid algorithm\n", "Possible arguments are - mse, psnr, ssim, msssim, iqi, all");
              printError(fs, error, out_status);
              exit(0);
          }
          break;
      
      case 'c':
          int color_code;
          #ifdef DEBUG
          cout<<"Option colorspace\n";
          #endif
          sscanf(optarg, "%d", &color_code);
          switch (color_code) {
            case 0:
              space = GRAYSCALE;
              break;
            case 1:
              space = RGB;
              break;
            case 2:
              space = YCbCr;
              break;
            default:
              space = GRAYSCALE;
              break;
          }
          #ifdef DEBUG
          cout<<"Space = "<<space<<"\n";
          #endif
          break;

      case 'p':
          opencl = true;
          #ifdef DEBUG
          cout<<"Using OpenCL\n";
          #endif
          break;
      
      case 'o':
          sscanf(optarg, "%s", output_file );
          out_status = 1;
          #ifdef DEBUG
          printf("output_file - %s \n", output_file);
          #endif
          break;

      case 1:
          sscanf(optarg, "%s", img_name1);
          #ifdef DEBUG
    		  printf("Image1 : %s\n", img_name1);
          #endif
          image1_status = 1;
          break;
      
      case 2:
          sscanf(optarg, "%s", img_name2);
          #ifdef DEBUG
    	  	printf("Image2 : %s\n", img_name2);
          #endif
          image2_status = 1;
          break;
      
      case 'L':
          int L;
          sscanf(optarg, "%d", &L);
          #ifdef DEBUG
          printf("Setting L value = %d\n", L);
          #endif
          psnr.setL(L);
          ssim.setL(L);
          msssim.setL(L);
          M.setL(L);
          S.setL(L);
          MS.setL(L);
          break;
      
      case '1':
          double K1;
          sscanf(optarg, "%lf", &K1);
          #ifdef DEBUG
          printf("Setting K1 value = %lf\n", K1);
          #endif
          ssim.setK1(K1);
          msssim.setK1(K1);
          S.setK1(K1);
          MS.setK1(K1);
          break;

      case '2':
          double K2;
          sscanf(optarg, "%lf", &K2);
          #ifdef DEBUG
          printf("Setting K2 value = %lf\n", K2);
          #endif
          ssim.setK2(K2);
          msssim.setK2(K2);
          S.setK2(K2);
          MS.setK2(K2);
          break;
      
      case 'w':
          int w;
          sscanf(optarg, "%d", &w);
          #ifdef DEBUG
          printf("Setting gaussian window = %d\n", w);
          #endif
          if ( w%2 == 0)
            w++;
          ssim.setGaussian_window(w);
          msssim.setGaussian_window(w);
          S.setGaussian_window(w);
          MS.setGaussian_window(w);
          break;
      
      case 's':
          double sigma;
          sscanf(optarg, "%lf", &sigma);
          #ifdef DEBUG
          printf("Setting gaussian sigma = %lf\n", sigma);
          #endif
          ssim.setGaussian_sigma(sigma);
          msssim.setGaussian_sigma(sigma);
          S.setGaussian_sigma(sigma);
          MS.setGaussian_sigma(sigma);
          break;
      
      case 'l':
          int level;
          sscanf(optarg, "%d", &level);
          #ifdef DEBUG
          printf("Setting level = %d\n", level);
          #endif
          msssim.setLevel(level);
          MS.setLevel(level);
          break;
      
      case 'a':
          char alpha_t[100];
          float alpha[50];
          ind = 0;
          sscanf(optarg, "%s", alpha_t);
          cut = strtok(alpha_t, ",");
          while (cut != NULL) {
            sscanf(cut, "%f", &alpha[ind]);
            cut = strtok(NULL, ",");
            ind++;
          }
          #ifdef DEBUG
          printf("Setting alpha = \n");
          #endif
          msssim.setAlpha(alpha);
          MS.setAlpha(alpha);
          break;
      
      case 'b':
          char beta_t[100];
          float beta[50];
          ind =0;
          sscanf(optarg, "%s", beta_t);
          cut = strtok(beta_t, ",");
          while (cut != NULL) {
            sscanf(cut, "%f", &beta[ind]);
            cut = strtok(NULL, ",");
            ind++;
          }
          #ifdef DEBUG
          printf("Setting beta = \n");
          #endif
          msssim.setBeta(beta);
          MS.setBeta(beta);
          break;
 
      case 'g':
          char gamma_t[100];
          float gamma[50];
          ind =0;
          sscanf(optarg, "%s", gamma_t);
          cut = strtok(gamma_t, ",");
          while (cut !=NULL) {
            sscanf(cut, "%f", &gamma[ind]);
            cut = strtok(NULL, ",");
            ind++;
          }
          #ifdef DEBUG
          printf("Setting gamma = \n");
          #endif
          msssim.setGamma(gamma);
          MS.setGamma(gamma);
          break;

      case 'B':
          int B;
          sscanf(optarg, "%d", &B);
          #ifdef DEBUG
          printf("Setting B = %d\n", B);
          #endif
          if (B%2==1)
            B++;
          iqi.setB(B);
          I.setB(B);
          break;
      
      case 'i':
          index_map =1; 
          break;
      
      case 'h':
          print_help_menu();
          break;

      case '?':
          break;
  
      default:
          print_help_menu();
          break;

    }
  }
 
  // Finished with get_opt_long 
  #ifdef DEBUG
  cout<<"Finished with get_opt_long\n";
  cout<<"Algo = "<<algo<<"\n";
  #endif
 
  if (out_status == 1)
    fs = cvOpenFileStorage(output_file, 0, CV_STORAGE_WRITE);

  if (!image1_status || !image2_status) {
    sprintf(error, "%s", "No Input image Arguments");
    printError(fs, error, out_status);
    exit(0);
  }

  // Loading the source images in src1 and src2
  src1 = cvLoadImage(img_name1);
  if (!src1) {
    sprintf(error, "%s: %s", "Could not load image file", img_name1);
    printError(fs, error, out_status);
    exit(0);
  }
  src2 = cvLoadImage(img_name2);
  if (!src2) {
    sprintf(error, "%s: %s", "Could not load image file", img_name2);
    printError(fs, error, out_status);
    exit(0);
  }

  if ( (src1->width != src2->width) || (src1->height != src2->height) || (src1->nChannels != src2->nChannels) ) {
    printError(fs, "Image Dimensions mis-match", out_status);
    if (fs != NULL)
      cvReleaseFileStorage( &fs);
    exit(0);
  }

  if (algo!=0)
  {
    if ((algo & opt_mse) != 0)
      if (opencl == false) {
        res = mse.compare(src1, src2, space);
        printCvScalar(fs, res, "MSE", out_status);
      } 
      else {
        res = M.compare(src1, src2, space);
        printCvScalar(fs, res, "MSE_openCl", out_status);
      }
    
    if ((algo & opt_psnr) != 0)
      if (opencl == false) {
        res = psnr.compare(src1, src2, space);
        printCvScalar(fs, res, "PSNR", out_status);
      }
      else {
        res = M.compare(src1, src2, space);
        res = M.getPSNR();
        printCvScalar(fs, res, "PSNR_openCl", out_status);
      }
    
    if ((algo & opt_ssim) != 0)
      if (opencl == false) {
        res = ssim.compare(src1, src2, space);
        printCvScalar(fs, res, "SSIM", out_status);
        if (index_map == 1) {
          err = ssim.print_map();
          if (err == 0)
            printError(fs, "Error Printing Index_map SSIM", out_status);
        }
      }
      else {
        res = S.compare(src1, src2, space);
        printCvScalar(fs, res, "SSIM_opencl", out_status);
        if (index_map == 1) {
          err = S.print_map();
          if (err == 0)
            printError(fs, "Error Printing Index_map SSIM OpenCl", out_status);
        }
      }
    
    if ((algo & opt_msssim) != 0)
      if (opencl == false) {
        res = msssim.compare(src1, src2, space);
        printCvScalar(fs, res, "MSSSIM", out_status);
        if (index_map == 1) {
          err = msssim.print_map();
          if (err == 0)
            printError(fs, "Error Printing Index_map MS-SSIM", out_status);
        }
      }
      else {
        res = MS.compare(src1, src2, space);
        printCvScalar(fs, res, "MSSSIM_opencl", out_status);
        if (index_map == 1) {
          err = MS.print_map();
          if (err == 0)
            printError(fs, "Error Printing Index_map MS-SSIM openCl", out_status);
        }
      }
    
    if ((algo & opt_iqi) != 0)
      if (opencl == false) {
        res = iqi.compare(src1, src2, space);
        printCvScalar(fs, res, "IQI", out_status);
        if (index_map == 1) {
          err = iqi.print_map();
          if (err == 0)
            printError(fs, "Error Printing Index_map IQI", out_status);
        }
      }
      else {
        res = I.compare(src1, src2, space);
        printCvScalar(fs, res, "IQI_opencl", out_status);
        if (index_map == 1) {
          err = I.print_map();
          if (err == 0)
            printError(fs, "Error Printing Index_map IQI openCl", out_status);
        }
      }
  }

  // Cleaning up OpenCL Hosts
  M.clean_up_host();
  S.clean_up_host();
  MS.clean_up_host();
  I.clean_up_host();
  
  //Release images
  cvReleaseImage(&src1);
  cvReleaseImage(&src2);

  // Releasing storage
  if (fs != NULL)
    cvReleaseFileStorage( &fs);
  
  //Will be used to calculate time
  /* 
  gettimeofday(&end_time,NULL);
  start_dtime=(double)start_time.tv_sec+(double)start_time.tv_usec/1000000.0;
  end_dtime=(double)end_time.tv_sec+(double)end_time.tv_usec/1000000.0; 
  diff=end_dtime-start_dtime;
  printf("Reading Buffer - %ld %ld %f %ld %ld %f %f\n",start_time.tv_sec,start_time.tv_usec,start_dtime,end_time.tv_sec,end_time.tv_usec,end_dtime,diff); 
  */
 
  exit(0);

}
