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

int AddSimple(float *A, float *B, float *C, int LIST_SIZE) {

    // -------------- Non-parallel -----------
    int i, j;
    for(j=0; j<LIST_SIZE; j++)
      C[j] = A[j]+ B[j];
    return 1;
}

//int AddOpenCl (float *A1, float *B1, float *C2, int LIST_SIZE, int LOCAL_SIZE) {
int AddOpenCl (float *A, float *B, float *C, int LIST_SIZE, int LOCAL_SIZE, int TIMES) {

    int i;
    // Load the kernel source code into the array source_str
    FILE *fp;
    char *source_str;
    size_t source_size;

    fp = fopen("vector_add_kernel.cl", "r");
    if (!fp) {
        fprintf(stderr, "Failed to load kernel.\n");
        exit(1);
    }
    source_str = (char*)malloc(MAX_SOURCE_SIZE);
    source_size = fread( source_str, 1, MAX_SOURCE_SIZE, fp);
    fclose( fp );

    // Get platform and device information
    cl_platform_id platform_id = NULL;
    cl_device_id device_id = NULL;
    cl_uint ret_num_devices;
    cl_uint ret_num_platforms;
    cl_int ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
    ret = clGetDeviceIDs( platform_id, CL_DEVICE_TYPE_ALL, 1, &device_id, &ret_num_devices);

    // Create an OpenCL context
    cl_context context = clCreateContext( NULL, 1, &device_id, NULL, NULL, &ret);

    // Create a command queue
    cl_command_queue command_queue = clCreateCommandQueue(context, device_id, 0, &ret);

    // Create memory buffers on the device for each vector
    cl_mem a_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);
    cl_mem b_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);
    cl_mem c_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);

    // Copy the lists A and B to their respective memory buffers
    ret = clEnqueueWriteBuffer(command_queue, a_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(float), A, 0, NULL, NULL);
    ret = clEnqueueWriteBuffer(command_queue, b_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(float), B, 0, NULL, NULL);

    // Create a program from the kernel source
    cl_program program = clCreateProgramWithSource(context, 1, (const char **)&source_str, (const size_t *)&source_size, &ret);

    // Build the program
    ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);

    // Create the OpenCL kernel
    cl_kernel kernel = clCreateKernel(program, "vector_add", &ret);

    // Set the arguments of the kernel
    ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&a_mem_obj);
    ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&b_mem_obj);
    ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&c_mem_obj);
    
    // Execute the OpenCL kernel on the list
    size_t global_item_size = LIST_SIZE; // Process the entire lists
    size_t local_item_size = LOCAL_SIZE; // Process in groups of 64
    for(i=0; i<TIMES; i++)
      ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);

    // Read the memory buffer C on the device to the local variable C
    ret = clEnqueueReadBuffer(command_queue, c_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(float), C, 0, NULL, NULL);

    // Clean up
    ret = clFlush(command_queue);
    ret = clFinish(command_queue);
    ret = clReleaseKernel(kernel);
    ret = clReleaseProgram(program);
    ret = clReleaseMemObject(a_mem_obj);
    ret = clReleaseMemObject(b_mem_obj);
    ret = clReleaseMemObject(c_mem_obj);
    ret = clReleaseCommandQueue(command_queue);
    ret = clReleaseContext(context);

    return 1;
}

