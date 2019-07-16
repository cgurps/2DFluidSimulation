#ifndef SIMULATIONBASE_H
#define SIMULATIONBASE_H

#include "GLUtils.h"

class SimulationBase
{
  public:
    SimulationBase(const int width, const int height, const float dt)
      : width(width), height(height), dt(dt)
    {}

    ~SimulationBase()
    {}

    virtual void Init() = 0;
    virtual void Update() = 0;
    virtual void AddSplat() = 0;
    virtual void AddMultipleSplat(const int nb) = 0;
    virtual void RemoveSplat() = 0;

    GLuint shared_texture;

    int width, height;

    float dt;
};

#endif //SIMULATIONBASE_H
