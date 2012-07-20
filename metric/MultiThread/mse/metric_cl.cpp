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
      // Create the OpenCL kernel
      cl_kernel kernel;

      size_t global_item_size;
      size_t local_item_size;

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
      
      void setup (const char *kernel_function_name) {
      
        platform_id = NULL;
        device_id = NULL;
        
        // Get platform and device information
        ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
        ret = clGetDeviceIDs( platform_id, CL_DEVICE_TYPE_ALL, 1, &device_id, &ret_num_devices);
       
        // Create an OpenCL context
        context = clCreateContext( NULL, 1, &device_id, NULL, NULL, &ret);
       
        // Create a command queue
        command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
        
        // Create a program from the kernel source
        program = clCreateProgramWithSource(context, 1, (const char **)&source_str, (const size_t *)&source_size, &ret);
       
        // Build the program
        ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
       
        // Create the OpenCL kernel
        kernel = clCreateKernel(program, kernel_function_name, &ret);

     }

      void execute (float *A, float *B, float *C, int LIST_SIZE, int LOCAL_SIZE) {
  
       global_item_size = LIST_SIZE;
       local_item_size = LOCAL_SIZE;
       // Create memory buffers on the device for each vector
       cl_mem a_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);
       cl_mem b_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);
       cl_mem c_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);
   
       // Copy the lists A and B to their respective memory buffers
       ret = clEnqueueWriteBuffer(command_queue, a_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(float), A, 0, NULL, NULL);
       ret = clEnqueueWriteBuffer(command_queue, b_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(float), B, 0, NULL, NULL);
   
       // Set the arguments of the kernel
       ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&a_mem_obj);
       ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&b_mem_obj);
       ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&c_mem_obj);
       
       // Execute the OpenCL kernel on the list
       ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);
   
       // Read the memory buffer C on the device to the local variable C
       ret = clEnqueueReadBuffer(command_queue, c_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(float), C, 0, NULL, NULL);
       // Clean up
       ret = clReleaseMemObject(a_mem_obj);
       ret = clReleaseMemObject(b_mem_obj);
       ret = clReleaseMemObject(c_mem_obj);
    }

    // Clean up
    void clean_up_host() {
       ret = clFlush(command_queue);
       ret = clFinish(command_queue);
       ret = clReleaseKernel(kernel);
       ret = clReleaseProgram(program);
       ret = clReleaseCommandQueue(command_queue);
       ret = clReleaseContext(context);
    }
};

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
    //LOCAL_SIZE = LIST_SIZE/2;

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

    // Create a program from the kernel source
    cl_program program = clCreateProgramWithSource(context, 1, (const char **)&source_str, (const size_t *)&source_size, &ret);

    // Build the program
    ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);

    // Create the OpenCL kernel
    cl_kernel kernel = clCreateKernel(program, "vector_mse", &ret);


   // -------------------------------------------------------------
    int i, j;
    clock_t start;

    start = clock();
    double diff, time, result;
    // -------------- Non-parallel -----------
    
    for(j=0; j<TIMES; j++)
    {

    
    // Create memory buffers on the device for each vector
    cl_mem a_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);
    cl_mem b_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);
    cl_mem c_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);

    // Copy the lists A and B to their respective memory buffers
    ret = clEnqueueWriteBuffer(command_queue, a_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(float), A, 0, NULL, NULL);
    ret = clEnqueueWriteBuffer(command_queue, b_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(float), B, 0, NULL, NULL);

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
    ret = clReleaseMemObject(a_mem_obj);
    ret = clReleaseMemObject(b_mem_obj);
    ret = clReleaseMemObject(c_mem_obj);
    
  }

    // Clean up
    ret = clFlush(command_queue);
    ret = clFinish(command_queue);
    ret = clReleaseKernel(kernel);
    ret = clReleaseProgram(program);
    ret = clReleaseCommandQueue(command_queue);
    ret = clReleaseContext(context);

    diff = ( std::clock() - start ) / (double)CLOCKS_PER_SEC;
    time = diff/TIMES;
    printf("In host Program Time of CPU run (averaged for %d runs): %f milliseconds.\n",TIMES, time);
    printf("In host program Time of CPU run (total for %d runs): %f milliseconds.\n",TIMES, diff);

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

