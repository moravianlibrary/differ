#include "core.h"
#include "similarityMetric.h"
#include "host_program_opencl.h"
#include "msssim_opencl.h"

MS_SSIM_openCl :: MS_SSIM_openCl()
{
  K1 = 0.01;
  K2 = 0.03;
  gaussian_window = 11;
  gaussian_sigma = 1.5;
  level = 5;
  L = 255;
  ms_ssim_map = NULL;
  for (int i=0; i < 4; i++)
    ms_ssim_value.val[i] = -1; // Initialize with an out of bound value of mssim [0,1]

  float alpha_t[5] = {0.0, 0.0, 0.0, 0.0, 0.1333};
  float beta_t[5] = {0.0448, 0.2856, 0.3001, 0.2363, 0.1333};
  float gamma_t[5] = {0.0448, 0.2856, 0.3001, 0.2363, 0.1333};
  alpha = new float[level];
  beta = new float[level];
  gamma = new float[level];
  memcpy(alpha, alpha_t, sizeof(alpha_t));
  memcpy(beta, beta_t, sizeof(beta_t));
  memcpy(gamma, gamma_t, sizeof(gamma_t));
}

MS_SSIM_openCl :: ~MS_SSIM_openCl()
{
  delete[] alpha;
  delete[] beta;
  delete[] gamma;
  int i;
  for (i=0; i < level; i++) {
    if (ms_ssim_map[i] != NULL)
      cvReleaseImage(&ms_ssim_map[i]);
  }
  if (ms_ssim_map !=NULL) 
    free(ms_ssim_map);
}

void MS_SSIM_openCl :: releaseMSSSIM_map() {
  int i;
  for (i=0; i < level; i++) {
    if (ms_ssim_map[i] != NULL)
      cvReleaseImage(&ms_ssim_map[i]);
  }
  if (ms_ssim_map != NULL)
    free(ms_ssim_map);
}

// Prints all index maps of all the levels into different xml files
int MS_SSIM_openCl :: print_map()
{
  if (ms_ssim_map == NULL)
  {
    cout<<"Error>> No Index_map_created.\n";
    return 0;
  }
  char file_name[50];
  // Printing the MS-SSIM_Map
  for (int i=0; i < level; i++)
  {
    sprintf(file_name, "img_MS-SSIM_map_level_%d.xml", i);
    cvSave(file_name, ms_ssim_map[i], NULL, "Testing MS-SSIM Index map");
  }
  return 1;
}

void MS_SSIM_openCl :: Init() {
  
  ms_ssim1 = "ms_ssim_part1.cl";
  ms_ssim2 = "ms_ssim_part2.cl";
  setup();
  source_str_ms_ssim1 = load_kernel(ms_ssim1);
  source_size_ms_ssim1 = source_size;
  kernel_ms_ssim1 = create_program("ms_ssim_A", source_str_ms_ssim1, source_size_ms_ssim1);
  source_str_ms_ssim2 = load_kernel(ms_ssim2);
  source_size_ms_ssim2 = source_size;
  kernel_ms_ssim2 = create_program("ms_ssim_B", source_str_ms_ssim2, source_size_ms_ssim2);
}

