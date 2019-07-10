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
    cl::Program program;
};

#endif //SIMPLEFLUID_H
