#include "SimpleFluid.h"
#include "GLFWHandler.h"

#include <string>
#include <sstream>
#include <fstream>

void SimpleFluid::Init()
{
  std::ifstream file("kernels/fluid.cs"); 
  std::ostringstream file_buffer;
  file_buffer << file.rdbuf();

  program = cl::Program(ocl->context, file_buffer.str());
  try
  {
    program.build({ocl->device});
  }
  catch (cl::Error& e)
  {
    if (e.err() == CL_BUILD_PROGRAM_FAILURE)
    {
      for (cl::Device dev : {ocl->device})
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

void SimpleFluid::Update()
{

}
