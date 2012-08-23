#include "core.h"
#include "similarityMetric.h"
#include "host_program_opencl.h"
#include "iqi_opencl.h"

ImageQuI_openCl :: ImageQuI_openCl()
{
  B = 8;
  image_quality_map = NULL;
  for (int i=0; i < 4; i++)
    image_quality_value.val[i] = -2;   // Initialize with an out of bound value for image_quality_value [-1.1]
}

ImageQuI_openCl :: ~ImageQuI_openCl()
{
  if (image_quality_map != NULL)
    cvReleaseImage(&image_quality_map);
}

void ImageQuI_openCl :: releaseImageQuality_map() {
  if (image_quality_map != NULL)
    cvReleaseImage(&image_quality_map);
}

int ImageQuI_openCl :: print_map()
{
  if (image_quality_map == NULL)
  {
    cout<<"Error>> No Index_map_created.\n";
    return 0;
  }
  cvSave("imgQI.xml", image_quality_map, NULL, "TESTing Index map");
  return 1;
}

void ImageQuI_openCl :: Init() {
  
  iqi1 = "img_qi1.cl";
  iqi2 = "img_qi2.cl";
  setup(); // setting up the context, command queue
  source_str_iqi1 = load_kernel(iqi1);
  source_size_iqi1 = source_size;
  kernel_iqi1 = create_program("img_qi_A", source_str_iqi1, source_size_iqi1);
  source_str_iqi2 = load_kernel(iqi2);
  source_size_iqi2 = source_size;
  kernel_iqi2 = create_program("img_qi_B", source_str_iqi2, source_size_iqi2);
}

void ImageQuI_openCl :: execute_iqi (float *src1, float *src2, float *iqi, int LIST_SIZE, int LOCAL_SIZE, int x, int y, int nChan, int filter_size) {

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
 
   // check if memory allocation is successful
   if (!src1_mem_obj || !src2_mem_obj || !img1_sq_mem_obj || !img2_sq_mem_obj || !img1_img2_mem_obj || 
       !mu1_mem_obj || !mu2_mem_obj || !iqi_index_mem_obj)
       cout<<"Failed to allocate device memory!\n"; 
    
   #ifdef DEBUG
   cout<<"Allocating memory buffers - \n"; 
   #endif
   
   // Copy the lists Image Data src1, src2 to their respective memory buffers
   ret = clEnqueueWriteBuffer(command_queue, src1_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(float), src1, 0, NULL, NULL);
   if (ret != CL_SUCCESS)
      printf("Error: Failed to write to source array!\n");
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
   ret = clEnqueueNDRangeKernel(command_queue, kernel_iqi1, 2, NULL, global_item_size, local_item_size, 0, NULL, &event[0]);
   if (ret!=CL_SUCCESS) {
     printf("Error: Kernel could not be executed\n"); 
     cout<<ret;
   }
   clWaitForEvents(1, &event[0]);
   
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
   
   // Read the memory buffer iqi_index_mem_obj on the device to the local variable iqi
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
void ImageQuI_openCl :: clean_up_host() {
   ret = clReleaseKernel(kernel_iqi1);
   ret = clReleaseKernel(kernel_iqi2);
}

CvScalar ImageQuI_openCl :: compare(IplImage *source1, IplImage *source2, Colorspace space)
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
  image_quality_map = cvCreateImage(size, d, nChan);
  cvConvert(src1, img1);
	cvConvert(src2, img2);

  // calling the openCl implementation
  execute_iqi((float*)(img1->imageData), (float*)(img2->imageData), (float*)(image_quality_map->imageData),x*y*nChan,x,x,y,nChan,B);

  image_quality_value = cvAvg(image_quality_map);

  //Release images
  cvReleaseImage(&img1);
  cvReleaseImage(&img2);
  cvReleaseImage(&src1);
  cvReleaseImage(&src2);

  // returning the final value
  return image_quality_value;
}

