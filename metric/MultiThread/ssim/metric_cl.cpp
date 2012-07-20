#include <stdlib.h>
#include <sstream>
#include <ctime>
#include <cv.h>
#include <highgui.h>
#include <stdio.h>
#include <iostream>
//#ifndef DEBUG
//#define DEBUG
//#endif
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#define MAX_SOURCE_SIZE (0x100000)
using namespace cv;
using namespace std;

class host_program_openCl {

  public:
      char *source_str;
      size_t source_size;
      // Get platform and device information
      cl_platform_id platform_id;
      cl_device_id device_id;
      cl_uint ret_num_devices;
      cl_uint ret_num_platforms;
      // Create an OpenCL context
      cl_context context;
      // Create a command queue
      cl_command_queue command_queue;
      // Create the OpenCL program
      cl_program program;
      // Create the OpenCL kernels
      cl_kernel kernel_mse, kernel_ssim1, kernel_ssim2, kernel_gaus;

      cl_int ret;
      
    
      void load_kernel (const char * kernel_name) {
        // Load the kernel source code into the array source_str
        FILE *fp;
        printf("%s\n",kernel_name); 
        //fp = fopen("vector_add_kernel.cl", "r");
        fp = fopen(kernel_name, "r");
        if (!fp) {
            fprintf(stderr, "Failed to load kernel.\n");
        }
        source_str = (char*)malloc(MAX_SOURCE_SIZE);
        source_size = fread( source_str, 1, MAX_SOURCE_SIZE, fp);
        fclose( fp );
      }
      
      void setup_gaussian (const char *kernel_function_name) {
        platform_id = NULL;
        device_id = NULL;
        
        // Get platform and device information
        ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
        ret = clGetDeviceIDs( platform_id, CL_DEVICE_TYPE_ALL, 1, &device_id, &ret_num_devices);
       
        // Create an OpenCL context
        context = clCreateContext( NULL, 1, &device_id, NULL, NULL, &ret);
       
        // Create a command queue
        command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
        if( ret != CL_SUCCESS ) 
          printf( "Error : Cannot create command queue.\n" ); 

        // Create a program from the kernel source
        program = clCreateProgramWithSource(context, 1, (const char **)&source_str, (const size_t *)&source_size, &ret);
       
        // Build the program
        ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
       
        // Create the OpenCL kernel
        kernel_gaus = clCreateKernel(program, kernel_function_name, &ret);
     }

