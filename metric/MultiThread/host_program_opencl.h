#ifndef _host_program_opencl_h
#define _host_program_opencl_h

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
 
      host_program_openCl() {

      }

      void print_Device_Info();
      
      char * load_kernel (const char * kernel_name);

      void setup ();

      cl_kernel create_program (const char *kernel_function_name, const char *source_str, const size_t source_size);
    
      // Clean up
      ~host_program_openCl();

};

#endif
