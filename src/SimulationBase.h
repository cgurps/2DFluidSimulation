#ifndef SIMULATIONBASE_H
#define SIMULATIONBASE_H

#include "GLUtils.h"
#include "ProgramOptions.h"

class SimulationBase
{
  public:
    SimulationBase(ProgramOptions *options)
      : options(options)
    {}

    ~SimulationBase()
    {}

    virtual void Init() = 0;
    virtual void Update() = 0;
    virtual void AddSplat() = 0;
    virtual void AddMultipleSplat(const int nb) = 0;
    virtual void RemoveSplat() = 0;

    GLuint shared_texture;

    ProgramOptions *options;
};

#endif //SIMULATIONBASE_H