      void execute_gauss (float *A, float *B, float *C, int LIST_SIZE, int LOCAL_SIZE, int x, int y, int nChan, int filter_size) {
  
       //global_item_size = LIST_SIZE;
       //local_item_size = LOCAL_SIZE;
       size_t global_item_size[] = {x,y};
       size_t local_item_size[] = {2,2};

       // Create memory buffers on the device for each vector
       cl_mem a_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);
       cl_mem b_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, filter_size*filter_size* sizeof(float), NULL, &ret);
       cl_mem c_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);
     
       if (!a_mem_obj || !b_mem_obj || !c_mem_obj)
            cout<<"Failed to allocate device memory!\n"; 
   
       // Copy the lists A and B to their respective memory buffers
       ret = clEnqueueWriteBuffer(command_queue, a_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(float), A, 0, NULL, NULL);
       if (ret != CL_SUCCESS)
          printf("Error: Failed to write to source array!\n");
       ret = clEnqueueWriteBuffer(command_queue, b_mem_obj, CL_TRUE, 0, filter_size*filter_size* sizeof(float), B, 0, NULL, NULL);
     
       // Set the arguments of the kernel
       ret = clSetKernelArg(kernel_gaus, 0, sizeof(cl_mem), (void *)&a_mem_obj);
       ret = clSetKernelArg(kernel_gaus, 1, sizeof(cl_mem), (void *)&b_mem_obj);
       ret = clSetKernelArg(kernel_gaus, 2, sizeof(cl_mem), (void *)&c_mem_obj);
       ret = clSetKernelArg(kernel_gaus, 3, sizeof(int), &x);
       ret = clSetKernelArg(kernel_gaus, 4, sizeof(int), &y);
       ret = clSetKernelArg(kernel_gaus, 5, sizeof(int), &nChan);
       ret = clSetKernelArg(kernel_gaus, 6, sizeof(int), &filter_size);
       
       // Execute the OpenCL kernel on the list
       //ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);
       ret = clEnqueueNDRangeKernel(command_queue, kernel_gaus, 2, NULL, global_item_size, local_item_size, 0, NULL, NULL);
   
       // Read the memory buffer C on the device to the local variable C
       ret = clEnqueueReadBuffer(command_queue, c_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(float), C, 0, NULL, NULL);
       // Clean up
       ret = clReleaseMemObject(a_mem_obj);
       ret = clReleaseMemObject(b_mem_obj);
       ret = clReleaseMemObject(c_mem_obj);
    }
    
     void setup_ssim1 (const char *kernel_function_name) {
        platform_id = NULL;
        device_id = NULL;
        
        // Get platform and device information
        ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
        ret = clGetDeviceIDs( platform_id, CL_DEVICE_TYPE_ALL, 1, &device_id, &ret_num_devices);
       
        // Create an OpenCL context
        context = clCreateContext( NULL, 1, &device_id, NULL, NULL, &ret);
       
        // Create a command queue
        command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
        if( ret != CL_SUCCESS ) 
          printf( "Error : Cannot create command queue.\n" ); 

        // Create a program from the kernel source
        program = clCreateProgramWithSource(context, 1, (const char **)&source_str, (const size_t *)&source_size, &ret);
       
        // Build the program
        ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
        if (ret !=CL_SUCCESS)
          printf("Error: the Program not built..\n");
       
        // Create the OpenCL kernel
        kernel_ssim1 = clCreateKernel(program, kernel_function_name, &ret);
        if (ret !=CL_SUCCESS)
          printf("Error: the kernel not created\n");
     }
      
     void setup_ssim2 (const char *kernel_function_name) {
        platform_id = NULL;
        device_id = NULL;
        
        // Get platform and device information
        ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
        ret = clGetDeviceIDs( platform_id, CL_DEVICE_TYPE_ALL, 1, &device_id, &ret_num_devices);
       
        // Create an OpenCL context
        context = clCreateContext( NULL, 1, &device_id, NULL, NULL, &ret);
       
        // Create a command queue
        command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
        if( ret != CL_SUCCESS ) 
          printf( "Error : Cannot create command queue.\n" ); 

        // Create a program from the kernel source
        program = clCreateProgramWithSource(context, 1, (const char **)&source_str, (const size_t *)&source_size, &ret);
       
        // Build the program
        ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
        if (ret !=CL_SUCCESS)
          printf("Error: the Program not built..\n");
       
        // Create the OpenCL kernel
        kernel_ssim2 = clCreateKernel(program, kernel_function_name, &ret);
        if (ret !=CL_SUCCESS)
          printf("Error: the kernel not created\n");
     }

    
     void setup_ssim () {
        platform_id = NULL;
        device_id = NULL;
        
        // Get platform and device information
        ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
        ret = clGetDeviceIDs( platform_id, CL_DEVICE_TYPE_ALL, 1, &device_id, &ret_num_devices);
       
        // Create an OpenCL context
        context = clCreateContext( NULL, 1, &device_id, NULL, NULL, &ret);
       
        // Create a command queue
        command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
        if( ret != CL_SUCCESS ) 
          printf( "Error : Cannot create command queue.\n" );

        // Load the kernel source code into the array source_str
        FILE *fp;
        printf("%s\n","ssim_part1.cl"); 
        fp = fopen("ssim_part1.cl", "r");
        if (!fp) {
            fprintf(stderr, "Failed to load kernel.\n");
        }
        source_str = (char*)malloc(MAX_SOURCE_SIZE);
        source_size = fread( source_str, 1, MAX_SOURCE_SIZE, fp);
        fclose( fp );

        // Create a program from the kernel source
        program = clCreateProgramWithSource(context, 1, (const char **)&source_str, (const size_t *)&source_size, &ret);
       
        // Build the program
        ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
        if (ret !=CL_SUCCESS)
          printf("Error: the Program not built..\n");
       
        // Create the OpenCL kernel
        kernel_ssim1 = clCreateKernel(program, "ssim_A", &ret);
        if (ret !=CL_SUCCESS)
          printf("Error: the kernel not created\n");

         printf("%s\n","ssim_part2.cl"); 
        fp = fopen("ssim_part2.cl", "r");
        if (!fp) {
            fprintf(stderr, "Failed to load kernel.\n");
        }
        source_str = (char*)malloc(MAX_SOURCE_SIZE);
        source_size = fread( source_str, 1, MAX_SOURCE_SIZE, fp);
        fclose( fp );

        // Create a program from the kernel source
        program = clCreateProgramWithSource(context, 1, (const char **)&source_str, (const size_t *)&source_size, &ret);
       
        // Build the program
        ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
        if (ret !=CL_SUCCESS)
          printf("Error: the Program not built..\n");
       
        // Create the OpenCL kernel
        kernel_ssim2 = clCreateKernel(program, "ssim_B", &ret);
        if (ret !=CL_SUCCESS)
          printf("Error: the kernel not created\n");


     }

     void execute_ssim (float *src1, float *src2, float *filter, float *ssim, int LIST_SIZE, int LOCAL_SIZE, int x, int y, int nChan, int filter_size, float C1, float C2) {
  
       //global_item_size = LIST_SIZE;
       //local_item_size = LOCAL_SIZE;
       size_t global_item_size[] = {x,y};
       size_t local_item_size[] = {1,1};
        cout<<"Creating hte memory buffers-\n";
       // Create memory buffers on the device for each vector
       cl_mem src1_mem_obj      = clCreateBuffer(context, CL_MEM_READ_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);
       cl_mem src2_mem_obj      = clCreateBuffer(context, CL_MEM_READ_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);
       cl_mem filter_mem_obj    = clCreateBuffer(context, CL_MEM_READ_ONLY, filter_size*filter_size* sizeof(float), NULL, &ret);
       cl_mem img1_sq_mem_obj   = clCreateBuffer(context, CL_MEM_READ_WRITE, LIST_SIZE * sizeof(float), NULL, &ret);
       cl_mem img2_sq_mem_obj   = clCreateBuffer(context, CL_MEM_READ_WRITE, LIST_SIZE * sizeof(float), NULL, &ret);
       cl_mem img1_img2_mem_obj = clCreateBuffer(context, CL_MEM_READ_WRITE, LIST_SIZE * sizeof(float), NULL, &ret);
       cl_mem mu1_mem_obj       = clCreateBuffer(context, CL_MEM_READ_WRITE, LIST_SIZE * sizeof(float), NULL, &ret);
       cl_mem mu2_mem_obj       = clCreateBuffer(context, CL_MEM_READ_WRITE, LIST_SIZE * sizeof(float), NULL, &ret);
       //cl_mem img1_sq_mem_obj   = clCreateBuffer(context, CL_MEM_WRITE_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);
       //cl_mem img2_sq_mem_obj   = clCreateBuffer(context, CL_MEM_WRITE_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);
       //cl_mem img1_img2_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);
       //cl_mem mu1_mem_obj       = clCreateBuffer(context, CL_MEM_WRITE_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);
       //cl_mem mu2_mem_obj       = clCreateBuffer(context, CL_MEM_WRITE_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);
       cl_mem ssim_index_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);
     
       if (!src1_mem_obj || !src2_mem_obj || !filter_mem_obj || !img1_sq_mem_obj || !img2_sq_mem_obj || !img1_img2_mem_obj || 
           !mu1_mem_obj || !mu2_mem_obj || !ssim_index_mem_obj)
           cout<<"Failed to allocate device memory!\n"; 
        
       cout<<"Allocating memory buffers - \n"; 
       // Copy the lists Image Data src1, src2 to their respective memory buffers
       ret = clEnqueueWriteBuffer(command_queue, src1_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(float), src1, 0, NULL, NULL);
       ret = clEnqueueWriteBuffer(command_queue, src2_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(float), src2, 0, NULL, NULL);
       if (ret != CL_SUCCESS)
          printf("Error: Failed to write to source array!\n");
       ret = clEnqueueWriteBuffer(command_queue, filter_mem_obj, CL_TRUE, 0, filter_size*filter_size* sizeof(float), filter, 0, NULL, NULL);
        
       cout<<"Setting arguments of kernel1 ssim- \n"; 
       // Set the arguments of the kernel ssim1
       ret = clSetKernelArg(kernel_ssim1, 0, sizeof(cl_mem), (void *)&src1_mem_obj);
       ret = clSetKernelArg(kernel_ssim1, 1, sizeof(cl_mem), (void *)&src2_mem_obj);
       ret = clSetKernelArg(kernel_ssim1, 2, sizeof(cl_mem), (void *)&filter_mem_obj   );
       ret = clSetKernelArg(kernel_ssim1, 3, sizeof(cl_mem), (void *)&img1_sq_mem_obj  );
       ret = clSetKernelArg(kernel_ssim1, 4, sizeof(cl_mem), (void *)&img2_sq_mem_obj  );
       ret = clSetKernelArg(kernel_ssim1, 5, sizeof(cl_mem), (void *)&img1_img2_mem_obj);
       ret = clSetKernelArg(kernel_ssim1, 6, sizeof(cl_mem), (void *)&mu1_mem_obj);
       ret = clSetKernelArg(kernel_ssim1, 7, sizeof(cl_mem), (void *)&mu2_mem_obj);
       ret = clSetKernelArg(kernel_ssim1, 8, sizeof(int), &x);
       ret = clSetKernelArg(kernel_ssim1, 9, sizeof(int), &y);
       ret = clSetKernelArg(kernel_ssim1, 10, sizeof(int), &nChan);
       ret = clSetKernelArg(kernel_ssim1, 11, sizeof(int), &filter_size);
       
       // Execute the OpenCL kernel on the list
       //ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);
       cout<<"Executing ssim kernel1 - \n";
       ret = clEnqueueNDRangeKernel(command_queue, kernel_ssim1, 2, NULL, global_item_size, local_item_size, 0, NULL, NULL);
       if (ret!=CL_SUCCESS) {
         printf("Error: Kernel could not be executed\n"); 
         cout<<ret;
       }
       
       //ret = clEnqueueReadBuffer(command_queue, mu1_mem_obj, CL_TRUE, 0, LIST_SIZE*sizeof(float), ssim, 0, NULL, NULL);
       
       cout<<"Setting arguments of kernel2 ssim- \n"; 
       // Set the arguments of the kernel ssim2
       ret = clSetKernelArg(kernel_ssim2, 0, sizeof(cl_mem), (void *)&mu1_mem_obj);
       ret = clSetKernelArg(kernel_ssim2, 1, sizeof(cl_mem), (void *)&mu2_mem_obj);
       ret = clSetKernelArg(kernel_ssim2, 2, sizeof(cl_mem), (void *)&filter_mem_obj   );
       ret = clSetKernelArg(kernel_ssim2, 3, sizeof(cl_mem), (void *)&img1_sq_mem_obj  );
       ret = clSetKernelArg(kernel_ssim2, 4, sizeof(cl_mem), (void *)&img2_sq_mem_obj  );
       ret = clSetKernelArg(kernel_ssim2, 5, sizeof(cl_mem), (void *)&img1_img2_mem_obj);
       ret = clSetKernelArg(kernel_ssim2, 6, sizeof(cl_mem), (void *)&ssim_index_mem_obj);
       ret = clSetKernelArg(kernel_ssim2, 7, sizeof(int), &x);
       ret = clSetKernelArg(kernel_ssim2, 8, sizeof(int), &y);
       ret = clSetKernelArg(kernel_ssim2, 9, sizeof(int), &nChan);
       ret = clSetKernelArg(kernel_ssim2, 10, sizeof(int), &filter_size);
       ret = clSetKernelArg(kernel_ssim2, 11, sizeof(float), &C1);
       ret = clSetKernelArg(kernel_ssim2, 12, sizeof(float), &C2);
       
       cout<<"Executing ssim kernel2 - \n";
       // Execute the OpenCL kernel on the list
       //ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);
       ret = clEnqueueNDRangeKernel(command_queue, kernel_ssim2, 2, NULL, global_item_size, local_item_size, 0, NULL, NULL);
       
       // Read the memory buffer C on the device to the local variable C
       //ret = clEnqueueReadBuffer(command_queue, ssim_index_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(float), ssim, 0, NULL, NULL);
       ret = clEnqueueReadBuffer(command_queue, ssim_index_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(float), ssim, 0, NULL, NULL);
    
       cout<<"Cleaning up the memory bufffer\n"; 
       // Clean up
       ret = clReleaseMemObject(src1_mem_obj      );
       ret = clReleaseMemObject(src2_mem_obj      );
       ret = clReleaseMemObject(filter_mem_obj    );
       ret = clReleaseMemObject(img1_sq_mem_obj   );
       ret = clReleaseMemObject(img2_sq_mem_obj   );
       ret = clReleaseMemObject(img1_img2_mem_obj );
       ret = clReleaseMemObject(mu1_mem_obj       );
       ret = clReleaseMemObject(mu2_mem_obj       );
       ret = clReleaseMemObject(ssim_index_mem_obj);
    }
 
    // Clean up
    void clean_up_host() {
       ret = clFlush(command_queue);
       ret = clFinish(command_queue);
       ret = clReleaseKernel(kernel_gaus);
       ret = clReleaseKernel(kernel_ssim2);
       ret = clReleaseKernel(kernel_ssim1);
       ret = clReleaseProgram(program);
       ret = clReleaseCommandQueue(command_queue);
       ret = clReleaseContext(context);
    }
};

