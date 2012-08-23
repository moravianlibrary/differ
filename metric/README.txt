metric is a Multi-threaded Image Similarity Metric Library used to 
compare two images. It is a part of project DIFFER as a part of Google Summer 
of Code (GSoC-2012). It is implemented using C++ with openCV and 
openCL support. It implements the following metric - Mean Square Error(MSE),
Peak Signal to Noise Ratio(PSNR), SSIM, MS-SSIM and Universal Image Quality Index. 
This document explains the use of "metric".

Table of Contents
=================

- Quick Start
- Installation
- `metric' Usage
- Output format
- Examples
- Additional Information

Quick Start
===========

If you are new to Image Similarity metrics. Please go through the following links -
- https://ece.uwaterloo.ca/~z70wang/publications/ssim.html
- https://ece.uwaterloo.ca/~z70wang/publications/msssim.pdf
- Z. Wang, A.C. Bovik, "A Universal Image Quality Index", IEEE Signal Processing Letters, vol.9, no.3, pp.81-84, Mar 2002.

Installation and Data Format
============================

Dependencies- openCV, openCL
On Unix systems, type `make' to build the `metric' program. 
Run "metric" without arguments to see its usage.

`metric' Usage
=================

Usage: metric [OPTIONS] [arguments] ...

Options with Mandatory arguments.
  --image1                  input image 1 name 
  --image2                  input image 2 name
  --out                     output format (.xml, [Default stdout])
  --algorithm               algorithm to use 
                            mse - Mean Square Error
                            psnr - Peak Signal to Noise Ratio 
                            ssim - Structural Similarity Index Metric
                            msssim - Multi-scale Structural Similarity Index Metric
                            iqi - Image Quality Index 
                            all - All of the above metrics
  --colorspace              colorspace 
                            0 - GRAYSCALE
                            1 - RGB
                            2 - YCbCr
  --L                       L value for PSNR 
  --K1                      K1 value for SSIM 
  --K2                      K2 value for SSIM 
  --gaussian_window_size    Gaussian window value for SSIM 
  --sigma                   Gaussian sigma value 
  --level                   No. of levels MSSSIM 
  --alpha                   Alpha - comma separated vector (size of level)  
  --beta                    Beta  - comma separated vector (size of level)  
  --gamma                   Gamma - comma separated vector (size of level)  
                            For example - --alpha 0.3,0.5,0.1,0.1 (for level = 4) 
  --B                       B is Block size value for smoothing in IQI 
Options with no arguments. 
  --opencl                  Use OpenCL 
  --index_map               Print Index_map to xml file 
  --help                    Displays help menu 

Output format
=============

* Each line of output displays -
Algorithm : valueR, valueG, valueB
* In case of error, appropriate message is printed on stdout or in xml format. 

Examples
========

Display help
> ./metric

Examples without openCL
> ./metric --image1 lena.bmp --image2 lena_blur.bmp --algorithm mse 
> ./metric --image1 lena.bmp --image2 lena_blur.bmp --algorithm psnr
> ./metric --image1 lena.bmp --image2 lena_blur.bmp --algorithm ssim 
> ./metric --image1 lena.bmp --image2 lena_blur.bmp --algorithm msssim 

Examples With openCL
> ./metric --image1 lena.bmp --image2 lena_blur.bmp --algorithm mse --opencl
> ./metric --image1 lena.bmp --image2 lena_blur.bmp --algorithm psnr --opencl

Executing all algorithms
> ./metric --image1 lena.bmp --image2 lena_blur.bmp --algorithm all

Using options colorspace
> ./metric --image1 lena.bmp --image2 lena_blur.bmp --algorithm all --colorspace 1


Additional information
======================

Acknowledgments:
This work was supported as a part of Google Summer of code (GSoC-2012)

Mentor:
Vaclav Rosecky (xrosecky@gmail.com)

Developer:
Dushyant Goyal (goyal1dushyant@gmail.com)