int cvAbsDiff_OpenCl (IplImage *src1, IplImage *src2, IplImage *output, int TIMES) {

    float *A;
    float *B;
    float *C;

    A = (float *)src1->imageData;
    B = (float *)src2->imageData;
    C = (float *)output->imageData;

    int LIST_SIZE = (src1->height)*(src1->width)*(src1->nChannels);
    int LOCAL_SIZE;
    LOCAL_SIZE = src1->height;

    int i;
    // Load the kernel source code into the array source_str
    FILE *fp;
    char *source_str;
    size_t source_size;

    fp = fopen("vector_abs_diff_kernel.cl", "r");
    if (!fp) {
        fprintf(stderr, "Failed to load kernel.\n");
        exit(1);
    }
    source_str = (char*)malloc(MAX_SOURCE_SIZE);
    source_size = fread( source_str, 1, MAX_SOURCE_SIZE, fp);
    fclose( fp );

    // Get platform and device information
    cl_platform_id platform_id = NULL;
    cl_device_id device_id = NULL;
    cl_uint ret_num_devices;
    cl_uint ret_num_platforms;
    cl_int ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
    ret = clGetDeviceIDs( platform_id, CL_DEVICE_TYPE_ALL, 1, &device_id, &ret_num_devices);

    // Create an OpenCL context
    cl_context context = clCreateContext( NULL, 1, &device_id, NULL, NULL, &ret);

    // Create a command queue
    cl_command_queue command_queue = clCreateCommandQueue(context, device_id, 0, &ret);

    // Create memory buffers on the device for each vector
    cl_mem a_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);
    cl_mem b_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);
    cl_mem c_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);

    // Copy the lists A and B to their respective memory buffers
    ret = clEnqueueWriteBuffer(command_queue, a_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(float), A, 0, NULL, NULL);
    ret = clEnqueueWriteBuffer(command_queue, b_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(float), B, 0, NULL, NULL);

    // Create a program from the kernel source
    cl_program program = clCreateProgramWithSource(context, 1, (const char **)&source_str, (const size_t *)&source_size, &ret);

    // Build the program
    ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);

    // Create the OpenCL kernel
    cl_kernel kernel = clCreateKernel(program, "vector_abs_diff", &ret);

    // Set the arguments of the kernel
    ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&a_mem_obj);
    ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&b_mem_obj);
    ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&c_mem_obj);
    
    // Execute the OpenCL kernel on the list
    size_t global_item_size = LIST_SIZE; // Process the entire lists
    size_t local_item_size = LOCAL_SIZE; // Process in groups of 64
    //for(i=0; i<TIMES; i++)
      ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);

    // Read the memory buffer C on the device to the local variable C
    ret = clEnqueueReadBuffer(command_queue, c_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(float), C, 0, NULL, NULL);

    // Clean up
    ret = clFlush(command_queue);
    ret = clFinish(command_queue);
    ret = clReleaseKernel(kernel);
    ret = clReleaseProgram(program);
    ret = clReleaseMemObject(a_mem_obj);
    ret = clReleaseMemObject(b_mem_obj);
    ret = clReleaseMemObject(c_mem_obj);
    ret = clReleaseCommandQueue(command_queue);
    ret = clReleaseContext(context);

    return 1;
}

int cvPow_OpenCl (IplImage *src1, IplImage *output, int n, int TIMES) {

    float *A;
    float *C;

    A = (float *)src1->imageData;
    C = (float *)output->imageData;

    int LIST_SIZE = (src1->height)*(src1->width)*(src1->nChannels);
    int LOCAL_SIZE;
    LOCAL_SIZE = src1->height;

    int i;
    // Load the kernel source code into the array source_str
    FILE *fp;
    char *source_str;
    size_t source_size;

    fp = fopen("vector_pow_kernel.cl", "r");
    if (!fp) {
        fprintf(stderr, "Failed to load kernel.\n");
        exit(1);
    }
    source_str = (char*)malloc(MAX_SOURCE_SIZE);
    source_size = fread( source_str, 1, MAX_SOURCE_SIZE, fp);
    fclose( fp );

    // Get platform and device information
    cl_platform_id platform_id = NULL;
    cl_device_id device_id = NULL;
    cl_uint ret_num_devices;
    cl_uint ret_num_platforms;
    cl_int ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
    ret = clGetDeviceIDs( platform_id, CL_DEVICE_TYPE_ALL, 1, &device_id, &ret_num_devices);

    // Create an OpenCL context
    cl_context context = clCreateContext( NULL, 1, &device_id, NULL, NULL, &ret);

    // Create a command queue
    cl_command_queue command_queue = clCreateCommandQueue(context, device_id, 0, &ret);

    // Create memory buffers on the device for each vector
    cl_mem a_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);
    cl_mem c_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);

    // Copy the lists A and B to their respective memory buffers
    ret = clEnqueueWriteBuffer(command_queue, a_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(float), A, 0, NULL, NULL);

    // Create a program from the kernel source
    cl_program program = clCreateProgramWithSource(context, 1, (const char **)&source_str, (const size_t *)&source_size, &ret);

    // Build the program
    ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);

    // Create the OpenCL kernel
    cl_kernel kernel = clCreateKernel(program, "vector_pow", &ret);

    // Set the arguments of the kernel
    ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&a_mem_obj);
    ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&c_mem_obj);
    ret = clSetKernelArg(kernel, 2, sizeof(int), &n);
    
    // Execute the OpenCL kernel on the list
    size_t global_item_size = LIST_SIZE; // Process the entire lists
    size_t local_item_size = LOCAL_SIZE; // Process in groups of 64
    //for(i=0; i<TIMES; i++)
    ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);

    // Read the memory buffer C on the device to the local variable C
    ret = clEnqueueReadBuffer(command_queue, c_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(float), C, 0, NULL, NULL);

    // Clean up
    ret = clFlush(command_queue);
    ret = clFinish(command_queue);
    ret = clReleaseKernel(kernel);
    ret = clReleaseProgram(program);
    ret = clReleaseMemObject(a_mem_obj);
    ret = clReleaseMemObject(c_mem_obj);
    ret = clReleaseCommandQueue(command_queue);
    ret = clReleaseContext(context);

    return 1;
}

