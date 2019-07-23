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

    GLuint velocitiesTexture[4];
    GLuint density[4];
    GLuint temperature[4];
    GLuint divergenceCurlTexture;
    GLuint pressureTexture[2];
    GLuint emptyTexture;
};

#endif //SMOKE_H
