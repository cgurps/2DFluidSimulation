#ifndef SIMPLEFLUID_H
#define SIMPLEFLUID_H

#include "SimulationBase.h"

class SimpleFluid : public SimulationBase
{
  public:
    SimpleFluid(const int width, const int height)
      : SimulationBase(width, height) {}

    void Init();
    void Update();

  private:
    int READ = 0, WRITE = 1;

    GLint copyProgram;
    GLint advectProgram;

    GLuint velocitiesTexture[2];
    GLuint dummyTexture[2];
};

#endif //SIMPLEFLUID_H
