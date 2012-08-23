#include "core.h"
#include "host_program_opencl.h"

void host_program_openCl :: print_Device_Info() {

  size_t max_wrkgrp_size; 
  platform_id = NULL;
  device_id = NULL;
 	int err;
	cl_uint platforms;
	cl_platform_id platform = NULL;
	char cBuffer[1024];

	err = clGetPlatformIDs( 1, &platform, &platforms );
	if (err != CL_SUCCESS)
		printf("Error in OpenCL call!\n");
  else {
    #ifdef DEBUG
	  printf("Number of platforms: %d\n", platforms);
    #endif
  }
      
	err = clGetPlatformInfo( platform, CL_PLATFORM_NAME, sizeof(cBuffer), cBuffer, NULL );
	if (err != CL_SUCCESS)
		printf("Error in OpenCL call!\n");
  else {
    #ifdef DEBUG
    printf("CL_PLATFORM_NAME :\t %s\n", cBuffer);
    #endif
  }

	err = clGetPlatformInfo( platform, CL_PLATFORM_VERSION, sizeof(cBuffer), cBuffer, NULL );
	if (err != CL_SUCCESS)
		printf("Error in OpenCL call!\n");
  else {
    #ifdef DEBUG
	  printf("CL_PLATFORM_VERSION :\t %s\n", cBuffer);
    #endif
  }

  // Get platform and device information
  ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
  ret = clGetDeviceIDs( platform_id, CL_DEVICE_TYPE_ALL, 1, &device_id, &ret_num_devices);
  ret = clGetDeviceInfo( device_id, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t),  &max_wrkgrp_size, NULL);
  if (ret != CL_SUCCESS)
    cout<<"Error: Failed to get device Info (CL_DEVICE_MAX_WORK_GROUP_SIZE)!\n";
  else {
    #ifdef DEBUG
    cout<<"Max work group size = "<<(int)max_wrkgrp_size <<"\n";
    #endif
  }
}
    
char * host_program_openCl :: load_kernel (const char * kernel_name) {
   // Load the kernel source code into the array source_str
   FILE *fp;
   char *source_str;
   #ifdef DEBUG
   printf("%s\n",kernel_name); 
   #endif
   fp = fopen(kernel_name, "r");
   if (!fp) {
       fprintf(stderr, "Failed to load kernel.\n");
   }
   source_str = (char*)malloc(MAX_SOURCE_SIZE);
   source_size = fread( source_str, 1, MAX_SOURCE_SIZE, fp);
   fclose( fp );
   return source_str;
}
 
void host_program_openCl :: setup () {
   
   platform_id = NULL;
   device_id = NULL;
   
   // Get platform and device information
   ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
   ret = clGetDeviceIDs( platform_id, CL_DEVICE_TYPE_ALL, 1, &device_id, &ret_num_devices);
  
   // Create an OpenCL context
   context = clCreateContext( NULL, 1, &device_id, NULL, NULL, &ret);
  
   // Create a command queue
   command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
   if ( ret != CL_SUCCESS ) 
     printf( "Error : Cannot create command queue.\n" ); 
}


cl_kernel host_program_openCl :: create_program (const char *kernel_function_name, const char *source_str, const size_t source_size) {
 
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
host_program_openCl :: ~host_program_openCl() {
   ret = clFlush(command_queue);
   ret = clFinish(command_queue);
   ret = clReleaseProgram(program);
   ret = clReleaseCommandQueue(command_queue);
   ret = clReleaseContext(context);
}

