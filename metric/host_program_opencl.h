#ifndef _host_program_opencl_h
#define _host_program_opencl_h

// host_program_openCl
// contains the basic function implementation like loading kernel, creating command queue, create program etc.
// these functions are used for all similarity metrics implementations
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

      // Prints the device information 
      void print_Device_Info();
      
      // loads the kernel from a source file
      char * load_kernel (const char * kernel_name);

      // create context and command queue
      void setup ();

      // created and builds the program
      cl_kernel create_program (const char *kernel_function_name, const char *source_str, const size_t source_size);
    
      // Clean up
      ~host_program_openCl();

};

#endif
