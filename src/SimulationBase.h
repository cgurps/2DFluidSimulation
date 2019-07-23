#ifndef SIMULATIONBASE_H
#define SIMULATIONBASE_H

#include "GLUtils.h"
#include "ProgramOptions.h"
#include "GLFWHandler.h"
#include "SimulationFactory.h"

class SimulationBase
{
  public:
    SimulationBase(ProgramOptions *options, GLFWHandler *handler)
      : options(options), handler(handler), sFact(SimulationFactory(options))
    {}

    ~SimulationBase()
    {}

    virtual void Init() = 0;
    virtual void Update() = 0;
    virtual void AddSplat() = 0;
    virtual void AddMultipleSplat(const int nb) = 0;
    virtual void RemoveSplat() = 0;

    ProgramOptions *options;
    GLuint shared_texture;

    GLFWHandler *handler; 
    SimulationFactory sFact;
};

#endif //SIMULATIONBASE_H
