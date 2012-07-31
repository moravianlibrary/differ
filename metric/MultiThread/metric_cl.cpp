#include <stdlib.h>
#include <sstream>
#include <ctime>
#include <cv.h>
#include <highgui.h>
#include <stdio.h>
#include <iostream>
#include <time.h>
#include <sys/time.h>
#ifndef DEBUG
#define DEBUG
#endif
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#define MAX_SOURCE_SIZE (0x100000)
using namespace cv;
using namespace std;

void print_time(clock_t start, int TIMES, const char *s) {
  double diff, time;
  diff = ( std::clock() - start ) / (double)CLOCKS_PER_SEC;
  time = diff/TIMES;
  printf("Time of CPU run using %s (averaged for %d runs): %f milliseconds.\n",s,TIMES, time);
  printf("Time of CPU run using %s (total for %d runs): %f milliseconds.\n",s, TIMES, diff);
}

class host_program_openCl {
  
  public:
      
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

      cl_int ret;
    
      char * load_kernel (const char * kernel_name) {
        // Load the kernel source code into the array source_str
        FILE *fp;
        char *source_str;
        printf("%s\n",kernel_name); 
        fp = fopen(kernel_name, "r");
        if (!fp) {
            fprintf(stderr, "Failed to load kernel.\n");
        }
        source_str = (char*)malloc(MAX_SOURCE_SIZE);
        source_size = fread( source_str, 1, MAX_SOURCE_SIZE, fp);
        fclose( fp );

        return source_str;
      }
      
      void setup () {
        
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
      }

      cl_kernel create_program (const char *kernel_function_name, const char *source_str, const size_t source_size) {
      
        // Create the OpenCL kernels
        cl_kernel kernel; 
        
        // Create a program from the kernel source
        program = clCreateProgramWithSource(context, 1, (const char **)&source_str, (const size_t *)&source_size, &ret);
       
        // Build the program
        ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
       
        // Create the OpenCL kernel
        kernel = clCreateKernel(program, kernel_function_name, &ret);

        return kernel;
     }

    // Clean up
    void clean_up_host() {
       ret = clFlush(command_queue);
       ret = clFinish(command_queue);
       ret = clReleaseProgram(program);
       ret = clReleaseCommandQueue(command_queue);
       ret = clReleaseContext(context);
    }

};

class MSE_openCl : public host_program_openCl { 

  public:

    char *source_str;
    size_t source_size_mse;
    // Create the OpenCL kernels
    cl_kernel kernel_mse; 
    const char *mse;

    void Init() {
      
      mse = "mse.cl";
      setup();
      source_str = load_kernel(mse);
      source_size_mse = source_size;
      kernel_mse = create_program("vector_mse", source_str, source_size_mse);
    }
   
    void execute_mse (float *A, float *B, float *C, int LIST_SIZE, int LOCAL_SIZE) {
  
       size_t global_item_size = LIST_SIZE;
       size_t local_item_size = LOCAL_SIZE;
       cl_event event[1];
       // Create memory buffers on the device for each vector
       cl_mem a_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);
       cl_mem b_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);
       cl_mem c_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);
   
       // Copy the lists A and B to their respective memory buffers
       ret = clEnqueueWriteBuffer(command_queue, a_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(float), A, 0, NULL, NULL);
       ret = clEnqueueWriteBuffer(command_queue, b_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(float), B, 0, NULL, NULL);
   
       // Set the arguments of the kernel
       ret = clSetKernelArg(kernel_mse, 0, sizeof(cl_mem), (void *)&a_mem_obj);
       ret = clSetKernelArg(kernel_mse, 1, sizeof(cl_mem), (void *)&b_mem_obj);
       ret = clSetKernelArg(kernel_mse, 2, sizeof(cl_mem), (void *)&c_mem_obj);
       
       // Execute the OpenCL kernel on the list
       ret = clEnqueueNDRangeKernel(command_queue, kernel_mse, 1, NULL, &global_item_size, &local_item_size, 0, NULL, &event[0]);
       if (ret!=CL_SUCCESS) {
         printf("Error: Kernel could not be executed\n"); 
         cout<<ret;
       }
       clWaitForEvents(1, &event[0]);
   
       // Read the memory buffer C on the device to the local variable C
       ret = clEnqueueReadBuffer(command_queue, c_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(float), C, 0, NULL, NULL);
       // Clean up
       ret = clReleaseMemObject(a_mem_obj);
       ret = clReleaseMemObject(b_mem_obj);
       ret = clReleaseMemObject(c_mem_obj);
    }
    
    // Clean up
    void clean_up_host() {
       ret = clReleaseKernel(kernel_mse);
    }

};