Mat get_gaussian_filter(int filter_size, int sigma) {
  Mat filter1D;
  filter1D = getGaussianKernel(filter_size, sigma, CV_32F);
  Mat filter2D = filter1D * filter1D.t();
  return filter2D;
}

void print_time(clock_t start, int TIMES, const char *s) {
  double diff, time;
  diff = ( std::clock() - start ) / (double)CLOCKS_PER_SEC;
  time = diff/TIMES;
  printf("Time of CPU run using %s (averaged for %d runs): %f milliseconds.\n",s,TIMES, time);
  printf("Time of CPU run using %s (total for %d runs): %f milliseconds.\n",s, TIMES, diff);
}

CvScalar ssim(IplImage *src1, IplImage *src2)
{
    double K1;
    double K2;
    int gaussian_window;
    double gaussian_sigma;
    int L;
    IplImage *ssim_map;
    CvScalar mssim_value;
    IplImage *cs_map;
    CvScalar mean_cs_value; // mean of contrast, structure (part of l,c,s)
      
    K1 = 0.01;
    K2 = 0.03;
    gaussian_window = 11;
    gaussian_sigma = 1.5;
    L = 255;
    ssim_map = NULL;
    for (int i=0; i < 4; i++)
      mssim_value.val[i] = -1; // Initialize with an out of bound value of mssim [0,1]
    cs_map = NULL;
    for (int i=0; i < 4; i++)
      mean_cs_value.val[i] = -1; // Initialize with an out of bound value of mssim [0,1]
    
    int x = src1->width, y = src1->height;
    // default settings
    const double C1 = (K1 * L) * (K1 * L); 
    const double C2 = (K2 * L) * (K2 * L);
 
    int nChan = src1->nChannels;
    int d = IPL_DEPTH_32F;
    CvSize size = cvSize(x, y);
    
    //creating FLOAT type images of src1 and src2 
    IplImage *img1 = cvCreateImage(size, d, nChan);
    IplImage *img2 = cvCreateImage(size, d, nChan);
  
    //Image squares
    IplImage *img1_sq = cvCreateImage(size, d, nChan);
    IplImage *img2_sq = cvCreateImage(size, d, nChan);
    IplImage *img1_img2 = cvCreateImage(size, d, nChan);
    
    cvConvert(src1, img1);
 		cvConvert(src2, img2);
    
    //Squaring the images thus created
    cvPow(img1, img1_sq, 2);
    cvPow(img2, img2_sq, 2);
    cvMul(img1, img2, img1_img2, 1);
  
    IplImage *mu1 = cvCreateImage(size, d, nChan);
    IplImage *mu2 = cvCreateImage(size, d, nChan);
    IplImage *mu1_sq = cvCreateImage(size, d, nChan);
    IplImage *mu2_sq = cvCreateImage(size, d, nChan);
    IplImage *mu1_mu2 = cvCreateImage(size, d, nChan);
  
    IplImage *sigma1_sq = cvCreateImage(size, d, nChan);
    IplImage *sigma2_sq = cvCreateImage(size, d, nChan);
    IplImage *sigma12 = cvCreateImage(size, d, nChan);
  
    //PRELIMINARY COMPUTING
    //gaussian smoothing is performed
    cvSmooth(img1, mu1, CV_GAUSSIAN, gaussian_window, gaussian_window, gaussian_sigma);
    cvSmooth(img2, mu2, CV_GAUSSIAN, gaussian_window, gaussian_window, gaussian_sigma);
  
    //gettting mu, mu_sq, mu1_mu2
    cvPow(mu1, mu1_sq, 2);
    cvPow(mu2, mu2_sq, 2);
    cvMul(mu1, mu2, mu1_mu2, 1);
  
    //calculating sigma1, sigma2, sigma12
    cvSmooth(img1_sq, sigma1_sq, CV_GAUSSIAN, gaussian_window, gaussian_window, gaussian_sigma);
    cvSub(sigma1_sq, mu1_sq, sigma1_sq);
  
    cvSmooth(img2_sq, sigma2_sq, CV_GAUSSIAN, gaussian_window, gaussian_window, gaussian_sigma);
    cvSub(sigma2_sq, mu2_sq, sigma2_sq);
  
    cvSmooth(img1_img2, sigma12, CV_GAUSSIAN, gaussian_window, gaussian_window, gaussian_sigma);
    cvSub(sigma12, mu1_mu2, sigma12);
    
    //releasing some junk buffers
    cvReleaseImage(&img1);
    cvReleaseImage(&img2);
    cvReleaseImage(&img1_sq);
    cvReleaseImage(&img2_sq);
    cvReleaseImage(&img1_img2);
    cvReleaseImage(&mu1);
    cvReleaseImage(&mu2);
    
    // creating buffers for numerator and denominator 
    IplImage *numerator1 = cvCreateImage(size, d, nChan);
    IplImage *numerator2 = cvCreateImage(size, d, nChan);
    IplImage *numerator = cvCreateImage(size, d, nChan);
    IplImage *denominator1 = cvCreateImage(size, d, nChan);
    IplImage *denominator2 = cvCreateImage(size, d, nChan);
    IplImage *denominator = cvCreateImage(size, d, nChan);
  
    // FORMULA to calculate SSIM
    // (2*mu1_mu2 + C1)
    cvScale(mu1_mu2, numerator1, 2);
    cvAddS(numerator1, cvScalarAll(C1), numerator1);
    // (2*sigma12 + C2) 
    cvScale(sigma12, numerator2, 2);
    cvAddS(numerator2, cvScalarAll(C2), numerator2);
    // ((2*mu1_mu2 + C1).*(2*sigma12 + C2))
    cvMul(numerator1, numerator2, numerator, 1);
  
    // (mu1_sq + mu2_sq + C1)
    cvAdd(mu1_sq, mu2_sq, denominator1);
    cvAddS(denominator1, cvScalarAll(C1), denominator1);
    // (sigma1_sq + sigma2_sq + C2) >>>
    cvAdd(sigma1_sq, sigma2_sq, denominator2);
    cvAddS(denominator2, cvScalarAll(C2),denominator2);
    // ((mu1_sq + mu2_sq + C1).*(sigma1_sq + sigma2_sq + C2))
    cvMul(denominator1, denominator2, denominator, 1);

    //Release some junk buffers 
    cvReleaseImage(&numerator1);
    cvReleaseImage(&denominator1);
    cvReleaseImage(&mu1_sq);
    cvReleaseImage(&mu2_sq);
    cvReleaseImage(&mu1_mu2);
    cvReleaseImage(&sigma1_sq);
    cvReleaseImage(&sigma2_sq);
    cvReleaseImage(&sigma12);

    //ssim map and cs_map
    ssim_map = cvCreateImage(size, d, nChan);
    cs_map = cvCreateImage(size, d, nChan);
    // SSIM_INDEX map 
    // ((2*mu1_mu2 + C1).*(2*sigma12 + C2))./((mu1_sq + mu2_sq + C1).*(sigma1_sq + sigma2_sq + C2))
    cvDiv(numerator, denominator, ssim_map, 1);
    // Contrast Structure CS_index map
    // (2*sigma12 + C2)./(sigma1_sq + sigma2_sq + C2)
    cvDiv(numerator2, denominator2, cs_map, 1);
  
    // average is taken for both SSIM_map and CS_map 
    mssim_value = cvAvg(ssim_map);
    mean_cs_value = cvAvg(cs_map);

    //Release images
    cvReleaseImage(&numerator);
    cvReleaseImage(&denominator);
    cvReleaseImage(&numerator2);
    cvReleaseImage(&denominator2);

    return mssim_value;
    
}

