#ifndef CLOUDS_H
#define CLOUDS_H

#include "SimulationBase.h"
#include "SimulationFactory.h"

class Clouds : public SimulationBase
{
  public:
    Clouds(ProgramOptions *options, GLFWHandler *handler)
      : SimulationBase(options, handler) {}

    ~Clouds();

    void Init() override;
    void Update() override;

    void AddSplat() override;
    void AddMultipleSplat(const int nb) override;
    void RemoveSplat() override;
  private:
    int READ = 0, WRITE = 1;

    GLuint velocitiesTexture[3];
    GLuint density[3];
    GLuint potentialTemperature[3];
    GLuint divergenceCurlTexture;
    GLuint pressureTexture[2];
    GLuint emptyTexture;
};

#endif //CLOUD_H
