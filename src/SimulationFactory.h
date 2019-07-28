#ifndef SIMULATIONFACTORY_H
#define SIMULATIONFACTORY_H

#include "GLUtils.h"
#include "ProgramOptions.h"

#include <functional>
#include <tuple>

void fillTextureWithFunctor(GLuint tex, 
    const unsigned width, 
    const unsigned height, 
    std::function<std::tuple<float, float, float, float>(unsigned, unsigned)> f);

class SimulationFactory
{
  public:
    SimulationFactory(ProgramOptions *options);

    ~SimulationFactory();

    void copy(const GLuint in, const GLuint out);
    float maxReduce(const GLuint tex);
    void addSplat(const GLuint field, const std::tuple<int, int> pos, const std::tuple<float, float, float> color, const float intensity);
    void simpleAdvect(const GLuint velocities, const GLuint field_READ, const GLuint field_WRITE);
    void RKAdvect(const GLuint velocities, const GLuint field_READ, const GLuint field_WRITE, const float dt);
    void mcAdvect(const GLuint velocities, const GLuint *fieldst);
    void maccormackStep(const GLuint field_WRITE, const GLuint field_n, const GLuint field_n_1, const GLuint field_n_hat, const GLuint velocities);
    void divergenceCurl(const GLuint velocities, const GLuint divergence_curl_WRITE);
    void solvePressure(const GLuint divergence_READ, const GLuint pressure_READ, const GLuint pressure_WRITE);
    void pressureProjection(const GLuint pressure_READ, const GLuint velocities_READ, const GLuint velocities_WRITE);
    void RBMethod(const GLuint *velocities, const GLuint divergence, const GLuint pressure);
    void applyVorticity(const GLuint velocities_READ_WRITE, const GLuint curl);
    void applyBuoyantForce(const GLuint velocities_READ_WRITE, const GLuint temperature, const GLuint density, const float kappa, const float sigma, const float t0);
    void updateQAndTheta(const GLuint qTex, const GLuint* thetaTex);
  private:
    void dispatch(const unsigned wSize, const unsigned hSize);

    ProgramOptions *options;

    unsigned globalSizeX, globalSizeY;

    GLint copyProgram;
    GLint maxReduceProgram;
    GLint addSmokeSpotProgram;
    GLint maccormackProgram;
    GLint RKProgram;
    GLint divCurlProgram;
    GLint divRBProgram;
    GLint jacobiProgram;
    GLint jacobiBlackProgram;
    GLint jacobiRedProgram;
    GLint pressureProjectionProgram;
    GLint pressureProjectionRBProgram;
    GLint applyVorticityProgram;
    GLint applyBuoyantForceProgram;
    GLint waterContinuityProgram;

    std::vector<GLuint> reduceTextures;
    GLuint emptyTexture;
};

#endif //SIMULATIONFACTORY_H
