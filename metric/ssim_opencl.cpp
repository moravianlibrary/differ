#include "core.h"
#include "similarityMetric.h"
#include "host_program_opencl.h"
#include "ssim_opencl.h"

SSIM_openCl :: SSIM_openCl()
{
  K1 = 0.01;
  K2 = 0.03;
  gaussian_window = 11;
  gaussian_sigma = 1.5;
  L = 255;
  ssim_map = NULL;
  for (int i=0; i < 4; i++)
    mssim_value.val[i] = -1; // Initialize with an out of bound value of mssim [0,1]
}

SSIM_openCl :: ~SSIM_openCl() {
  if (ssim_map != NULL)
    cvReleaseImage(&ssim_map);
  ssim_map = NULL;
}

int SSIM_openCl :: print_map()
{
  if (ssim_map == NULL)
  {
    cout<<"Error>> No Index_map_created.\n";
    return 0;
  }
  cvSave("imgSSIM.xml", ssim_map, NULL, "TESTing Index map");
  return 1;
}

void SSIM_openCl :: Init() {
  ssim1 = "ssim_part1.cl";
  ssim2 = "ssim_part2.cl";
  // setting up the context, command_queue
  setup();
  source_str_ssim1 = load_kernel(ssim1);
  source_size_ssim1 = source_size;
  kernel_ssim1 = create_program("ssim_A", source_str_ssim1, source_size_ssim1);
  source_str_ssim2 = load_kernel(ssim2);
  source_size_ssim2 = source_size;
  kernel_ssim2 = create_program("ssim_B", source_str_ssim2, source_size_ssim2);
}

void SSIM_openCl :: execute_ssim (float *src1, float *src2, float *filter, float *ssim, int LIST_SIZE, int LOCAL_SIZE, int x, int y, int nChan, int filter_size, float C1, float C2) {

   size_t global_item_size[] = {x,y};
   size_t local_item_size[] = {1,1};
   cl_event event[2];
   
   #ifdef DEBUG
   cout<<"Creating the memory buffers- LIST_SIZE ="<<LIST_SIZE<<" x="<<x<<" y="<<y<<" C1="<<C1<<" C2= "<<C2<<"\n";
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
  
   ret = clEnqueueNDRangeKernel(command_queue, kernel_ssim1, 2, NULL, global_item_size, local_item_size, 0, NULL, &event[0]);
   if (ret!=CL_SUCCESS) {
     printf("Error: Kernel could not be executed\n"); 
     cout<<ret;
   }
   clWaitForEvents(1, &event[0]);
   
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
void SSIM_openCl :: clean_up_host() {
   ret = clReleaseKernel(kernel_ssim1);
   ret = clReleaseKernel(kernel_ssim2);
}

CvScalar SSIM_openCl :: compare(IplImage *source1, IplImage *source2, Colorspace space)
{
  IplImage *src1, *src2;
  src1 = colorspaceConversion(source1, space);
  src2 = colorspaceConversion(source2, space);
  
  int x = src1->width, y = src1->height;
  int nChan = src1->nChannels, d = IPL_DEPTH_32F;
  int LIST_SIZE = x*y*nChan;
  int LOCAL_SIZE = 1;
  //size before down sampling
  CvSize size = cvSize(x, y);
  //creating diff and difference squares
  IplImage *img1 = cvCreateImage(size, d, nChan);
  IplImage *img2 = cvCreateImage(size, d, nChan);
  ssim_map = cvCreateImage(size, d, nChan);
  cvConvert(src1, img1);
	cvConvert(src2, img2);

  const float C1 = (K1 * L) * (K1 * L); 
  const float C2 = (K2 * L) * (K2 * L);

  Mat tempo = get_gaussian_filter(gaussian_window,gaussian_sigma);
  float *filter = (float*)tempo.data;

  execute_ssim((float*)(img1->imageData), (float*)(img2->imageData), filter, (float*)(ssim_map->imageData), LIST_SIZE ,LOCAL_SIZE, x, y, nChan, gaussian_window, C1, C2);

  mssim_value = cvAvg(ssim_map);

  //Release images
  cvReleaseImage(&img1);
  cvReleaseImage(&img2);
  cvReleaseImage(&src1);
  cvReleaseImage(&src2);

  return mssim_value;
}

