#ifndef SIMULATIONBASE_H
#define SIMULATIONBASE_H

#include "GLUtils.h"

class SimulationBase
{
  public:
    SimulationBase(const int width, const int height)
      : width(width), height(height)
    {
      shared_texture = createTexture2D(width, height);
    }

    ~SimulationBase()
    {
      glDeleteTextures(1, &shared_texture);
    }

    virtual void Init() = 0;
    virtual void Update() = 0;

    GLuint shared_texture;

    int width, height;
};

#endif //SIMULATIONBASE_H