CvScalar calc_MSE_openCl(IplImage *src1, IplImage *src2, host_program_openCl *mse_host)
{
      int x = src1->width, y = src1->height;
      int nChan = src1->nChannels, d = IPL_DEPTH_32F;
      //size before down sampling
      CvSize size = cvSize(x, y);
      int LIST_SIZE=x*y*nChan;
      //int LOCAL_SIZE = x*nChan ; 
      int LOCAL_SIZE = x ; 
      cout<<"x="<<x<<" y="<<y<<" nChan="<<nChan <<" LIST_SIZE="<<LIST_SIZE<<" LOCAL_SIZE"<<LOCAL_SIZE<<"\n";
      //creating diff and difference squares
      IplImage *img1 = cvCreateImage(size, d, nChan);
      IplImage *img2 = cvCreateImage(size, d, nChan);
      IplImage *diff_sq = cvCreateImage(size, d, nChan);
  
      cvConvert(src1, img1);
    	cvConvert(src2, img2);
      mse_host->execute((float*)img1->imageData,(float*)img2->imageData,(float*)diff_sq->imageData, LIST_SIZE ,LOCAL_SIZE );
  
      CvScalar mse = cvAvg(diff_sq);
      //Release images
      cvReleaseImage(&img1);
      cvReleaseImage(&img2);
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
      cvMse_OpenCl(img1, img2, diff_sq, 10);
  
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
  cout<<"Image Dimensions- ";
  cout<<x<<" "<<y<<" "<<nChan<<" \n";

  Mat p;
 int h;
  p = getGaussianKernel(11,2, CV_32F );
  cout<<p.rows<<" "<<p.cols<<"\n";
  float *data1 = (float*)p.data;
  for(h=0;h<(p.rows)*(p.cols);h++)
    printf("%f\n",data1[h]);
  p = getGaussianKernel(3,1,CV_32F);
  cout<<p.rows<<" "<<p.cols<<"\n";
  float *data2 = (float*)p.data;
  for(h=0;h<(p.rows)*(p.cols);h++)
    printf("%f\n",data2[h]);

  Mat pp = p * p.t();
  cout<<pp.rows<<" "<<pp.cols<<"\n";
  float *data3 = (float*)pp.data;
  for(h=0;h<(pp.rows)*(pp.cols);h++)
    printf("%f\n",data3[h]);


  //Creating a blurred image of the input image for testing
  IplImage *src3 = cvCreateImage(cvSize(x,y), IPL_DEPTH_8U, nChan);
  cvSmooth(src1, src3, CV_GAUSSIAN, 11 , 11, 2);
  cvSaveImage("test_images/lena_blur.bmp",src3);
  
  cout<<"MSE value - \n";

  CvScalar mse, mse2, mse3;

    int i, j;
    int TIMES = 10;
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

    /*// -------------- Parallel 3-----------
    start = clock();
    // Execute the OpenCL kernel on the list
    for(i=0; i<TIMES; i++)
      mse3 = calc_MSE_openCl2(src1, src3);
    diff = ( std::clock() - start ) / (double)CLOCKS_PER_SEC;
    time = diff/TIMES;
    printf("Time of CPU run (averaged for %d runs): %f milliseconds.\n",TIMES, time);
    printf("Time of CPU run (total for %d runs): %f milliseconds.\n",TIMES, diff);
    */

     // -------------- Parallel 2-----------
    host_program_openCl mse_host;
    char kernel_mse[30] = "vector_mse_kernel.cl";
    mse_host.load_kernel("vector_mse_kernel.cl");
    mse_host.setup("vector_mse");
    start = clock();
    // Execute the OpenCL kernel on the list
    for(i=0; i<TIMES; i++)
    {
     mse2 = calc_MSE_openCl(src1, src3, &mse_host);
     // cout<<"check result - "<<mse2.val[0]<<" "<<mse2.val[1]<<" "<<mse2.val[2]<<" "<<mse2.val[3]<<"\n";
    }
    diff = ( std::clock() - start ) / (double)CLOCKS_PER_SEC;
    time = diff/TIMES;
    printf("Time of CPU run using host_program class (averaged for %d runs): %f milliseconds.\n",TIMES, time);
    printf("Time of CPU run using host program class (total for %d runs): %f milliseconds.\n",TIMES, diff);

    //mse_host.clean_up_host();

    cout<<mse.val[0]<<" "<<mse.val[1]<<" "<<mse.val[2]<<" "<<mse.val[3]<<"\n";
    cout<<"...................................................................................\n";
    //cout<<mse3.val[0]<<" "<<mse3.val[1]<<" "<<mse3.val[2]<<" "<<mse3.val[3]<<"\n";
    cout<<"...................................................................................\n";
    cout<<mse2.val[0]<<" "<<mse2.val[1]<<" "<<mse2.val[2]<<" "<<mse2.val[3]<<"\n";
    cout<<"...................................................................................\n";

  //Release images
  cvReleaseImage(&src1);
  cvReleaseImage(&src2);
  cvReleaseImage(&src3);

}