int cvMse_OpenCl (IplImage *src1, IplImage *src2, IplImage *output, int TIMES) {

    float *A;
    float *B;
    float *C;

    A = (float *)src1->imageData;
    B = (float *)src2->imageData;
    C = (float *)output->imageData;

    int LIST_SIZE = (src1->height)*(src1->width)*(src1->nChannels);
    int LOCAL_SIZE;
    LOCAL_SIZE = src1->height;

    int i;
    // Load the kernel source code into the array source_str
    FILE *fp;
    char *source_str;
    size_t source_size;

    fp = fopen("vector_mse_kernel.cl", "r");
    if (!fp) {
        fprintf(stderr, "Failed to load kernel.\n");
        exit(1);
    }
    source_str = (char*)malloc(MAX_SOURCE_SIZE);
    source_size = fread( source_str, 1, MAX_SOURCE_SIZE, fp);
    fclose( fp );

    // Get platform and device information
    cl_platform_id platform_id = NULL;
    cl_device_id device_id = NULL;
    cl_uint ret_num_devices;
    cl_uint ret_num_platforms;
    cl_int ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
    ret = clGetDeviceIDs( platform_id, CL_DEVICE_TYPE_ALL, 1, &device_id, &ret_num_devices);

    // Create an OpenCL context
    cl_context context = clCreateContext( NULL, 1, &device_id, NULL, NULL, &ret);

    // Create a command queue
    cl_command_queue command_queue = clCreateCommandQueue(context, device_id, 0, &ret);

    // Create memory buffers on the device for each vector
    cl_mem a_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);
    cl_mem b_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);
    cl_mem c_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);

    // Copy the lists A and B to their respective memory buffers
    ret = clEnqueueWriteBuffer(command_queue, a_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(float), A, 0, NULL, NULL);
    ret = clEnqueueWriteBuffer(command_queue, b_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(float), B, 0, NULL, NULL);

    // Create a program from the kernel source
    cl_program program = clCreateProgramWithSource(context, 1, (const char **)&source_str, (const size_t *)&source_size, &ret);

    // Build the program
    ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);

    // Create the OpenCL kernel
    cl_kernel kernel = clCreateKernel(program, "vector_mse", &ret);

    // Set the arguments of the kernel
    ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&a_mem_obj);
    ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&b_mem_obj);
    ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&c_mem_obj);
    
    // Execute the OpenCL kernel on the list
    size_t global_item_size = LIST_SIZE; // Process the entire lists
    size_t local_item_size = LOCAL_SIZE; // Process in groups of 64
    //for(i=0; i<TIMES; i++)
      ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);

    // Read the memory buffer C on the device to the local variable C
    ret = clEnqueueReadBuffer(command_queue, c_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(float), C, 0, NULL, NULL);

    // Clean up
    ret = clFlush(command_queue);
    ret = clFinish(command_queue);
    ret = clReleaseKernel(kernel);
    ret = clReleaseProgram(program);
    ret = clReleaseMemObject(a_mem_obj);
    ret = clReleaseMemObject(b_mem_obj);
    ret = clReleaseMemObject(c_mem_obj);
    ret = clReleaseCommandQueue(command_queue);
    ret = clReleaseContext(context);

    return 1;
}

