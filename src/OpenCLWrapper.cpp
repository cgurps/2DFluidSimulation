#include "OpenCLWrapper.h"

#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

OpenCLWrapper::OpenCLWrapper(int width, int height, GLFWHandler* glfwHandler)
  : width(width), height(height)
{

  try
  {
    std::vector<cl::Platform> all_platforms;
    cl::Platform::get(&all_platforms);

    cl::Platform default_platform = all_platforms[0];
    std::cout << "OpenCL Infos:" << std::endl;
    std::cout << "  Profile: " << default_platform.getInfo<CL_PLATFORM_PROFILE>() << std::endl;
    std::cout << "  Version: " << default_platform.getInfo<CL_PLATFORM_VERSION>() << std::endl;
    std::cout << "  Platform: " << default_platform.getInfo<CL_PLATFORM_NAME>() << std::endl;
    std::cout << "  Vendor: " << default_platform.getInfo<CL_PLATFORM_VENDOR>() << std::endl;

    // get default device (CPUs, GPUs) of the default platform
    std::vector<cl::Device> all_devices;
    default_platform.getDevices(CL_DEVICE_TYPE_ALL, &all_devices);

    std::cout << "There is " << all_devices.size() << " device(s)." << std::endl;

    device = (all_devices.size() == 1) ? all_devices[0] : all_devices[1];
    std::cout << "Device Infos:" << std::endl;
    std::cout << "  Name: " << device.getInfo<CL_DEVICE_NAME>() << std::endl;
    std::cout << "  Version: " << device.getInfo<CL_DEVICE_VERSION>() << std::endl;
    std::cout << "  Max Workgroup Size: " << device.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>() << std::endl;

    std::string extensions = device.getInfo<CL_DEVICE_EXTENSIONS>();
    std::cout << "  Extensions: " << extensions << std::endl;

#ifdef __APPLE__
    std::string CL_GL_SHARING_EXT = "cl_APPLE_gl_sharing";
#else
    std::string CL_GL_SHARING_EXT = "cl_khr_gl_sharing";
#endif

    if(extensions.find(CL_GL_SHARING_EXT) != std::string::npos)
      std::cout << "Found CL GL extension." << std::endl;
    else
    {
      std::cout << std::endl << "Your OpenCL doesn't support CL GL Sharing..." << std::endl;
      exit(1);
    }

#ifdef __APPLE__
    CGLContextObj kCGLContext = CGLGetCurrentContext();
    CGLShareGroupObj kCGLShareGroup = CGLGetShareGroup(kCGLContext);
    cl_context_properties properties[] = 
    {
      CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE,
      (cl_context_properties) kCGLShareGroup,
      0
    };
#else
    cl_context_properties properties[] =
    {
      CL_GL_CONTEXT_KHR, (cl_context_properties) glfwGetGLXContext(glfwHandler->window),
      CL_GLX_DISPLAY_KHR, (cl_context_properties) glfwGetX11Display(),
      CL_CONTEXT_PLATFORM, (cl_context_properties) default_platform(),
      0
    };
#endif

    context = cl::Context({device}, properties, NULL, NULL, NULL);

    std::cout << "Context created." << std::endl;

    command_queue = cl::CommandQueue(context, device, 0, NULL);
  }
  catch(cl::Error& e)
  {
    cl_ok(e.err());
  }
}

/*
   void Simulation::InitBuffers()
   {
   cl_int err;

   std::vector<float> empty2(2 * width * height, 0.0f);
   fluid_velocities[0] = cl::Buffer(context, empty2.begin(), empty2.end(), false, true, &err);   cl_ok(err);
   fluid_velocities[1] = cl::Buffer(context, empty2.begin(), empty2.end(), false, true, &err);   cl_ok(err);

   std::vector<float> empty(width * height, 0.0f);
   divergence          = cl::Buffer(context, empty.begin(), empty.end(), false, true, &err);   cl_ok(err);
   pressure            = cl::Buffer(context, empty.begin(), empty.end(), false, true, &err);   cl_ok(err);
   }

   void Simulation::InitProgram()
   {
   std::ifstream file("kernels/fluid.cs"); 
   std::ostringstream file_buffer;
   file_buffer << file.rdbuf();

   program = cl::Program(context, file_buffer.str());
   try
   {
   program.build({device});
   }
   catch (cl::Error& e)
   {
   if (e.err() == CL_BUILD_PROGRAM_FAILURE)
   {
   for (cl::Device dev : {device})
   {
// Check the build status
cl_build_status status = program.getBuildInfo<CL_PROGRAM_BUILD_STATUS>(dev);
if (status != CL_BUILD_ERROR)
continue;

// Get the build log
std::string name     = dev.getInfo<CL_DEVICE_NAME>();
std::string buildlog = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(dev);
std::cerr << "Build log for " << name << ":" << std::endl
<< buildlog << std::endl;
}
}
else
{
throw e;
}
}
}
*/
