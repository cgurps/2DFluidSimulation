#ifndef SIMPLEFLUID_H
#define SIMPLEFLUID_H

#include "SimulationBase.h"

class SimpleFluid : public SimulationBase
{
  public:
    SimpleFluid(OpenCLWrapper* ocl)
      : SimulationBase(ocl) {}

    void Init();
    void Update();

  private:
    void UpdateSimulation();

    int READ = 0, WRITE = 1;

    cl::Program program;

    cl::Image2D velocitiesBuffer[2];
    cl::Image2D pressureBuffer[2];
    cl::Image2D fieldBuffer[2];

    cl::Image2D emptyBuffer;

    cl::Buffer divergenceBuffer;

    cl::Buffer deltaTimeBuffer;

    cl::Kernel advectKernel;
    cl::Kernel divergenceKernel;
    cl::Kernel jacobiKernel;
    cl::Kernel pressureProjectionKernel;
};

#endif //SIMPLEFLUID_H