void MS_SSIM_openCl :: execute_ssim_temp (float *src1, float *src2, float *filter, float *ms_ssim, float *cs_map, int LIST_SIZE, int LOCAL_SIZE, int x, int y, int nChan, int filter_size, float C1, float C2) {

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
   cl_mem ms_ssim_index_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);
   cl_mem cs_index_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY, LIST_SIZE * sizeof(float), NULL, &ret);
 
   if (!src1_mem_obj || !src2_mem_obj || !filter_mem_obj || !img1_sq_mem_obj || !img2_sq_mem_obj || !img1_img2_mem_obj || 
       !mu1_mem_obj || !mu2_mem_obj || !ms_ssim_index_mem_obj || !cs_index_mem_obj)
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
   ret = clEnqueueWriteBuffer(command_queue, filter_mem_obj, CL_TRUE, 0, filter_size*filter_size* sizeof(float), filter, 0, NULL, NULL);
   if (ret != CL_SUCCESS)
      printf("Error: Failed to write to source array!\n");
    
   #ifdef DEBUG
   cout<<"Setting arguments of kernel1 ms_ssim- \n"; 
   #endif
   // Set the arguments of the kernel ms_ssim1
   ret = clSetKernelArg(kernel_ms_ssim1, 0, sizeof(cl_mem), (void *)&src1_mem_obj);
   ret = clSetKernelArg(kernel_ms_ssim1, 1, sizeof(cl_mem), (void *)&src2_mem_obj);
   ret = clSetKernelArg(kernel_ms_ssim1, 2, sizeof(cl_mem), (void *)&filter_mem_obj   );
   ret = clSetKernelArg(kernel_ms_ssim1, 3, sizeof(cl_mem), (void *)&img1_sq_mem_obj  );
   ret = clSetKernelArg(kernel_ms_ssim1, 4, sizeof(cl_mem), (void *)&img2_sq_mem_obj  );
   ret = clSetKernelArg(kernel_ms_ssim1, 5, sizeof(cl_mem), (void *)&img1_img2_mem_obj);
   ret = clSetKernelArg(kernel_ms_ssim1, 6, sizeof(cl_mem), (void *)&mu1_mem_obj);
   ret = clSetKernelArg(kernel_ms_ssim1, 7, sizeof(cl_mem), (void *)&mu2_mem_obj);
   ret = clSetKernelArg(kernel_ms_ssim1, 8, sizeof(int), &x);
   ret = clSetKernelArg(kernel_ms_ssim1, 9, sizeof(int), &y);
   ret = clSetKernelArg(kernel_ms_ssim1, 10, sizeof(int), &nChan);
   ret = clSetKernelArg(kernel_ms_ssim1, 11, sizeof(int), &filter_size);
   
   // Execute the OpenCL kernel on the list
   #ifdef DEBUG
   cout<<"Executing ms_ssim kernel1 - \n";
   #endif
   // executing hte kernel
   ret = clEnqueueNDRangeKernel(command_queue, kernel_ms_ssim1, 2, NULL, global_item_size, local_item_size, 0, NULL, &event[0]);
   if (ret!=CL_SUCCESS) {
     printf("Error: Kernel could not be executed\n"); 
     cout<<ret;
   }
   clWaitForEvents(1, &event[0]);
   
   #ifdef DEBUG
   cout<<"Setting arguments of kernel2 ms_ssim- \n"; 
   #endif
   // Set the arguments of the kernel ms_ssim2
   ret = clSetKernelArg(kernel_ms_ssim2, 0, sizeof(cl_mem), (void *)&mu1_mem_obj);
   ret = clSetKernelArg(kernel_ms_ssim2, 1, sizeof(cl_mem), (void *)&mu2_mem_obj);
   ret = clSetKernelArg(kernel_ms_ssim2, 2, sizeof(cl_mem), (void *)&filter_mem_obj   );
   ret = clSetKernelArg(kernel_ms_ssim2, 3, sizeof(cl_mem), (void *)&img1_sq_mem_obj  );
   ret = clSetKernelArg(kernel_ms_ssim2, 4, sizeof(cl_mem), (void *)&img2_sq_mem_obj  );
   ret = clSetKernelArg(kernel_ms_ssim2, 5, sizeof(cl_mem), (void *)&img1_img2_mem_obj);
   ret = clSetKernelArg(kernel_ms_ssim2, 6, sizeof(cl_mem), (void *)&ms_ssim_index_mem_obj);
   ret = clSetKernelArg(kernel_ms_ssim2, 7, sizeof(cl_mem), (void *)&cs_index_mem_obj);
   ret = clSetKernelArg(kernel_ms_ssim2, 8, sizeof(int), &x);
   ret = clSetKernelArg(kernel_ms_ssim2, 9, sizeof(int), &y);
   ret = clSetKernelArg(kernel_ms_ssim2, 10, sizeof(int), &nChan);
   ret = clSetKernelArg(kernel_ms_ssim2, 11, sizeof(int), &filter_size);
   ret = clSetKernelArg(kernel_ms_ssim2, 12, sizeof(float), &C1);
   ret = clSetKernelArg(kernel_ms_ssim2, 13, sizeof(float), &C2);

   #ifdef DEBUG
   cout<<"Executing ms_ssim kernel2 - \n";
   #endif
   // Execute the OpenCL kernel on the list
   ret = clEnqueueNDRangeKernel(command_queue, kernel_ms_ssim2, 2, NULL, global_item_size, local_item_size, 0, NULL, &event[1]);
   clWaitForEvents(1, &event[1]);
   
   // Read the memory buffers ms_ssim_index_mem_obj and cs_index_mem_obj on the device to the local variable ms_ssim and cs_map
   ret = clEnqueueReadBuffer(command_queue, ms_ssim_index_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(float), ms_ssim, 0, NULL, NULL);
   ret = clEnqueueReadBuffer(command_queue, cs_index_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(float), cs_map, 0, NULL, NULL);

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
   ret = clReleaseMemObject(ms_ssim_index_mem_obj);
   ret = clReleaseMemObject(cs_index_mem_obj);
}

