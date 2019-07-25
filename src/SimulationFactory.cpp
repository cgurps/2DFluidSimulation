#include "SimulationFactory.h"

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
  addSmokeSpotProgram = compileAndLinkShader("shaders/simulation/addSmokeSpot.comp", GL_COMPUTE_SHADER); 
  maccormackProgram = compileAndLinkShader("shaders/simulation/mccormack.comp", GL_COMPUTE_SHADER);
  RKProgram = compileAndLinkShader("shaders/simulation/RKAdvect.comp", GL_COMPUTE_SHADER);
  divCurlProgram = compileAndLinkShader("shaders/simulation/divCurl.comp", GL_COMPUTE_SHADER); 
  jacobiProgram = compileAndLinkShader("shaders/simulation/jacobi.comp", GL_COMPUTE_SHADER); 
  pressureProjectionProgram = compileAndLinkShader("shaders/simulation/pressure_projection.comp", GL_COMPUTE_SHADER); 
  applyVorticityProgram = compileAndLinkShader("shaders/simulation/applyVorticity.comp", GL_COMPUTE_SHADER); 
  applyBuoyantForceProgram = compileAndLinkShader("shaders/simulation/buoyantForce.comp", GL_COMPUTE_SHADER); 
}

SimulationFactory::~SimulationFactory()
{
}

void SimulationFactory::copy(const GLuint in, const GLuint out)
{
  GL_CHECK( glUseProgram(copyProgram) );
  GL_CHECK( glBindImageTexture(0, out, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F) );
  GL_CHECK( glBindImageTexture(1, in, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F) );
  GL_CHECK( glDispatchCompute(globalSizeX, globalSizeY, 1) );
  GL_CHECK( glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT) );
}

void SimulationFactory::RKAdvect(const GLuint velocities, const GLuint field_READ, const GLuint field_WRITE, const float dt)
{
  GL_CHECK( glUseProgram(RKProgram) );

  GLuint location = glGetUniformLocation(RKProgram, "dt");
  GL_CHECK( glUniform1f(location, dt) );
  location = glGetUniformLocation(RKProgram, "order");
  GL_CHECK( glUniform1i(location, options->RKorder) );

  GL_CHECK( glBindImageTexture(0, field_WRITE, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F) );
  GL_CHECK( glActiveTexture(GL_TEXTURE1) ); GL_CHECK( glBindTexture(GL_TEXTURE_2D, field_READ) );
  GL_CHECK( glActiveTexture(GL_TEXTURE2) ); GL_CHECK( glBindTexture(GL_TEXTURE_2D, velocities) );
  GL_CHECK( glDispatchCompute(globalSizeX, globalSizeY, 1) );
  GL_CHECK( glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT) );
}

void SimulationFactory::mcAdvect(const GLuint velocities, const GLuint *fields, const float dt)
{
  RKAdvect(velocities, fields[0], fields[1],   dt);
  RKAdvect(velocities, fields[1], fields[2], - dt);
  maccormackStep(fields[0], fields[1], fields[2]);
}

void SimulationFactory::maccormackStep(const GLuint field_n, const GLuint field_n_1, const GLuint field_n_hat)
{
  GL_CHECK( glUseProgram(maccormackProgram) );

  GL_CHECK( glBindImageTexture(0, field_n, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F) );
  GL_CHECK( glActiveTexture(GL_TEXTURE1) ); GL_CHECK( glBindTexture(GL_TEXTURE_2D, field_n_hat) );
  GL_CHECK( glActiveTexture(GL_TEXTURE2) ); GL_CHECK( glBindTexture(GL_TEXTURE_2D, field_n_1) );
  GL_CHECK( glDispatchCompute(globalSizeX, globalSizeY, 1) );
  GL_CHECK( glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT) );
}

void SimulationFactory::divergenceCurl(const GLuint velocities, const GLuint divergence_curl_WRITE)
{
  GL_CHECK( glUseProgram(divCurlProgram) );
  GL_CHECK( glBindImageTexture(0, divergence_curl_WRITE, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F) );
  GL_CHECK( glActiveTexture(GL_TEXTURE1) ); GL_CHECK( glBindTexture(GL_TEXTURE_2D, velocities) );
  GL_CHECK( glDispatchCompute(globalSizeX, globalSizeY, 1) );
  GL_CHECK( glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT) );
}