class SSIM_openCl : public host_program_openCl { 

  public:

    char *source_str_ssim1;
    char *source_str_ssim2;
    size_t source_size_ssim1;
    size_t source_size_ssim2;
    // Create the OpenCL kernels
    cl_kernel kernel_ssim1, kernel_ssim2;
    const char *ssim1;
    const char *ssim2;

    void Init() {
      
      ssim1 = "ssim_part1.cl";
      ssim2 = "ssim_part2.cl";
      setup();
      source_str_ssim1 = load_kernel(ssim1);
      source_size_ssim1 = source_size;
      kernel_ssim1 = create_program("ssim_A", source_str_ssim1, source_size_ssim1);
      source_str_ssim2 = load_kernel(ssim2);
      source_size_ssim2 = source_size;
      kernel_ssim2 = create_program("ssim_B", source_str_ssim2, source_size_ssim2);
    }
    
    void execute_ssim (float *src1, float *src2, float *filter, float *ssim, int LIST_SIZE, int LOCAL_SIZE, int x, int y, int nChan, int filter_size, float C1, float C2) {
  
       size_t global_item_size[] = {x,y};
       size_t local_item_size[] = {1,1};
       cl_event event[2];
       
       #ifdef DEBUG
       cout<<"Creating the memory buffers-\n";
       #endif
       // Create memory buffers on the device for each vector
       cl_mem src1_mem_obj      = clCreateBuffer(context, CL_MEM_READ_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);
       cl_mem src2_mem_obj      = clCreateBuffer(context, CL_MEM_READ_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);
       cl_mem filter_mem_obj    = clCreateBuffer(context, CL_MEM_READ_ONLY, filter_size*filter_size* sizeof(float), NULL, &ret);
       cl_mem img1_sq_mem_obj   = clCreateBuffer(context, CL_MEM_READ_WRITE, LIST_SIZE * sizeof(float), NULL, &ret);
       cl_mem img2_sq_mem_obj   = clCreateBuffer(context, CL_MEM_READ_WRITE, LIST_SIZE * sizeof(float), NULL, &ret);
       cl_mem img1_img2_mem_obj = clCreateBuffer(context, CL_MEM_READ_WRITE, LIST_SIZE * sizeof(float), NULL, &ret);
       cl_mem mu1_mem_obj       = clCreateBuffer(context, CL_MEM_READ_WRITE, LIST_SIZE * sizeof(float), NULL, &ret);
       cl_mem mu2_mem_obj       = clCreateBuffer(context, CL_MEM_READ_WRITE, LIST_SIZE * sizeof(float), NULL, &ret);
       cl_mem ssim_index_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);
     
       if (!src1_mem_obj || !src2_mem_obj || !filter_mem_obj || !img1_sq_mem_obj || !img2_sq_mem_obj || !img1_img2_mem_obj || 
           !mu1_mem_obj || !mu2_mem_obj || !ssim_index_mem_obj)
           cout<<"Failed to allocate device memory!\n"; 
        
       #ifdef DEBUG
       cout<<"Allocating memory buffers - \n"; 
       #endif
       // Copy the lists Image Data src1, src2 to their respective memory buffers
       ret = clEnqueueWriteBuffer(command_queue, src1_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(float), src1, 0, NULL, NULL);
       ret = clEnqueueWriteBuffer(command_queue, src2_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(float), src2, 0, NULL, NULL);
       if (ret != CL_SUCCESS)
          printf("Error: Failed to write to source array!\n");
       ret = clEnqueueWriteBuffer(command_queue, filter_mem_obj, CL_TRUE, 0, filter_size*filter_size* sizeof(float), filter, 0, NULL, NULL);
        