// Clean up
void MS_SSIM_openCl :: clean_up_host() {
   ret = clReleaseKernel(kernel_ms_ssim1);
   ret = clReleaseKernel(kernel_ms_ssim2);
}

CvScalar MS_SSIM_openCl :: execute_ms_ssim (IplImage *src1, IplImage *src2, float *filter, int LIST_SIZE, int LOCAL_SIZE, int x, int y, int nChan, int filter_size, float C1, float C2, int level) {

  CvScalar mssim_t;
  CvScalar mcs_t;
  ms_ssim_map = (IplImage**)(malloc(sizeof(IplImage*)*level));
  int d = IPL_DEPTH_32F;
  #ifdef DEBUG
  cout<<"\nBeta = "<<beta[0]<<" "<<beta[1]<<" "<<beta[2]<<" "<<beta[3]<<"\n";
  #endif

  for (int i=0; i<level; i++)
  {
    //creating a cs_map image
    IplImage *cs_map;
    //Downsampling of the original images
    IplImage *downsampleSrc1, *downsampleSrc2;
    //Downsampling the images
    CvSize downs_size = cvSize((int)(x/pow(2, i)), (int)(y/pow(2, i)));
    downsampleSrc1 = cvCreateImage(downs_size, d, nChan);
    downsampleSrc2 = cvCreateImage(downs_size, d, nChan);
    cvResize(src1, downsampleSrc1, CV_INTER_NN);
    cvResize(src2, downsampleSrc2, CV_INTER_NN);
    ms_ssim_map[i] = cvCreateImage(downs_size, d, nChan);
    cs_map = cvCreateImage(downs_size, d, nChan);
    CvScalar test_avg = cvAvg(downsampleSrc1);
    #ifdef DEBUG
    cout<<"Checking for average value - "<<test_avg.val[0]<<" "<<test_avg.val[1]<<" "<<test_avg.val[2]<<"\n";
    #endif

    // Downsampled height and width
    int xds = downsampleSrc1->width;
    int yds = downsampleSrc1->height; 
    LIST_SIZE = xds*yds*nChan;
    LOCAL_SIZE = 1 ;
    #ifdef DEBUG
    cout<<"Values at level="<<i<<" \n";
    #endif 

    execute_ssim_temp((float*)(downsampleSrc1->imageData), (float*)(downsampleSrc2->imageData), filter, (float*)(ms_ssim_map[i]->imageData), 
        (float*)(cs_map->imageData), LIST_SIZE, LOCAL_SIZE, xds, yds, nChan, gaussian_window, C1, C2);

    mssim_t = cvAvg(ms_ssim_map[i]);
    mcs_t = cvAvg(cs_map);
    
    #ifdef DEBUG
    cout<<"Size of MAP at level = "<<i<<" size = "<<ms_ssim_map[i]->width<<" "<<ms_ssim_map[i]->height<<"\n";
    cout<<"Some values for testing - "<<mssim_t.val[0]<<" "<<mssim_t.val[1]<<" "<<mssim_t.val[2]<<"\n";
    #endif

    // calculating the mean ms_sssim value using the weighted average as defined
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
    cvReleaseImage(&cs_map);
  }
  return ms_ssim_value;

}

CvScalar MS_SSIM_openCl :: compare(IplImage *source1, IplImage *source2, Colorspace space)
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
  cvConvert(src1, img1);
	cvConvert(src2, img2);

  const float C1 = (K1 * L) * (K1 * L); 
  const float C2 = (K2 * L) * (K2 * L);

  Mat tempo = get_gaussian_filter(gaussian_window, gaussian_sigma);
  float *filter = (float*)tempo.data;

  // calling kernel implementation
  ms_ssim_value  = execute_ms_ssim(img1, img2, filter, x*y*nChan, x, x, y, nChan, gaussian_window, C1, C2, level);

  //Release images
  cvReleaseImage(&img1);
  cvReleaseImage(&img2);
  cvReleaseImage(&src1);
  cvReleaseImage(&src2);

  return ms_ssim_value;
}