void test_ssim (IplImage * src1, IplImage *src2 ) {

  int TIMES=10;
  double diff, time, result;
  clock_t start;
  int x = src1->width, y = src1->height;
  int nChan = src1->nChannels;
  cout<<"Image Dimensions- ";
  cout<<x<<" "<<y<<" "<<nChan<<" \n";

  Mat tempo = get_gaussian_filter(11,1.5);
  float *filter = (float*)tempo.data;

  //Creating a blurred image of the input image for testing
  CvScalar out1, out2;
  // Testing using Normal SSIM
  start = clock();
  for(int tt=0;tt<TIMES;tt++)
    out1 = ssim(src1, src2);
  print_time(start,TIMES, "SSIM normal" );
  cout<<"output 1= "<<out1.val[0]<<" "<<out1.val[1]<<" "<<out1.val[2]<<"\n";

  // Testing for OpenCl
  host_program_openCl ssim_host;
  char kernel_ssim1[30] = "ssim_part1.cl";
  char kernel_ssim2[30] = "ssim_part2.cl";
  /*
  ssim_host.load_kernel(kernel_ssim1);
  ssim_host.setup_ssim1("ssim_A");
  ssim_host.load_kernel(kernel_ssim2);
  ssim_host.setup_ssim2("ssim_B");*/
  ssim_host.setup_ssim();
  cout<<"Kernel successfully loaded\n";
  cout<<"Host successfully setup\n";
  start = clock();
  // Execute the OpenCL kernel on the list
  int LIST_SIZE = x*y*nChan;
  int LOCAL_SIZE = 1;
  float K1 = 0.01;
  float K2 = 0.03;
  int L = 255;
  float C1 = (K1 * L) * (K1 * L); 
  float C2 = (K2 * L) * (K2 * L);
  int d = IPL_DEPTH_32F;
  IplImage *img1 = cvCreateImage(cvSize(x,y),d,nChan); 
  IplImage *img2 = cvCreateImage(cvSize(x,y),d,nChan);
  cvConvert(src1,img1); 
  cvConvert(src2,img2); 
 
  IplImage *ssim_index = cvCreateImage(cvSize(x,y), IPL_DEPTH_32F, nChan);
  for(int tt=0;tt<TIMES;tt++)
    ssim_host.execute_ssim((float*)(img1->imageData), (float*)(img2->imageData), filter, (float*)(ssim_index->imageData),LIST_SIZE,LOCAL_SIZE,x,y,nChan,11, C1, C2);
  cout<<"Successfully executed \n";
  print_time(start,TIMES, "SSIM OpenCl" );
  out2 = cvAvg(ssim_index);
  cout<<"output 2= "<<out2.val[0]<<" "<<out2.val[1]<<" "<<out2.val[2]<<"\n";


}