CvScalar calc_MSE(IplImage *src1, IplImage *src2)
{
      int x = src1->width, y = src1->height;
      int nChan = src1->nChannels, d = IPL_DEPTH_32F;
      //size before down sampling
      CvSize size = cvSize(x, y);
      
      //creating diff and difference squares
      IplImage *img1 = cvCreateImage(size, d, nChan);
      IplImage *img2 = cvCreateImage(size, d, nChan);
      IplImage *diff = cvCreateImage(size, d, nChan);
      IplImage *diff_sq = cvCreateImage(size, d, nChan);
  
      cvConvert(src1, img1);
    	cvConvert(src2, img2);
      cvAbsDiff(img1, img2, diff);
      //Squaring the images thus created
      cvPow(diff, diff_sq, 2);
      CvScalar mse = cvAvg(diff_sq);

      //Release images
      cvReleaseImage(&img1);
      cvReleaseImage(&img2);
      cvReleaseImage(&diff);
      cvReleaseImage(&diff_sq);

      return mse;
}

CvScalar calc_MSE_openCl(IplImage *src1, IplImage *src2)
{
      int x = src1->width, y = src1->height;
      int nChan = src1->nChannels, d = IPL_DEPTH_32F;
      //size before down sampling
      CvSize size = cvSize(x, y);
      
      //creating diff and difference squares
      IplImage *img1 = cvCreateImage(size, d, nChan);
      IplImage *img2 = cvCreateImage(size, d, nChan);
      IplImage *diff = cvCreateImage(size, d, nChan);
      IplImage *diff_sq = cvCreateImage(size, d, nChan);
  
      cvConvert(src1, img1);
    	cvConvert(src2, img2);
      cvAbsDiff_OpenCl(img1, img2, diff, 1);
      //Squaring the images thus created
      cvPow_OpenCl(diff, diff_sq, 2, 1);
  
      CvScalar mse = cvAvg(diff_sq);
      //Release images
      cvReleaseImage(&img1);
      cvReleaseImage(&img2);
      cvReleaseImage(&diff);
      cvReleaseImage(&diff_sq);
      return mse;
}

CvScalar calc_MSE_openCl2(IplImage *src1, IplImage *src2)
{
      int x = src1->width, y = src1->height;
      int nChan = src1->nChannels, d = IPL_DEPTH_32F;
      //size before down sampling
      CvSize size = cvSize(x, y);
      
      //creating diff and difference squares
      IplImage *img1 = cvCreateImage(size, d, nChan);
      IplImage *img2 = cvCreateImage(size, d, nChan);
      IplImage *diff_sq = cvCreateImage(size, d, nChan);
  
      cvConvert(src1, img1);
    	cvConvert(src2, img2);
      cvMse_OpenCl(img1, img2, diff_sq, 1);
  
      CvScalar mse = cvAvg(diff_sq);
      //Release images
      cvReleaseImage(&img1);
      cvReleaseImage(&img2);
      cvReleaseImage(&diff_sq);
      return mse;
}

