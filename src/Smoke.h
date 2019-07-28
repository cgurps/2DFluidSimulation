#ifndef SMOKE_H
#define SMOKE_H

#include "SimulationBase.h"
#include "SimulationFactory.h"

class Smoke : public SimulationBase
{
  public:
    Smoke(ProgramOptions *options, GLFWHandler *handler)
      : SimulationBase(options, handler) {}

    ~Smoke();

    void Init() override;
    void Update() override;

    void AddSplat() override;
    void AddMultipleSplat(const int nb) override;
    void RemoveSplat() override;
  private:
    int READ = 0, WRITE = 1;

    GLuint velocitiesTexture[2];
    GLuint density[2];
    GLuint temperature[2];
    GLuint divRBTexture;
    GLuint pressureRBTexture;
    GLuint divergenceCurlTexture;
};

#endif //SMOKE_H