       #ifdef DEBUG
       cout<<"Setting arguments of kernel1 ssim- \n"; 
       #endif
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
       #ifdef DEBUG
       cout<<"Executing ssim kernel1 - \n";
       #endif
       //for(int l=1; l <100;l++)
        ret = clEnqueueNDRangeKernel(command_queue, kernel_ssim1, 2, NULL, global_item_size, local_item_size, 0, NULL, &event[0]);
       if (ret!=CL_SUCCESS) {
         printf("Error: Kernel could not be executed\n"); 
         cout<<ret;
       }
       clWaitForEvents(1, &event[0]);
       
       //ret = clEnqueueReadBuffer(command_queue, mu1_mem_obj, CL_TRUE, 0, LIST_SIZE*sizeof(float), ssim, 0, NULL, NULL);
       #ifdef DEBUG
       cout<<"Setting arguments of kernel2 ssim- \n"; 
       #endif
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
  
       #ifdef DEBUG
       cout<<"Executing ssim kernel2 - \n";
       #endif
       // Execute the OpenCL kernel on the list
       ret = clEnqueueNDRangeKernel(command_queue, kernel_ssim2, 2, NULL, global_item_size, local_item_size, 0, NULL, &event[1]);
       clWaitForEvents(1, &event[1]);
       
       // Read the memory buffer C on the device to the local variable C
       ret = clEnqueueReadBuffer(command_queue, ssim_index_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(float), ssim, 0, NULL, NULL);
    
       #ifdef DEBUG
       cout<<"Cleaning up the memory bufffer\n"; 
       #endif
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
       ret = clReleaseKernel(kernel_ssim1);
       ret = clReleaseKernel(kernel_ssim2);
    }

};

class MS_SSIM_openCl : public SSIM_openCl {

  public:
      
    CvScalar execute_ms_ssim (IplImage *src1, IplImage *src2, float *filter, IplImage **ms_ssim_map, int LIST_SIZE, int LOCAL_SIZE, int x, int y, int nChan, int filter_size, float C1, float C2) {
    
    //Creating an array of IplImages for ssim_map at various levels
    float alpha[5] = {0.0, 0.0, 0.0, 0.0, 0.1333};
    float beta[5] = {0.0448, 0.2856, 0.3001, 0.2363, 0.1333};
    float gamma[5] = {0.0448, 0.2856, 0.3001, 0.2363, 0.1333};
    int level =5; 
    //IplImage** ms_ssim_map;
    CvScalar ms_ssim_value;
      ms_ssim_map = (IplImage**)(malloc(sizeof(IplImage*)*level));
      int d = IPL_DEPTH_32F;

      #ifdef DEBUG
      cout<<"\nBeta = "<<beta[0]<<" "<<beta[1]<<" "<<beta[2]<<" "<<beta[3]<<"\n";
      #endif

      for (int i=0; i<level; i++)
      {
        //Downsampling of the original images
        IplImage *downsampleSrc1, *downsampleSrc2;
        //Downsampling the images
        CvSize downs_size = cvSize((int)(x/pow(2, i)), (int)(y/pow(2, i)));
        downsampleSrc1 = cvCreateImage(downs_size, d, nChan);
        downsampleSrc2 = cvCreateImage(downs_size, d, nChan);
        cvResize(src1, downsampleSrc1, CV_INTER_NN);
        cvResize(src2, downsampleSrc2, CV_INTER_NN);
        ms_ssim_map[i] = cvCreateImage(downs_size, d, nChan);
        CvScalar test_avg = cvAvg(downsampleSrc1);
        cout<<"Checking for average value - "<<test_avg.val[0]<<" "<<test_avg.val[1]<<" "<<test_avg.val[2]<<"\n";

        int x1 = downsampleSrc1->height;
        int y1 = downsampleSrc1->width;
        LIST_SIZE = x1*y1*nChan;
        LOCAL_SIZE = 1 ;
        #ifdef DEBUG
        cout<<"Values at level="<<i<<" \n";
        #endif 

        execute_ssim((float*)(downsampleSrc1->imageData), (float*)(downsampleSrc2->imageData), filter, (float*)(ms_ssim_map[i]->imageData),LIST_SIZE,LOCAL_SIZE,x1,y1,nChan,11, C1, C2);

        CvScalar mssim_t = cvAvg(ms_ssim_map[i]);
        CvScalar mcs_t = cvAvg(ms_ssim_map[i]);
        
        #ifdef DEBUG
        cout<<"Size of MAP at level = "<<i<<" size = "<<ms_ssim_map[i]->width<<" "<<ms_ssim_map[i]->height<<"\n";
        #endif
        cout<<"Some values for testing - "<<mssim_t.val[0]<<" "<<mssim_t.val[1]<<" "<<mssim_t.val[2]<<"\n";

        for (int j=0; j < 4; j++)
        {
          if (i == 0)
            ms_ssim_value.val[j] = pow((mcs_t.val[j]), (double)(beta[i]));
          else 
            if (i == level-1)
              ms_ssim_value.val[j] = (ms_ssim_value.val[j]) * pow((mssim_t.val[j]), (double)(beta[i]));
            else
              ms_ssim_value.val[j] = (ms_ssim_value.val[j]) * pow((mcs_t.val[j]), (double)(beta[i]));
        }
        //Release images
        cvReleaseImage(&downsampleSrc1);
        cvReleaseImage(&downsampleSrc2);

      }
      return ms_ssim_value;
    }
    
};