int main(int argc, char** argv)
{
  IplImage *src1;
  IplImage *src2;
  src1 = cvLoadImage(argv[1]);
  src2 = cvLoadImage(argv[2]);

  int x = src1->width, y = src1->height;
  int nChan = src1->nChannels;
  
  //Creating a blurred image of the input image for testing
  IplImage *src3 = cvCreateImage(cvSize(x,y), IPL_DEPTH_8U, nChan);
  cvSmooth(src1, src3, CV_GAUSSIAN, 11 , 11, 2);
  cvSaveImage("test_images/lena_blur.bmp",src3);
  
  cout<<"MSE value - \n";

  CvScalar mse, mse2, mse3;

    int i, j;
    int TIMES = 1;
    clock_t start;

    start = clock();
    double diff, time, result;
    // -------------- Non-parallel -----------
    for(j=0; j<TIMES; j++)
      mse = calc_MSE(src1, src3);
    diff = ( std::clock() - start ) / (double)CLOCKS_PER_SEC;
    time = diff/TIMES;
    printf("Time of CPU run (averaged for %d runs): %f milliseconds.\n",TIMES, time);
    printf("Time of CPU run (total for %d runs): %f milliseconds.\n",TIMES, diff);

    // -------------- Parallel -----------
    start = clock();
    // Execute the OpenCL kernel on the list
    for(i=0; i<TIMES; i++)
      mse2 = calc_MSE_openCl(src1, src3);

    diff = ( std::clock() - start ) / (double)CLOCKS_PER_SEC;
    time = diff/TIMES;
    printf("Time of CPU run (averaged for %d runs): %f milliseconds.\n",TIMES, time);
    printf("Time of CPU run (total for %d runs): %f milliseconds.\n",TIMES, diff);

    // -------------- Parallel 2-----------
    start = clock();
    // Execute the OpenCL kernel on the list
    for(i=0; i<TIMES; i++)
      mse3 = calc_MSE_openCl2(src1, src3);

    diff = ( std::clock() - start ) / (double)CLOCKS_PER_SEC;
    time = diff/TIMES;
    printf("Time of CPU run (averaged for %d runs): %f milliseconds.\n",TIMES, time);
    printf("Time of CPU run (total for %d runs): %f milliseconds.\n",TIMES, diff);

    cout<<mse.val[0]<<" "<<mse.val[1]<<" "<<mse.val[2]<<" "<<mse.val[3]<<"\n";
    cout<<"...................................................................................\n";
    cout<<mse2.val[0]<<" "<<mse2.val[1]<<" "<<mse2.val[2]<<" "<<mse2.val[3]<<"\n";
    cout<<"...................................................................................\n";
    cout<<mse3.val[0]<<" "<<mse3.val[1]<<" "<<mse3.val[2]<<" "<<mse3.val[3]<<"\n";
    cout<<"...................................................................................\n";

    /*
    // Program to test the openCL code for array addition

    cout<<"OpenCL Code Test RUn-\n";
    // Create the two input vectors
    int i, j;
    //nst float LIST_SIZE = 1048576 ;
    const int LIST_SIZE = 10 ;
    float *A = (float*)malloc(sizeof(float)*LIST_SIZE);
    float *B = (float*)malloc(sizeof(float)*LIST_SIZE);
    float *C1 = (float*)malloc(sizeof(float)*LIST_SIZE);
    float *C2 = (float*)malloc(sizeof(float)*LIST_SIZE);
    for(i = 0; i < LIST_SIZE; i++) {
        A[i] = i;
        B[i] = LIST_SIZE - i;
    }
    
    int TIMES = 1;
    clock_t start;

    start = clock();
    double diff, time, result;
    // -------------- Non-parallel -----------
    for(j=0; j<TIMES; j++)
     AddSimple(A, B, C1, LIST_SIZE);
    diff = ( std::clock() - start ) / (double)CLOCKS_PER_SEC;
    time = diff/TIMES;
    printf("Time of CPU run (averaged for %d runs): %f milliseconds.\n",TIMES, time);
    printf("Time of CPU run (total for %d runs): %f milliseconds.\n",TIMES, diff);

    // -------------- Parallel -----------
    start = clock();
    // Execute the OpenCL kernel on the list
    size_t global_item_size = LIST_SIZE; // Process the entire lists
    size_t local_item_size = 2; // Process in groups of 64
    //for(i=0; i<TIMES; i++)
      AddOpenCl(A, B, C2, LIST_SIZE, local_item_size, TIMES);

    diff = ( std::clock() - start ) / (double)CLOCKS_PER_SEC;
    time = diff/TIMES;
    printf("Time of CPU run (averaged for %d runs): %f milliseconds.\n",TIMES, time);
    printf("Time of CPU run (total for %d runs): %f milliseconds.\n",TIMES, diff);

    // Display the result to the screen
    printf("Result Simple Add\n");
    for(i = 0; i < LIST_SIZE; i++)
        printf("%f + %f = %f\n", A[i], B[i], C1[i]);
    printf("Result Parallel Add\n");
    for(i = 0; i < LIST_SIZE; i++)
        printf("%f + %f = %f\n", A[i], B[i], C2[i]);

    free(A);
    free(B);
    free(C1);
    free(C2);
  */

  //Release images
  cvReleaseImage(&src1);
  cvReleaseImage(&src2);
  cvReleaseImage(&src3);

}



