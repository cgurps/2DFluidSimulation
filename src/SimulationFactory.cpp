#include "SimulationFactory.h"
#include "GLUtils.h"

#include <iostream>
#include <cmath>

/********** Utility Functions **********/
void bindImageTexture(const GLuint binding, const GLuint tex)
{
  glBindImageTexture(binding, tex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
}

void bindTexture(const GLuint binding, const GLuint tex)
{
  glActiveTexture(GL_TEXTURE0 + binding);
  glBindTexture(GL_TEXTURE_2D, tex);
}

void fillTextureWithFunctor(GLuint tex,
    const unsigned width,
    const unsigned height,
    std::function<std::tuple<float, float, float, float>(unsigned, unsigned)> f)
{
  float *data = new float[4 * width * height];

  for(unsigned x = 0; x < width; ++x)
  {
    for(unsigned y = 0; y < height; ++y)
    {
      const unsigned pos = 4 * (y * width + x);

      auto [r, g, b, a] = f(x, y);

      data[pos    ] = r;
      data[pos + 1] = g;
      data[pos + 2] = b;
      data[pos + 3] = a;
    }
  }

  glBindTexture(GL_TEXTURE_2D, tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, data);

  delete [] data;
}

SimulationFactory::SimulationFactory(ProgramOptions *options)
  : options(options),
    globalSizeX(options->simWidth / 32),
    globalSizeY(options->simHeight / 32)
{
  copyProgram = compileAndLinkShader("shaders/simulation/copy.comp", GL_COMPUTE_SHADER);
  maxReduceProgram = compileAndLinkShader("shaders/simulation/maxReduce.comp", GL_COMPUTE_SHADER);
  addSmokeSpotProgram = compileAndLinkShader("shaders/simulation/addSmokeSpot.comp", GL_COMPUTE_SHADER);
  maccormackProgram = compileAndLinkShader("shaders/simulation/mccormack.comp", GL_COMPUTE_SHADER);
  RKProgram = compileAndLinkShader("shaders/simulation/RKAdvect.comp", GL_COMPUTE_SHADER);
  divCurlProgram = compileAndLinkShader("shaders/simulation/divCurl.comp", GL_COMPUTE_SHADER);
  divRBProgram = compileAndLinkShader("shaders/simulation/divRB.comp", GL_COMPUTE_SHADER);
  jacobiProgram = compileAndLinkShader("shaders/simulation/jacobi.comp", GL_COMPUTE_SHADER);
  jacobiBlackProgram = compileAndLinkShader("shaders/simulation/jacobiBlack.comp", GL_COMPUTE_SHADER);
  jacobiRedProgram = compileAndLinkShader("shaders/simulation/jacobiRed.comp", GL_COMPUTE_SHADER);
  pressureProjectionProgram = compileAndLinkShader("shaders/simulation/pressure_projection.comp", GL_COMPUTE_SHADER);
  pressureProjectionRBProgram = compileAndLinkShader("shaders/simulation/pressureProjectionRB.comp", GL_COMPUTE_SHADER);
  applyVorticityProgram = compileAndLinkShader("shaders/simulation/applyVorticity.comp", GL_COMPUTE_SHADER);
  applyBuoyantForceProgram = compileAndLinkShader("shaders/simulation/buoyantForce.comp", GL_COMPUTE_SHADER);
  waterContinuityProgram = compileAndLinkShader("shaders/simulation/waterContinuity.comp", GL_COMPUTE_SHADER);

  /********** Textures for reduce **********/
  int nb = static_cast<int>(std::log(static_cast<double>(options->simWidth)) / std::log(2.0));

  int tSize = options->simWidth / 2;
  for(int i = 0; i < nb; ++i)
  {
    reduceTextures.emplace_back(createTexture2D(tSize, tSize));
    tSize /= 2;
  }

  emptyTexture = createTexture2D(options->simWidth / 2, options->simHeight / 2);
}

SimulationFactory::~SimulationFactory()
{
  glDeleteTextures(reduceTextures.size(), reduceTextures.data());
  glDeleteTextures(1, &emptyTexture);
}

void SimulationFactory::dispatch(const unsigned wSize, const unsigned hSize)
{
  glDispatchCompute(wSize, hSize, 1);
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void SimulationFactory::copy(const GLuint in, const GLuint out)
{
  glUseProgram(copyProgram);
  bindImageTexture(0, out);
  bindImageTexture(1, in);
  dispatch(globalSizeX, globalSizeY);
}

float SimulationFactory::maxReduce(const GLuint tex)
{
  auto rUtil = [&](const GLuint iTex, const GLuint oTex, const unsigned size)
  {
    glUseProgram(maxReduceProgram);
    bindImageTexture(0, oTex);
    bindTexture(1, iTex);
    unsigned dSize = std::max(size / 32, 1u);
    dispatch(dSize, dSize);
  };

  unsigned tSize = options->simWidth / 2;
  rUtil(tex, reduceTextures[0], tSize);
  for(unsigned i = 0; i < reduceTextures.size() - 1; ++i)
  {
    tSize /= 2;
    rUtil(reduceTextures[i], reduceTextures[i + 1], tSize);
  }

  float *data = new float[4];
  glBindTexture(GL_TEXTURE_2D, reduceTextures[reduceTextures.size() - 1]);
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, data);

  using std::max; using std::abs;
  float m = max(max(max(abs(data[0]), abs(data[1])), abs(data[2])), abs(data[3]));

  delete[] data;

  return m;
}

void SimulationFactory::RKAdvect(const GLuint velocities, const GLuint field_READ, const GLuint field_WRITE, const float dt)
{
  glUseProgram(RKProgram);
  GLuint location = glGetUniformLocation(RKProgram, "dt");
  glUniform1f(location, dt);
  bindImageTexture(0, field_WRITE);
  bindTexture(1, field_READ);
  bindTexture(2, velocities);
  dispatch(globalSizeX, globalSizeY);
}

void SimulationFactory::mcAdvect(const GLuint velocities, const GLuint *fields)
{
  RKAdvect(velocities, fields[0], fields[1], options->dt);
  RKAdvect(velocities, fields[1], fields[2], - options->dt);
  maccormackStep(fields[3], fields[0], fields[1], fields[2], velocities);
}

void SimulationFactory::maccormackStep(const GLuint field_WRITE, const GLuint field_n, const GLuint field_n_1, const GLuint field_n_hat, const GLuint velocities)
{
  glUseProgram(maccormackProgram);
  GLuint location = glGetUniformLocation(maccormackProgram, "dt");
  glUniform1f(location, options->dt);
  location = glGetUniformLocation(maccormackProgram, "revert");
  glUniform1f(location, options->mcRevert);
  bindImageTexture(0, field_WRITE);
  bindTexture(1, field_n);
  bindTexture(2, field_n_hat);
  bindTexture(3, field_n_1);
  bindTexture(4, velocities);
  dispatch(globalSizeX, globalSizeY);
}

void SimulationFactory::RBMethod(const GLuint *velocities, const GLuint divergence, const GLuint pressure)
{
  glUseProgram(divRBProgram);
  bindImageTexture(0, divergence);
  bindTexture(1, velocities[0]);
  dispatch(globalSizeX / 2, globalSizeY / 2);

  copy(emptyTexture, pressure); //TODO

  for(unsigned i = 0; i < options->jacobiIterations; ++i)
  {
    glUseProgram(jacobiBlackProgram);
    bindImageTexture(0, pressure);
    bindTexture(1, pressure);
    bindTexture(2, divergence);
    dispatch(globalSizeX / 2, globalSizeY / 2);

    glUseProgram(jacobiRedProgram);
    bindImageTexture(0, pressure);
    bindTexture(1, pressure);
    bindTexture(2, divergence);
    dispatch(globalSizeX / 2, globalSizeY / 2);
  }

  glUseProgram(pressureProjectionRBProgram);
  bindImageTexture(0, velocities[1]);
  bindTexture(1, velocities[0]);
  bindTexture(2, pressure);
  dispatch(globalSizeX / 2, globalSizeY / 2);
}

void SimulationFactory::divergenceCurl(const GLuint velocities, const GLuint divergence_curl_WRITE)
{
  glUseProgram(divCurlProgram);
  bindImageTexture(0, divergence_curl_WRITE);
  bindTexture(1, velocities);
  dispatch(globalSizeX, globalSizeY);
}

void SimulationFactory::solvePressure(const GLuint divergence_READ, const GLuint pressure_READ, const GLuint pressure_WRITE)
{
  glUseProgram(jacobiProgram);
  bindImageTexture(0, pressure_WRITE);
  bindTexture(1, pressure_READ);
  bindTexture(2, divergence_READ);
  dispatch(globalSizeX, globalSizeY);
}

void SimulationFactory::pressureProjection(const GLuint pressure_READ, const GLuint velocities_READ, const GLuint velocities_WRITE)
{
  glUseProgram(pressureProjectionProgram);
  bindImageTexture(0, velocities_WRITE);
  bindTexture(1, velocities_READ);
  bindTexture(2, pressure_READ);
  dispatch(globalSizeX, globalSizeY);
}

void SimulationFactory::applyVorticity(const GLuint velocities_READ_WRITE, const GLuint curl)
{
  glUseProgram(applyVorticityProgram);
  GLuint location = glGetUniformLocation(applyVorticityProgram, "dt");
  glUniform1f(location, options->dt);
  bindImageTexture(0, velocities_READ_WRITE);
  bindTexture(1, curl);
  dispatch(globalSizeX, globalSizeY);
}

void SimulationFactory::applyBuoyantForce(const GLuint velocities_READ_WRITE, const GLuint temperature, const GLuint density, const float kappa, const float sigma, const float t0)
{
  glUseProgram(applyBuoyantForceProgram);
  GLuint location = glGetUniformLocation(applyBuoyantForceProgram, "dt");
  glUniform1f(location, options->dt);
  location = glGetUniformLocation(applyBuoyantForceProgram, "kappa");
  glUniform1f(location, kappa);
  location = glGetUniformLocation(applyBuoyantForceProgram, "sigma");
  glUniform1f(location, sigma);
  location = glGetUniformLocation(applyBuoyantForceProgram, "t0");
  glUniform1f(location, t0);
  bindImageTexture(0, velocities_READ_WRITE);
  bindTexture(1, temperature);
  bindTexture(2, density);
  dispatch(globalSizeX, globalSizeY);
}

void SimulationFactory::addSplat(const GLuint field, const std::tuple<int, int> pos, const std::tuple<float, float, float> color, const float intensity)
{
  auto [x, y] = pos;
  auto [r, g, b] = color;

  glUseProgram(addSmokeSpotProgram);
  GLuint location = glGetUniformLocation(addSmokeSpotProgram, "spotPos");
  glUniform2i(location, x, y);
  location = glGetUniformLocation(addSmokeSpotProgram, "color");
  glUniform3f(location, r, g, b);
  location = glGetUniformLocation(addSmokeSpotProgram, "intensity");
  glUniform1f(location, intensity);
  bindImageTexture(0, field);
  dispatch(globalSizeX, globalSizeY);
}

void SimulationFactory::updateQAndTheta(const GLuint qTex, const GLuint* thetaTex)
{
  glUseProgram(waterContinuityProgram);
  bindImageTexture(0, qTex);
  bindImageTexture(1, thetaTex[2]);
  bindTexture(2, thetaTex[0]);
  dispatch(globalSizeX, globalSizeY);
}