class ImageQuI_openCl : public host_program_openCl { 

  public:

    char *source_str_iqi1;
    char *source_str_iqi2;
    size_t source_size_iqi1;
    size_t source_size_iqi2;
    // Create the OpenCL kernels
    cl_kernel kernel_iqi1, kernel_iqi2;
    const char *iqi1;
    const char *iqi2;

    void Init() {
      
      iqi1 = "img_qi1.cl";
      iqi2 = "img_qi2.cl";
      setup();
      source_str_iqi1 = load_kernel(iqi1);
      source_size_iqi1 = source_size;
      kernel_iqi1 = create_program("img_qi_A", source_str_iqi1, source_size_iqi1);
      source_str_iqi2 = load_kernel(iqi2);
      source_size_iqi2 = source_size;
      kernel_iqi2 = create_program("img_qi_B", source_str_iqi2, source_size_iqi2);
    }
    
    void execute_iqi (float *src1, float *src2, float *iqi, int LIST_SIZE, int LOCAL_SIZE, int x, int y, int nChan, int filter_size) {
  
       size_t global_item_size[] = {x,y};
       size_t local_item_size[] = {1,1};
       cl_event event[2];
       
       #ifdef DEBUG
       cout<<"Creating the memory buffers-\n";
       #endif
       // Create memory buffers on the device for each vector
       cl_mem src1_mem_obj      = clCreateBuffer(context, CL_MEM_READ_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);
       cl_mem src2_mem_obj      = clCreateBuffer(context, CL_MEM_READ_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);
       cl_mem img1_sq_mem_obj   = clCreateBuffer(context, CL_MEM_READ_WRITE, LIST_SIZE * sizeof(float), NULL, &ret);
       cl_mem img2_sq_mem_obj   = clCreateBuffer(context, CL_MEM_READ_WRITE, LIST_SIZE * sizeof(float), NULL, &ret);
       cl_mem img1_img2_mem_obj = clCreateBuffer(context, CL_MEM_READ_WRITE, LIST_SIZE * sizeof(float), NULL, &ret);
       cl_mem mu1_mem_obj       = clCreateBuffer(context, CL_MEM_READ_WRITE, LIST_SIZE * sizeof(float), NULL, &ret);
       cl_mem mu2_mem_obj       = clCreateBuffer(context, CL_MEM_READ_WRITE, LIST_SIZE * sizeof(float), NULL, &ret);
       cl_mem iqi_index_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);
     
