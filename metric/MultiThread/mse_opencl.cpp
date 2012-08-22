#include "core.h"
#include "similarityMetric.h"
#include "host_program_opencl.h"
#include "mse_opencl.h"

MSE_openCl :: MSE_openCl()
{
  L = 255;
  for (int i=0; i < 4; i++)
    mse.val[i] = -1;
  for (int i=0; i < 4; i++)
    psnr.val[i] = -1;
}

void MSE_openCl :: Init() {
  mse_cl = "mse.cl";
  setup(); // setting up context, command queue
  source_str = load_kernel(mse_cl);
  source_size_mse = source_size;
  kernel_mse = create_program("vector_mse", source_str, source_size_mse);
}

void MSE_openCl :: execute_mse (float *A, float *B, float *C, int LIST_SIZE, int LOCAL_SIZE) {

   size_t global_item_size = LIST_SIZE;
   size_t local_item_size = LOCAL_SIZE;
   cl_event event[1];
   // Create memory buffers on the device for each vector
   cl_mem a_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);
   cl_mem b_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);
   cl_mem c_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);
   if (!a_mem_obj || !b_mem_obj || !c_mem_obj )
       cout<<"Failed to allocate device memory!\n"; 

   // Copy the lists A and B to their respective memory buffers
   ret = clEnqueueWriteBuffer(command_queue, a_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(float), A, 0, NULL, NULL);
   ret = clEnqueueWriteBuffer(command_queue, b_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(float), B, 0, NULL, NULL);

   // Set the arguments of the kernel
   ret = clSetKernelArg(kernel_mse, 0, sizeof(cl_mem), (void *)&a_mem_obj);
   ret = clSetKernelArg(kernel_mse, 1, sizeof(cl_mem), (void *)&b_mem_obj);
   ret = clSetKernelArg(kernel_mse, 2, sizeof(cl_mem), (void *)&c_mem_obj);
   
   // Execute the OpenCL kernel on the list
   ret = clEnqueueNDRangeKernel(command_queue, kernel_mse, 1, NULL, &global_item_size, NULL, 0, NULL, &event[0]);
   if (ret!=CL_SUCCESS) {
     printf("Error: Kernel could not be executed\n"); 
     cout<<ret<<"\n";
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
void MSE_openCl :: clean_up_host() {
   ret = clReleaseKernel(kernel_mse);
}

CvScalar MSE_openCl :: compare(IplImage *source1, IplImage *source2, Colorspace space)
{
  IplImage *src1, *src2;
  src1 = colorspaceConversion(source1, space);
  src2 = colorspaceConversion(source2, space);
  
  int x = src1->width, y = src1->height;
  int nChan = src1->nChannels, d = IPL_DEPTH_32F;
  //size before down sampling
  CvSize size = cvSize(x, y);
  //creating diff and difference squares
  IplImage *img1 = cvCreateImage(size, d, nChan);
  IplImage *img2 = cvCreateImage(size, d, nChan);
  IplImage *mse_index = cvCreateImage(size, d, nChan);
  cvConvert(src1, img1);
	cvConvert(src2, img2);

  // calling the openCl implementation of mse
  execute_mse((float*)(img1->imageData), (float*)(img2->imageData), (float*)(mse_index->imageData), x*y*nChan, x);

  mse = cvAvg(mse_index);

  psnr.val[0] = 10.0*log10((L*L)/mse.val[0]);
  psnr.val[1] = 10.0*log10((L*L)/mse.val[1]);
  psnr.val[2] = 10.0*log10((L*L)/mse.val[2]);
  psnr.val[3] = 10.0*log10((L*L)/mse.val[3]);
  
  //Release images
  cvReleaseImage(&img1);
  cvReleaseImage(&img2);
  cvReleaseImage(&mse_index);
  cvReleaseImage(&src1);
  cvReleaseImage(&src2);

  return mse;
}