void SimulationFactory::solvePressure(const GLuint divergence_READ, const GLuint pressure_READ, const GLuint pressure_WRITE)
{
  GL_CHECK( glUseProgram(jacobiProgram) );

  GL_CHECK( glBindImageTexture(0, pressure_WRITE, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F) );
  GL_CHECK( glActiveTexture(GL_TEXTURE1) ); GL_CHECK( glBindTexture(GL_TEXTURE_2D, pressure_READ) );
  GL_CHECK( glActiveTexture(GL_TEXTURE2) ); GL_CHECK( glBindTexture(GL_TEXTURE_2D, divergence_READ) );
  GL_CHECK( glDispatchCompute(globalSizeX, globalSizeY, 1) );
  GL_CHECK( glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT) );
}

void SimulationFactory::pressureProjection(const GLuint pressure_READ, const GLuint velocities_READ, const GLuint velocities_WRITE)
{
  GL_CHECK( glUseProgram(pressureProjectionProgram) );
  GL_CHECK( glBindImageTexture(0, velocities_WRITE, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F) );
  GL_CHECK( glActiveTexture(GL_TEXTURE1) ); GL_CHECK( glBindTexture(GL_TEXTURE_2D, velocities_READ) );
  GL_CHECK( glActiveTexture(GL_TEXTURE2) ); GL_CHECK( glBindTexture(GL_TEXTURE_2D, pressure_READ) );
  GL_CHECK( glDispatchCompute(globalSizeX, globalSizeY, 1) );
  GL_CHECK( glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT) );
}

void SimulationFactory::applyVorticity(const GLuint velocities_READ_WRITE, const GLuint vorticity, const float dt)
{
  GL_CHECK( glUseProgram(applyVorticityProgram) );

  GLuint location = glGetUniformLocation(applyVorticityProgram, "dt");
  GL_CHECK( glUniform1f(location, dt) );

  GL_CHECK( glBindImageTexture(0, velocities_READ_WRITE, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F) );
  GL_CHECK( glActiveTexture(GL_TEXTURE1) ); GL_CHECK( glBindTexture(GL_TEXTURE_2D, vorticity) );
  GL_CHECK( glDispatchCompute(globalSizeX, globalSizeY, 1) );
  GL_CHECK( glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT) );
}

void SimulationFactory::applyBuoyantForce(const GLuint velocities_READ_WRITE, const GLuint temperature, const GLuint density, const float dt, const float kappa, const float sigma, const float t0)
{
  GL_CHECK( glUseProgram(applyBuoyantForceProgram) );

  GLuint location = glGetUniformLocation(applyBuoyantForceProgram, "dt");
  GL_CHECK( glUniform1f(location, dt) );
  location = glGetUniformLocation(applyBuoyantForceProgram, "kappa");
  GL_CHECK( glUniform1f(location, kappa) );
  location = glGetUniformLocation(applyBuoyantForceProgram, "sigma");
  GL_CHECK( glUniform1f(location, sigma) );
  location = glGetUniformLocation(applyBuoyantForceProgram, "t0");
  GL_CHECK( glUniform1f(location, t0) );

  GL_CHECK( glBindImageTexture(0, velocities_READ_WRITE, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F) );
  GL_CHECK( glActiveTexture(GL_TEXTURE1) ); GL_CHECK( glBindTexture(GL_TEXTURE_2D, temperature) );
  GL_CHECK( glActiveTexture(GL_TEXTURE2) ); GL_CHECK( glBindTexture(GL_TEXTURE_2D, density) );
  GL_CHECK( glDispatchCompute(globalSizeX, globalSizeY, 1) );
  GL_CHECK( glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT) );
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
  GL_CHECK( glDispatchCompute(globalSizeX, globalSizeY, 1) );
  GL_CHECK( glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT) );
}
