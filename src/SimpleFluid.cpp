#include "SimpleFluid.h"
#include "GLFWHandler.h"

#include <string>
#include <sstream>
#include <fstream>

cl::Image2D create2DImage(OpenCLWrapper *ocl)
{
  try
  {
    cl::Image2D image = cl::Image2D(ocl->context,
        CL_MEM_READ_WRITE,
        cl::ImageFormat(CL_RGBA, CL_FLOAT),
        ocl->width, ocl->height);

    cl_float4 emp; emp.x = 0.0f; emp.y = 0.0f; emp.z = 0.0f; emp.w = 0.0f;
    cl::array<cl::size_type, 3> origin = {0, 0, 0};
    cl::array<cl::size_type, 3> region = {(unsigned long) ocl->width, (unsigned long) ocl->height, 1};
    ocl->command_queue.enqueueFillImage(image, emp, origin, region);

    return image;
  }
  catch(cl::Error& e)
  {
    throw e;
  }
}

void writeChestToImage(OpenCLWrapper *ocl, cl::Image2D *image)
{
  try
  {
    std::vector<float> pixels(4 * ocl->width * ocl->height);
    for(int y = 0; y < ocl->height; ++y)
    {
      for(int x = 0; x < ocl->width; ++x)
      {
        if(x % 50 > 25 && y % 50 > 25)
        {
          pixels[4 * (y * ocl->width + x)] = 1.0f;
          pixels[4 * (y * ocl->width + x) + 1] = 1.0f;
          pixels[4 * (y * ocl->width + x) + 2] = 1.0f;
        }
        else
        {
          pixels[4 * (y * ocl->width + x)] = 0.0f;
          pixels[4 * (y * ocl->width + x) + 1] = 0.0f;
          pixels[4 * (y * ocl->width + x) + 2] = 0.0f;
        }
        pixels[4 * (y * ocl->width + x) + 3] = 1.0f;
      }
    }

    cl::array<cl::size_type, 3> origin = {0, 0, 0};
    cl::array<cl::size_type, 3> region = {(unsigned long) ocl->width, (unsigned long) ocl->height, 1};
    ocl->command_queue.enqueueWriteImage(*image, CL_TRUE, origin, region, 0, 0, pixels.data());
  }
  catch(cl::Error& e)
  {
    throw e;
  }
}

void writeVelocities(OpenCLWrapper *ocl, cl::Image2D *image)
{
  try
  {
    std::vector<float> pixels(4 * ocl->width * ocl->height);
    for(int y = 0; y < ocl->height; ++y)
    {
      for(int x = 0; x < ocl->width; ++x)
      {
        pixels[4 * (y * ocl->width + x)    ] = 1.0f;
        pixels[4 * (y * ocl->width + x) + 1] = 0.0f;
        pixels[4 * (y * ocl->width + x) + 2] = 0.0f;
        pixels[4 * (y * ocl->width + x) + 3] = 0.0f;
      }
    }

    cl::array<cl::size_type, 3> origin = {0, 0, 0};
    cl::array<cl::size_type, 3> region = {(unsigned long) ocl->width, (unsigned long) ocl->height, 1};
    ocl->command_queue.enqueueWriteImage(*image, CL_TRUE, origin, region, 0, 0, pixels.data());
  }
  catch(cl::Error& e)
  {
    throw e;
  }
}