       if (!src1_mem_obj || !src2_mem_obj || !img1_sq_mem_obj || !img2_sq_mem_obj || !img1_img2_mem_obj || 
           !mu1_mem_obj || !mu2_mem_obj || !iqi_index_mem_obj)
           cout<<"Failed to allocate device memory!\n"; 
        
       #ifdef DEBUG
       cout<<"Allocating memory buffers - \n"; 
       #endif
       // Copy the lists Image Data src1, src2 to their respective memory buffers
       ret = clEnqueueWriteBuffer(command_queue, src1_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(float), src1, 0, NULL, NULL);
       ret = clEnqueueWriteBuffer(command_queue, src2_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(float), src2, 0, NULL, NULL);
       if (ret != CL_SUCCESS)
          printf("Error: Failed to write to source array!\n");
        
       #ifdef DEBUG
       cout<<"Setting arguments of kernel1 iqi- \n"; 
       #endif
       // Set the arguments of the kernel iqi1
       ret = clSetKernelArg(kernel_iqi1, 0, sizeof(cl_mem), (void *)&src1_mem_obj);
       ret = clSetKernelArg(kernel_iqi1, 1, sizeof(cl_mem), (void *)&src2_mem_obj);
       ret = clSetKernelArg(kernel_iqi1, 2, sizeof(cl_mem), (void *)&img1_sq_mem_obj  );
       ret = clSetKernelArg(kernel_iqi1, 3, sizeof(cl_mem), (void *)&img2_sq_mem_obj  );
       ret = clSetKernelArg(kernel_iqi1, 4, sizeof(cl_mem), (void *)&img1_img2_mem_obj);
       ret = clSetKernelArg(kernel_iqi1, 5, sizeof(cl_mem), (void *)&mu1_mem_obj);
       ret = clSetKernelArg(kernel_iqi1, 6, sizeof(cl_mem), (void *)&mu2_mem_obj);
       ret = clSetKernelArg(kernel_iqi1, 7, sizeof(int), &x);
       ret = clSetKernelArg(kernel_iqi1, 8, sizeof(int), &y);
       ret = clSetKernelArg(kernel_iqi1, 9, sizeof(int), &nChan);
       ret = clSetKernelArg(kernel_iqi1, 10, sizeof(int), &filter_size);
       
       // Execute the OpenCL kernel on the list
       #ifdef DEBUG
       cout<<"Executing iqi kernel1 - \n";
       #endif
       //for(int l=1; l <100;l++)
        ret = clEnqueueNDRangeKernel(command_queue, kernel_iqi1, 2, NULL, global_item_size, local_item_size, 0, NULL, &event[0]);
       if (ret!=CL_SUCCESS) {
         printf("Error: Kernel could not be executed\n"); 
         cout<<ret;
       }
       clWaitForEvents(1, &event[0]);
       
       //ret = clEnqueueReadBuffer(command_queue, mu1_mem_obj, CL_TRUE, 0, LIST_SIZE*sizeof(float), iqi, 0, NULL, NULL);
       #ifdef DEBUG
       cout<<"Setting arguments of kernel2 iqi- \n"; 
       #endif
       // Set the arguments of the kernel iqi2
       ret = clSetKernelArg(kernel_iqi2, 0, sizeof(cl_mem), (void *)&mu1_mem_obj);
       ret = clSetKernelArg(kernel_iqi2, 1, sizeof(cl_mem), (void *)&mu2_mem_obj);
       ret = clSetKernelArg(kernel_iqi2, 2, sizeof(cl_mem), (void *)&img1_sq_mem_obj  );
       ret = clSetKernelArg(kernel_iqi2, 3, sizeof(cl_mem), (void *)&img2_sq_mem_obj  );
       ret = clSetKernelArg(kernel_iqi2, 4, sizeof(cl_mem), (void *)&img1_img2_mem_obj);
       ret = clSetKernelArg(kernel_iqi2, 5, sizeof(cl_mem), (void *)&iqi_index_mem_obj);
       ret = clSetKernelArg(kernel_iqi2, 6, sizeof(int), &x);
       ret = clSetKernelArg(kernel_iqi2, 7, sizeof(int), &y);
       ret = clSetKernelArg(kernel_iqi2, 8, sizeof(int), &nChan);
       ret = clSetKernelArg(kernel_iqi2, 9, sizeof(int), &filter_size);
  
