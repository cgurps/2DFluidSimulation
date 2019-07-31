#ifndef SIMPLEFLUID_H
#define SIMPLEFLUID_H

#include "SimulationBase.h"
#include "SimulationFactory.h"

class SimpleFluid : public SimulationBase
{
  public:
    SimpleFluid(ProgramOptions *options, GLFWHandler* handler)
      : SimulationBase(options, handler) {}

    ~SimpleFluid();

    void Init() override;
    void Update() override;

    void AddSplat() override;
    void AddMultipleSplat(const int nb) override;
    void RemoveSplat() override;
  private:
    int READ = 0, WRITE = 1;

    bool addSplat = false;
    double sOriginX, sOriginY;
    int nbSplat = 0;

    GLuint velocitiesTexture[4];
    GLuint density[4];
    GLuint divRBTexture;
    GLuint pressureRBTexture;
    GLuint divergenceCurlTexture;
};

#endif //SIMPLEFLUID_H
