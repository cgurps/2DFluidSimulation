#ifndef SIMULATIONFACTORY_H
#define SIMULATIONFACTORY_H

#include "GLUtils.h"
#include <functional>
#include <tuple>

void fillTextureWithFunctor(GLuint tex, 
    const int width, 
    const int height, 
    std::function<std::tuple<float, float, float, float>(int, int)> f);

class SimulationFactory
{
  public:
    SimulationFactory(const int width, const int height);

    ~SimulationFactory();

    void copy(const GLuint in, const GLuint out);
    void addSplat(const GLuint field, const std::tuple<int, int> pos, const std::tuple<float, float, float> color, const float intensity);
    void simpleAdvect(const GLuint velocities, const GLuint field_READ, const GLuint field_WRITE, const float dt);
    void maccormackStep(const GLuint velocities, const GLuint field_n_READ, const GLuint field_n_hat_READ, const GLuint field_n_1_hat_READ, const GLuint field_WRITE, const float dt);
    void divergence(const GLuint velocities, const GLuint divergence_WRITE);
    void solvePressure(const GLuint divergence_READ, const GLuint pressure_READ, const GLuint pressure_WRITE);
    void pressureProjection(const GLuint pressure_READ, const GLuint velocities_READ, const GLuint velocities_WRITE);
    void computeVorticity(const GLuint velocities_READ, const GLuint vorticity);
    void applyVorticity(const GLuint velocities_READ_WRITE, const GLuint vorticity, const float dt);
    void applyBuoyantForce(const GLuint velocities_READ_WRITE, const GLuint temperature, const GLuint density, const float dt, const float kappa, const float sigma, const float t0);
  private:
    int width, height;
    int globalSizeX, globalSizeY;

    GLint copyProgram;
    GLint addSmokeSpotProgram;
    GLint simpleAdvectProgram;
    GLint maccormackProgram;
    GLint divergenceProgram;
    GLint jacobiProgram;
    GLint pressureProjectionProgram;
    GLint vorticityProgram;
    GLint applyVorticityProgram;
    GLint applyBuoyantForceProgram;
};


#endif //SIMULATIONFACTORY_H