       #ifdef DEBUG
       cout<<"Executing iqi kernel2 - \n";
       #endif
       // Execute the OpenCL kernel on the list
       ret = clEnqueueNDRangeKernel(command_queue, kernel_iqi2, 2, NULL, global_item_size, local_item_size, 0, NULL, &event[1]);
       clWaitForEvents(1, &event[1]);
       
       // Read the memory buffer C on the device to the local variable C
       ret = clEnqueueReadBuffer(command_queue, iqi_index_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(float), iqi, 0, NULL, NULL);
    
       #ifdef DEBUG
       cout<<"Cleaning up the memory bufffer\n"; 
       #endif
       // Clean up
       ret = clReleaseMemObject(src1_mem_obj      );
       ret = clReleaseMemObject(src2_mem_obj      );
       ret = clReleaseMemObject(img1_sq_mem_obj   );
       ret = clReleaseMemObject(img2_sq_mem_obj   );
       ret = clReleaseMemObject(img1_img2_mem_obj );
       ret = clReleaseMemObject(mu1_mem_obj       );
       ret = clReleaseMemObject(mu2_mem_obj       );
       ret = clReleaseMemObject(iqi_index_mem_obj );
    }
    
    // Clean up
    void clean_up_host() {
       ret = clReleaseKernel(kernel_iqi1);
       ret = clReleaseKernel(kernel_iqi2);
    }

};

Mat get_gaussian_filter(int filter_size, int sigma) {
  Mat filter1D;
  filter1D = getGaussianKernel(filter_size, sigma, CV_32F);
  Mat filter2D = filter1D * filter1D.t();
  return filter2D;
}

