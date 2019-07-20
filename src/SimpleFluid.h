#ifndef SIMPLEFLUID_H
#define SIMPLEFLUID_H

#include "SimulationBase.h"
#include "SimulationFactory.h"
#include "GLFWHandler.h"

class SimpleFluid : public SimulationBase
{
  public:
    SimpleFluid(const int width, const int height, const float dt)
      : SimulationBase(width, height, dt), sFact(SimulationFactory(width, height)) {}

    ~SimpleFluid();

    void Init() override;
    void Update() override;

    void SetHandler(GLFWHandler* hand);

    void AddSplat() override;
    void AddMultipleSplat(const int nb) override;
    void RemoveSplat() override;
  private:
    int READ = 0, WRITE = 1;

    SimulationFactory sFact;

    bool addSplat = false;
    double sOriginX, sOriginY;
    int nbSplat = 0;

    GLuint velocitiesTexture[4];
    GLuint density[4];
    GLuint divergenceCurlTexture;
    GLuint pressureTexture[2];
    GLuint emptyTexture;

    GLFWHandler* handler;
};

#endif //SIMPLEFLUID_H
