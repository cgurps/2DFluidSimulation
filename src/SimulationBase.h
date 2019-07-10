#ifndef SIMULATIONBASE_H
#define SIMULATIONBASE_H

#include "CLUtils.h"
#include "OpenCLWrapper.h"
#include "GLFWHandler.h"

class SimulationBase
{
  public:
    SimulationBase() = delete;
    SimulationBase(OpenCLWrapper* ocl)
      : ocl(ocl) 
    {
      try
      {
        GL_CHECK( glGenTextures(1, &shared_texture); );
        GL_CHECK( glBindTexture(GL_TEXTURE_2D, shared_texture); );
        GL_CHECK( glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, ocl->width, ocl->height, 0, GL_RGBA, GL_FLOAT, NULL); );
        GL_CHECK( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); );
        GL_CHECK( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); );
        GL_CHECK( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); );
        GL_CHECK( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); );

        imageGL = cl::ImageGL(ocl->context, CL_MEM_WRITE_ONLY, GL_TEXTURE_2D, 0, shared_texture, NULL);
      }
      catch(cl::Error& e)
      {
        cl_ok(e.err());
      }

    }

    virtual void Init() = 0;
    virtual void Update() = 0;

    OpenCLWrapper* ocl;

    GLuint shared_texture;
    cl::ImageGL imageGL;
};

#endif //SIMULATIONBASE_H