CvScalar iqi(IplImage * src1, IplImage * src2)
{
    IplImage *image_quality_map;
    CvScalar image_quality_value;
  int B = 8;
      int x = src1->width, y = src1->height;
      int nChan = src1->nChannels, d = IPL_DEPTH_32F;
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
    
      //average smoothing is performed
      cvSmooth(img1, mu1, CV_BLUR, B, B);
      cvSmooth(img2, mu2, CV_BLUR, B, B);
    
      //gettting mu, mu_sq, mu1_mu2
      cvPow(mu1, mu1_sq, 2);
      cvPow(mu2, mu2_sq, 2);
      cvMul(mu1, mu2, mu1_mu2, 1);
    
      //calculating sigma1, sigma2, sigma12
      cvSmooth(img1_sq, sigma1_sq, CV_BLUR, B, B);
      cvSub(sigma1_sq, mu1_sq, sigma1_sq);
    
      cvSmooth(img2_sq, sigma2_sq, CV_BLUR, B, B);
      cvSub(sigma2_sq, mu2_sq, sigma2_sq);
    
      cvSmooth(img1_img2, sigma12, CV_BLUR, B, B);
      cvSub(sigma12, mu1_mu2, sigma12);
 
      //Releasing unused images 
      cvReleaseImage(&img1);
      cvReleaseImage(&img2);
      cvReleaseImage(&img1_sq);
      cvReleaseImage(&img2_sq);
      cvReleaseImage(&img1_img2);
      
      // creating buffers for numerator and denominator and a temp buffer
      IplImage *numerator1 = cvCreateImage(size, d, nChan);
      IplImage *numerator = cvCreateImage(size, d, nChan);
      IplImage *denominator1 = cvCreateImage(size, d, nChan);
      IplImage *denominator2 = cvCreateImage(size, d, nChan);
      IplImage *denominator = cvCreateImage(size, d, nChan);
    
      // FORMULA to calculate Image Quality Index
    
      // (4*sigma12)
      cvScale(sigma12, numerator1, 4);
    
      // (4*sigma12).*(mu1*mu2)
      cvMul(numerator1, mu1_mu2, numerator, 1);
    
      // (mu1_sq + mu2_sq)
      cvAdd(mu1_sq, mu2_sq, denominator1);
    
      // (sigma1_sq + sigma2_sq)
      cvAdd(sigma1_sq, sigma2_sq, denominator2);
      
      //Release images
      cvReleaseImage(&mu1);
      cvReleaseImage(&mu2);
      cvReleaseImage(&mu1_sq);
      cvReleaseImage(&mu2_sq);
      cvReleaseImage(&mu1_mu2);
      cvReleaseImage(&sigma1_sq);
      cvReleaseImage(&sigma2_sq);
      cvReleaseImage(&sigma12);
      cvReleaseImage(&numerator1);
    
      // ((mu1_sq + mu2_sq).*(sigma1_sq + sigma2_sq))
      cvMul(denominator1, denominator2, denominator, 1);
    
      //image_quality map
      image_quality_map = cvCreateImage(size, d, nChan);
      
      // ((4*sigma12).*(mu1_mu2))./((mu1_sq + mu2_sq).*(sigma1_sq + sigma2_sq))
      cvDiv(numerator, denominator, image_quality_map, 1);
    
      // image_quality_map created in image_quality_map
      // average is taken 
      image_quality_value = cvAvg(image_quality_map);

      //Release images
      cvReleaseImage(&numerator);
      cvReleaseImage(&denominator);
      cvReleaseImage(&denominator1);
      cvReleaseImage(&denominator2);
      return image_quality_value;

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

void test_mse (IplImage * src1, IplImage *src2 ) {

  int TIMES=1;
  double diff, time, result;
  clock_t start;
  int x = src1->width, y = src1->height;
  int nChan = src1->nChannels;
  cout<<"Image Dimensions- ";
  cout<<x<<" "<<y<<" "<<nChan<<" \n";

  //Creating a blurred image of the input image for testing
  CvScalar out1, out2;
  MSE_openCl mse_host;
  mse_host.Init();
  cout<<"Kernel successfully loaded\n";
  start = clock();
  // Execute the OpenCL kernel on the list
  int LIST_SIZE = x*y*nChan;
  int LOCAL_SIZE = 1;
  int d = IPL_DEPTH_32F;
  IplImage *img1 = cvCreateImage(cvSize(x,y),d,nChan); 
  IplImage *img2 = cvCreateImage(cvSize(x,y),d,nChan);
  cvConvert(src1,img1); 
  cvConvert(src2,img2); 
 
  IplImage *mse_index = cvCreateImage(cvSize(x,y), IPL_DEPTH_32F, nChan);
  for(int tt=0;tt<TIMES;tt++)
    mse_host.execute_mse((float*)(img1->imageData), (float*)(img2->imageData), (float*)(mse_index->imageData), LIST_SIZE, LOCAL_SIZE);
  cout<<"Successfully executed \n";
  //sim_host.clean_up_host();
  print_time(start,TIMES, "SSIM OpenCl" );
  out2 = cvAvg(mse_index);
  cout<<"output 2= "<<out2.val[0]<<" "<<out2.val[1]<<" "<<out2.val[2]<<"\n";
}

void test_ssim (IplImage * src1, IplImage *src2 ) {

struct timeval start_time;
struct timeval end_time;
double start_dtime, end_dtime;
gettimeofday(&start_time,NULL);
  int TIMES=1;
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
gettimeofday(&end_time,NULL);
start_dtime=(double)start_time.tv_sec+(double)start_time.tv_usec/1000000.0;
end_dtime=(double)end_time.tv_sec+(double)end_time.tv_usec/1000000.0; 
diff=end_dtime-start_dtime;
printf("Reading Buffer - %ld %ld %f %ld %ld %f %f\n",start_time.tv_sec,start_time.tv_usec,start_dtime,end_time.tv_sec,end_time.tv_usec,end_dtime,diff); 


  // Testing for OpenCl
  
  SSIM_openCl ssim_host;
  ssim_host.Init();
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
  //sim_host.clean_up_host();
  print_time(start,TIMES, "SSIM OpenCl" );
  out2 = cvAvg(ssim_index);
  cout<<"output 2= "<<out2.val[0]<<" "<<out2.val[1]<<" "<<out2.val[2]<<"\n";


}

void test_ms_ssim (IplImage * src1, IplImage *src2 ) {

struct timeval start_time;
struct timeval end_time;
double start_dtime, end_dtime;
gettimeofday(&start_time,NULL);
  int TIMES=1;
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


  // Testing for OpenCl
  MS_SSIM_openCl ssim_host;
  ssim_host.Init();
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
 
  IplImage **ssim_index;
  out2 = ssim_host.execute_ms_ssim(img1, img2, filter, ssim_index,LIST_SIZE,LOCAL_SIZE,x,y,nChan,11, C1, C2);
  cout<<"Successfully executed \n";
  //sim_host.clean_up_host();
  print_time(start,TIMES, "SSIM OpenCl" );
  cout<<"output 2= "<<out2.val[0]<<" "<<out2.val[1]<<" "<<out2.val[2]<<"\n";

gettimeofday(&end_time,NULL);
start_dtime=(double)start_time.tv_sec+(double)start_time.tv_usec/1000000.0;
end_dtime=(double)end_time.tv_sec+(double)end_time.tv_usec/1000000.0; 
diff=end_dtime-start_dtime;
printf("Reading Buffer - %ld %ld %f %ld %ld %f %f\n",start_time.tv_sec,start_time.tv_usec,start_dtime,end_time.tv_sec,end_time.tv_usec,end_dtime,diff); 

}

void test_iqi (IplImage * src1, IplImage *src2 ) {

  int TIMES=1;
  double diff, time, result;
  clock_t start;
  int x = src1->width, y = src1->height;
  int nChan = src1->nChannels;
  cout<<"Image Dimensions- ";
  cout<<x<<" "<<y<<" "<<nChan<<" \n";

  //Creating a blurred image of the input image for testing
  CvScalar out1, out2;
  
  // Testing using Normal IQI
  start = clock();
  for(int tt=0;tt<TIMES;tt++)
    out1 = iqi(src1, src2);
  print_time(start,TIMES, "IQI normal" );
  cout<<"output 1= "<<out1.val[0]<<" "<<out1.val[1]<<" "<<out1.val[2]<<"\n";
  // Testing for OpenCl
  ImageQuI_openCl iqi_host; 
  iqi_host.Init();
  cout<<"Kernel successfully loaded\n";
  cout<<"Host successfully setup\n";
  start = clock();
  // Execute the OpenCL kernel on the list
  int LIST_SIZE = x*y*nChan;
  int LOCAL_SIZE = 1;
  int d = IPL_DEPTH_32F;
  IplImage *img1 = cvCreateImage(cvSize(x,y),d,nChan); 
  IplImage *img2 = cvCreateImage(cvSize(x,y),d,nChan);
  cvConvert(src1,img1); 
  cvConvert(src2,img2); 
  int B = 8;
  IplImage *iqi_index = cvCreateImage(cvSize(x,y), IPL_DEPTH_32F, nChan);
  for(int tt=0;tt<TIMES;tt++)
    iqi_host.execute_iqi((float*)(img1->imageData), (float*)(img2->imageData), (float*)(iqi_index->imageData),LIST_SIZE,LOCAL_SIZE,x,y,nChan,B);
  cout<<"Successfully executed \n";
  print_time(start,TIMES, "IQI OpenCl" );
  out2 = cvAvg(iqi_index);
  cout<<"output 2= "<<out2.val[0]<<" "<<out2.val[1]<<" "<<out2.val[2]<<"\n";
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

  //test_gaussian(src1,src2);
  test_ssim(src1,blur);
  test_mse(src1,blur);
  test_iqi(src1,blur);
  test_ms_ssim(src1,blur);
  //Release images
  cvReleaseImage(&src1);
  cvReleaseImage(&src2);

}

