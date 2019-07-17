#ifndef SMOKE_H
#define SMOKE_H

#include "SimulationBase.h"
#include "SimulationFactory.h"
#include "GLFWHandler.h"

class Smoke : public SimulationBase
{
  public:
    Smoke(const int width, const int height, const float dt)
      : SimulationBase(width, height, dt), sFact(SimulationFactory(width, height)) {}

    void Init() override;
    void Update() override;

    void SetHandler(GLFWHandler* hand);
    void AddSplat() override;
    void AddMultipleSplat(const int nb) override;
    void RemoveSplat() override;
  private:
    int READ = 0, WRITE = 1;

    SimulationFactory sFact;

    GLuint velocitiesTexture[4];
    GLuint density[4];
    GLuint temperature[4];
    GLuint divergenceCurlTexture;
    GLuint pressureTexture[2];
    GLuint emptyTexture;

    GLFWHandler* handler;
};

#endif //SMOKE_H