void test_gaussian (IplImage * src1, IplImage *src2 ) {

  int TIMES=10;
  double diff, time, result;
  clock_t start;
  int x = src1->width, y = src1->height;
  int nChan = src1->nChannels;
  cout<<"Image Dimensions- ";
  cout<<x<<" "<<y<<" "<<nChan<<" \n";

  Mat tempo = get_gaussian_filter(11,2);
  float *filter = (float*)tempo.data;

  //Creating a blurred image of the input image for testing
  IplImage *out1 = cvCreateImage(cvSize(x,y), IPL_DEPTH_8U, nChan);
  
  // Testing using CvSmooth
  start = clock();
  for(int tt=0;tt<TIMES;tt++)
    cvSmooth(src1, out1, CV_GAUSSIAN, 11 , 11, 2);
  print_time(start,TIMES, "cvSmooth" );
  cvSaveImage("lena_blur1.bmp",out1);
  cvSaveImage("lena.bmp",src1);
  
  // Testing using openCl
  int d = IPL_DEPTH_32F;
  IplImage *img1 = cvCreateImage(cvSize(x,y),d,nChan); 
  IplImage *out2 = cvCreateImage(cvSize(x,y),d,nChan);
  cvConvert(src1,img1); 
  // -------------- Parallel 2-----------
  host_program_openCl blur_host;
  char kernel_mse[30] = "gaussian_filter.cl";
  blur_host.load_kernel("gaussian_filter.cl");
  cout<<"Kernel successfully loaded\n";
  blur_host.setup_gaussian("gaussian_filter");
  cout<<"Host successfully setup\n";
  start = clock();
  // Execute the OpenCL kernel on the list
  int LIST_SIZE = x*y*nChan;
  int LOCAL_SIZE = 1;
  for(int tt=0;tt<TIMES;tt++)
    blur_host.execute_gauss((float*)(img1->imageData), filter, (float*)(out2->imageData),LIST_SIZE,LOCAL_SIZE,x,y,nChan,11);
  cout<<"Successfully executed \n";
  print_time(start,TIMES, "OpenCl" );

  // Testing using Naive implementation
  IplImage *out3 = cvCreateImage(cvSize(x,y),d,nChan);
  float *input = (float*)img1->imageData;
  float *out = (float*)out3->imageData;
  int size = x*y*nChan;
  int size_filter = 11;
  int i,j,h,w,k;
  cout<<"\nx="<<x<<"y="<<y<<"\n"; 
  start = clock();
  for(h=0; h<x;h++)
  {
    for(w=0;w<x;w++)
    {
         int rowOffset = h*x*3;
         int my = rowOffset + 3*w;
         if(h>=size_filter/2 && h<x-size_filter/2 && w>=size_filter/2 && w<x-size_filter/2)
         {
             float sumR = 0.0;
             float sumG = 0.0;
             float sumB = 0.0;
             k=0;
             for(int  i=-size_filter/2 ; i<=size_filter/2; i++)
             {
               int curRow = my + i*(x*3);
               for(int  j=-size_filter/2 ; j<=size_filter/2; j++,k++)
               {
                 int offset = j*3;
                 sumB += input[curRow + offset] * filter[k];
                 sumG += input[curRow + offset + 1] * filter[k];
                 sumR += input[curRow + offset + 2] * filter[k];
               }
             }
             out[my] =  sumB;
             out[my + 1] = sumG;
             out[my + 2] = sumR;
         }
         else {
             out[my] = input[my] ;
             out[my + 1] = input[my + 1];
             out[my + 2] = input[my + 2];
         }
    }
  }
  print_time(start,TIMES, "Naive Implementation" );

  cvSaveImage("lena_blur2.bmp",out2);
  cvSaveImage("lena_blur3.bmp",out3);

  CvScalar avg1 = cvAvg(src1);
  CvScalar avg2 = cvAvg(out1);
  CvScalar avg3 = cvAvg(out2);
  CvScalar avg4 = cvAvg(out3);

  cout<<avg1.val[0]<<" " <<avg1.val[1]<<" "<<avg1.val[2]<<"\n";
  cout<<avg2.val[0]<<" " <<avg2.val[1]<<" "<<avg2.val[2]<<"\n";
  cout<<avg3.val[0]<<" " <<avg3.val[1]<<" "<<avg3.val[2]<<"\n";
  cout<<avg4.val[0]<<" " <<avg4.val[1]<<" "<<avg4.val[2]<<"\n";

  //Release images
  cvReleaseImage(&out1);
  cvReleaseImage(&out3);

}

int main(int argc, char** argv)
{
  int TIMES=10;
  double diff, time, result;
  clock_t start;
  IplImage *src1;
  IplImage *src2;
  src1 = cvLoadImage(argv[1]);
  src2 = cvLoadImage(argv[2]);
  int x =src1->height, y =src1->width, nChan = src1->nChannels;
  IplImage *blur = cvCreateImage(cvSize(x,y), IPL_DEPTH_8U, nChan);
  cvSmooth(src1, blur, CV_GAUSSIAN, 11 , 11, 2);

  test_gaussian(src1,src2);
  test_ssim(src1,blur);
  //Release images
  cvReleaseImage(&src1);
  cvReleaseImage(&src2);

}

