#include "SimpleFluid.h"
#include "GLFWHandler.h"

#include <string>
#include <sstream>
#include <fstream>

cl::Image2D create2DImage(OpenCLWrapper *ocl)
{
  return cl::Image2D(ocl->context,
      CL_MEM_READ_WRITE,
      cl::ImageFormat(CL_RGBA, CL_FLOAT),
      ocl->width,
      ocl->height
      );
}

cl::Buffer createBuffer(OpenCLWrapper *ocl)
{
  return cl::Buffer(ocl->context,
      CL_MEM_READ_WRITE,
      ocl->width * ocl->height);
}

void SimpleFluid::Init()
{
  std::ifstream file("kernels/fluid.cs"); 
  std::ostringstream file_buffer;
  file_buffer << file.rdbuf();

  try
  {
    //Loading Program
    program = cl::Program(ocl->context, file_buffer.str());
    program.build({ocl->device});

    //Generating Buffers and Images
    velocitiesBuffer[READ] = create2DImage(ocl); 
    velocitiesBuffer[WRITE] = create2DImage(ocl); 

    pressureBuffer[READ] = create2DImage(ocl);
    pressureBuffer[WRITE] = create2DImage(ocl);

    divergenceBuffer = createBuffer(ocl);

    dummyBuffer = createBuffer(ocl);

    //Preparing Kernels
    advectKernel = cl::Kernel(program, "advect");
    divergenceKernel = cl::Kernel(program, "divergence");
    jacobiKernel = cl::Kernel(program, "jacobi");
    pressureProjectionKernel = cl::Kernel(program, "pressureProjection");

    //OpenGL Texture write
    writeToTextureKernel = cl::Kernel(program, "writeToTexture");
    writeToTextureKernel.setArg(0, imageGL);
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
      throw cl_ok(e.err());
    }
  }
}

void SimpleFluid::Update()
{
  auto f = [](const unsigned a, const unsigned b) -> unsigned
  {
    return (a + b - 1) / b;
  };

  try
  {
    cl::Event event;

    std::vector<cl::Memory> objects(1, imageGL);

    glFinish();

    ocl->command_queue.enqueueAcquireGLObjects(&objects, NULL, &event); event.wait();

    cl::NDRange local(16, 16);
    cl::NDRange global(local[0] * f(ocl->width , local[0]),
                       local[1] * f(ocl->height, local[1]));

    ocl->command_queue.enqueueNDRangeKernel(writeToTextureKernel, cl::NullRange, global, local);

    ocl->command_queue.enqueueReleaseGLObjects(&objects, NULL, &event); event.wait();

    ocl->command_queue.finish();
  }
  catch(cl::Error& e)
  {
    cl_ok(e.err());
  }
}

void SimpleFluid::UpdateSimulation()
{
  advectKernel.setArg(0, velocitiesBuffer[READ]);
  advectKernel.setArg(1, velocitiesBuffer[READ]);
  advectKernel.setArg(2, velocitiesBuffer[WRITE]);
  advectKernel.setArg(3, 0.1f);
}