template<typename T>
cl::Buffer createBuffer(OpenCLWrapper *ocl, const size_t size, const T value)
{
  try
  {
    std::vector<T> dummy(size, value);
    return cl::Buffer(ocl->context,
        dummy.begin(), dummy.end(), false);
  }
  catch(cl::Error& e)
  {
    throw e;
  }
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

    std::cout << "Successfully built the OpenCL kernels" << std::endl;

    //Generating Buffers and Images
    velocitiesBuffer[READ] = create2DImage(ocl); 
    velocitiesBuffer[WRITE] = create2DImage(ocl); 
    writeVelocities(ocl, velocitiesBuffer);

    pressureBuffer[READ] = create2DImage(ocl);
    pressureBuffer[WRITE] = create2DImage(ocl);

    fieldBuffer[READ] = create2DImage(ocl);
    fieldBuffer[WRITE] = create2DImage(ocl);
    writeChestToImage(ocl, fieldBuffer);

    emptyBuffer = create2DImage(ocl);

    divergenceBuffer = createBuffer(ocl, ocl->width * ocl->height, 0.0f);

    deltaTimeBuffer = createBuffer(ocl, 1, 0.1f);

    std::cout << "Buffers Initialized" << std::endl;

    //Preparing Kernels
    advectKernel = cl::Kernel(program, "advect");
    divergenceKernel = cl::Kernel(program, "divergence");
    jacobiKernel = cl::Kernel(program, "jacobi");
    pressureProjectionKernel = cl::Kernel(program, "pressureProjection");
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

    cl::NDRange local(16, 16);
    cl::NDRange global(local[0] * f(ocl->width , local[0]),
                       local[1] * f(ocl->height, local[1]));

    /********** ADVECTION KERNEL **********/
    advectKernel.setArg(0, velocitiesBuffer[READ]);
    advectKernel.setArg(1, velocitiesBuffer[READ]);
    advectKernel.setArg(2, velocitiesBuffer[WRITE]);
    advectKernel.setArg(3, deltaTimeBuffer);

    ocl->command_queue.enqueueNDRangeKernel(advectKernel, cl::NullRange, global, local);
    ocl->command_queue.finish();

    std::swap(velocitiesBuffer[READ], velocitiesBuffer[WRITE]);

    /********** DIVERGENCE KERNEL **********/
    divergenceKernel.setArg(0, velocitiesBuffer[READ]);
    divergenceKernel.setArg(1, divergenceBuffer);
    divergenceKernel.setArg(2, deltaTimeBuffer);

    ocl->command_queue.enqueueNDRangeKernel(divergenceKernel, cl::NullRange, global, local);
    ocl->command_queue.finish();

    /********** JACOBI KERNEL **********/
    jacobiKernel.setArg(0, divergenceBuffer);
    jacobiKernel.setArg(3, deltaTimeBuffer);

    //Initialization
    jacobiKernel.setArg(1, emptyBuffer);
    jacobiKernel.setArg(2, pressureBuffer[READ]);

    ocl->command_queue.enqueueNDRangeKernel(jacobiKernel, cl::NullRange, global, local);
    ocl->command_queue.finish();

    //Solving
    for(int k = 0; k < 45; ++k)
    {
      jacobiKernel.setArg(1, pressureBuffer[READ]);
      jacobiKernel.setArg(2, pressureBuffer[WRITE]);

      ocl->command_queue.enqueueNDRangeKernel(jacobiKernel, cl::NullRange, global, local);
      ocl->command_queue.finish();

      std::swap(pressureBuffer[READ], pressureBuffer[WRITE]);
    }

    /********** PRESSURE PROJECTION **********/
    pressureProjectionKernel.setArg(0, pressureBuffer[READ]);
    pressureProjectionKernel.setArg(1, velocitiesBuffer[READ]);
    pressureProjectionKernel.setArg(2, velocitiesBuffer[WRITE]);
    pressureProjectionKernel.setArg(3, deltaTimeBuffer);

    ocl->command_queue.enqueueNDRangeKernel(pressureProjectionKernel, cl::NullRange, global, local);
    ocl->command_queue.finish();
    
    std::swap(velocitiesBuffer[READ], velocitiesBuffer[WRITE]);

    /********** FIELD ADVECTION KERNEL **********/
    advectKernel.setArg(0, velocitiesBuffer[READ]);
    advectKernel.setArg(1, fieldBuffer[READ]);
    advectKernel.setArg(2, fieldBuffer[WRITE]);
    advectKernel.setArg(3, deltaTimeBuffer);

    ocl->command_queue.enqueueNDRangeKernel(advectKernel, cl::NullRange, global, local);
    ocl->command_queue.finish();

    std::swap(fieldBuffer[READ], fieldBuffer[WRITE]);

    std::vector<cl::Memory> objects(1, imageGL); glFinish();
    ocl->command_queue.enqueueAcquireGLObjects(&objects, NULL, &event); event.wait();
    ocl->command_queue.enqueueCopyImage(fieldBuffer[READ], imageGL, 
        {0, 0, 0}, {0, 0, 0}, {(unsigned long) ocl->width, (unsigned long)ocl->height, 1});
    ocl->command_queue.finish();
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

}
