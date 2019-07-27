#include "SimulationFactory.h"
#include "GLUtils.h"

#include <iostream>
#include <cmath>

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

  GL_CHECK( glBindTexture(GL_TEXTURE_2D, tex) );
  GL_CHECK( glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, data) );

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
  jacobiProgram = compileAndLinkShader("shaders/simulation/jacobi.comp", GL_COMPUTE_SHADER); 
  pressureProjectionProgram = compileAndLinkShader("shaders/simulation/pressure_projection.comp", GL_COMPUTE_SHADER); 
  applyVorticityProgram = compileAndLinkShader("shaders/simulation/applyVorticity.comp", GL_COMPUTE_SHADER); 
  applyBuoyantForceProgram = compileAndLinkShader("shaders/simulation/buoyantForce.comp", GL_COMPUTE_SHADER); 
  waterContinuityProgram = compileAndLinkShader("shaders/simulation/waterContinuity.comp", GL_COMPUTE_SHADER);

  /********** Textures for reduce **********/
  int nb = static_cast<int>(std::log(static_cast<double>(options->simWidth)) / std::log(2.0));

  int tSize = options->simWidth / 2;
  for(int i = 0; i < nb; ++i)
  {
    std::cout << i << " " << tSize << std::endl;
    reduceTextures.emplace_back(createTexture2D(tSize, tSize)); 
    tSize /= 2; 
  }
}

SimulationFactory::~SimulationFactory()
{
}

void SimulationFactory::dispatch()
{
  GL_CHECK( glDispatchCompute(globalSizeX, globalSizeY, 1) );
  GL_CHECK( glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT) );
}

void SimulationFactory::copy(const GLuint in, const GLuint out)
{
  GL_CHECK( glUseProgram(copyProgram) );
  GL_CHECK( glBindImageTexture(0, out, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F) );
  GL_CHECK( glBindImageTexture(1, in, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F) );

  dispatch();
}

float SimulationFactory::maxReduce(const GLuint tex)
{
  auto rUtil = [&](const GLuint iTex, const GLuint oTex, const unsigned size)
  {
    GL_CHECK( glUseProgram(maxReduceProgram) ); 

    GL_CHECK( glBindImageTexture(0, oTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F) );
    GL_CHECK( glActiveTexture(GL_TEXTURE1) ); GL_CHECK( glBindTexture(GL_TEXTURE_2D, iTex) );

    unsigned dSize = std::max(size / 32, 1u);

    GL_CHECK( glDispatchCompute(dSize, dSize, 1) );
    GL_CHECK( glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT) );
  };

  unsigned tSize = options->simWidth / 2;
  rUtil(tex, reduceTextures[0], tSize);
  for(unsigned i = 0; i < reduceTextures.size() - 1; ++i)
  {
    tSize /= 2;
    rUtil(reduceTextures[i], reduceTextures[i + 1], tSize);
  }

  float *data = new float[4];
  GL_CHECK( glBindTexture(GL_TEXTURE_2D, reduceTextures[reduceTextures.size() - 1]) );
  GL_CHECK( glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, data) );

  using std::max; using std::abs;
  float m = max(max(max(abs(data[0]), abs(data[1])), abs(data[2])), abs(data[3]));

  delete[] data;

  return m;
}

void SimulationFactory::RKAdvect(const GLuint velocities, const GLuint field_READ, const GLuint field_WRITE, const float dt)
{
  GL_CHECK( glUseProgram(RKProgram) );

  GLuint location = glGetUniformLocation(RKProgram, "dt");
  GL_CHECK( glUniform1f(location, dt) );

  GL_CHECK( glBindImageTexture(0, field_WRITE, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F) );
  GL_CHECK( glActiveTexture(GL_TEXTURE1) ); GL_CHECK( glBindTexture(GL_TEXTURE_2D, field_READ) );
  GL_CHECK( glActiveTexture(GL_TEXTURE2) ); GL_CHECK( glBindTexture(GL_TEXTURE_2D, velocities) );

  dispatch();
}

void SimulationFactory::mcAdvect(const GLuint velocities, const GLuint *fields)
{
  RKAdvect(velocities, fields[0], fields[1], options->dt);
  RKAdvect(velocities, fields[1], fields[2], - options->dt);
  maccormackStep(fields[3], fields[0], fields[1], fields[2], velocities);
}

void SimulationFactory::maccormackStep(const GLuint field_WRITE, const GLuint field_n, const GLuint field_n_1, const GLuint field_n_hat, const GLuint velocities)
{
  GL_CHECK( glUseProgram(maccormackProgram) );

  GLuint location = glGetUniformLocation(maccormackProgram, "dt");
  GL_CHECK( glUniform1f(location, options->dt) );

  GL_CHECK( glBindImageTexture(0, field_WRITE, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F) );
  GL_CHECK( glActiveTexture(GL_TEXTURE1) ); GL_CHECK( glBindTexture(GL_TEXTURE_2D, field_n) );
  GL_CHECK( glActiveTexture(GL_TEXTURE2) ); GL_CHECK( glBindTexture(GL_TEXTURE_2D, field_n_hat) );
  GL_CHECK( glActiveTexture(GL_TEXTURE3) ); GL_CHECK( glBindTexture(GL_TEXTURE_2D, field_n_1) );
  GL_CHECK( glActiveTexture(GL_TEXTURE4) ); GL_CHECK( glBindTexture(GL_TEXTURE_2D, velocities) );

  dispatch();
}

void SimulationFactory::divergenceCurl(const GLuint velocities, const GLuint divergence_curl_WRITE)
{
  GL_CHECK( glUseProgram(divCurlProgram) );
  GL_CHECK( glBindImageTexture(0, divergence_curl_WRITE, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F) );
  GL_CHECK( glActiveTexture(GL_TEXTURE1) ); GL_CHECK( glBindTexture(GL_TEXTURE_2D, velocities) );

  dispatch();
}

void SimulationFactory::solvePressure(const GLuint divergence_READ, const GLuint pressure_READ, const GLuint pressure_WRITE)
{
  GL_CHECK( glUseProgram(jacobiProgram) );

  GL_CHECK( glBindImageTexture(0, pressure_WRITE, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F) );
  GL_CHECK( glActiveTexture(GL_TEXTURE1) ); GL_CHECK( glBindTexture(GL_TEXTURE_2D, pressure_READ) );
  GL_CHECK( glActiveTexture(GL_TEXTURE2) ); GL_CHECK( glBindTexture(GL_TEXTURE_2D, divergence_READ) );

  dispatch();
}

void SimulationFactory::pressureProjection(const GLuint pressure_READ, const GLuint velocities_READ, const GLuint velocities_WRITE)
{
  GL_CHECK( glUseProgram(pressureProjectionProgram) );
  GL_CHECK( glBindImageTexture(0, velocities_WRITE, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F) );
  GL_CHECK( glActiveTexture(GL_TEXTURE1) ); GL_CHECK( glBindTexture(GL_TEXTURE_2D, velocities_READ) );
  GL_CHECK( glActiveTexture(GL_TEXTURE2) ); GL_CHECK( glBindTexture(GL_TEXTURE_2D, pressure_READ) );

  dispatch();
}

void SimulationFactory::applyVorticity(const GLuint velocities_READ_WRITE, const GLuint vorticity)
{
  GL_CHECK( glUseProgram(applyVorticityProgram) );

  GLuint location = glGetUniformLocation(applyVorticityProgram, "dt");
  GL_CHECK( glUniform1f(location, options->dt) );

  GL_CHECK( glBindImageTexture(0, velocities_READ_WRITE, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F) );
  GL_CHECK( glActiveTexture(GL_TEXTURE1) ); GL_CHECK( glBindTexture(GL_TEXTURE_2D, vorticity) );

  dispatch();
}

void SimulationFactory::applyBuoyantForce(const GLuint velocities_READ_WRITE, const GLuint temperature, const GLuint density, const float kappa, const float sigma, const float t0)
{
  GL_CHECK( glUseProgram(applyBuoyantForceProgram) );

  GLuint location = glGetUniformLocation(applyBuoyantForceProgram, "dt");
  GL_CHECK( glUniform1f(location, options->dt) );
  location = glGetUniformLocation(applyBuoyantForceProgram, "kappa");
  GL_CHECK( glUniform1f(location, kappa) );
  location = glGetUniformLocation(applyBuoyantForceProgram, "sigma");
  GL_CHECK( glUniform1f(location, sigma) );
  location = glGetUniformLocation(applyBuoyantForceProgram, "t0");
  GL_CHECK( glUniform1f(location, t0) );

  GL_CHECK( glBindImageTexture(0, velocities_READ_WRITE, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F) );
  GL_CHECK( glActiveTexture(GL_TEXTURE1) ); GL_CHECK( glBindTexture(GL_TEXTURE_2D, temperature) );
  GL_CHECK( glActiveTexture(GL_TEXTURE2) ); GL_CHECK( glBindTexture(GL_TEXTURE_2D, density) );

  dispatch();
}

void SimulationFactory::addSplat(const GLuint field, const std::tuple<int, int> pos, const std::tuple<float, float, float> color, const float intensity)
{
  auto [x, y] = pos;
  auto [r, g, b] = color;

  GL_CHECK( glUseProgram(addSmokeSpotProgram) );

  GLuint location = glGetUniformLocation(addSmokeSpotProgram, "spotPos");
  GL_CHECK( glUniform2i(location, x, y) );
  location = glGetUniformLocation(addSmokeSpotProgram, "color");
  GL_CHECK( glUniform3f(location, r, g, b) );
  location = glGetUniformLocation(addSmokeSpotProgram, "intensity");
  GL_CHECK( glUniform1f(location, intensity) );

  GL_CHECK( glBindImageTexture(0, field, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F) );

  dispatch();
}

void SimulationFactory::updateQAndTheta(const GLuint qTex, const GLuint* thetaTex)
{
  GL_CHECK( glUseProgram(waterContinuityProgram) );

  GL_CHECK( glBindImageTexture(0, qTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F) );
  GL_CHECK( glBindImageTexture(0, thetaTex[2], 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F) ); // The original potential temperature before advection
  GL_CHECK( glActiveTexture(GL_TEXTURE2) ); GL_CHECK( glBindTexture(GL_TEXTURE_2D, thetaTex[0]) );

  dispatch();
}
