#include "SimulationFactory.h"

void fillTextureWithFunctor(GLuint tex, 
    const int width, 
    const int height, 
    std::function<std::tuple<float, float, float, float>(int, int)> f)
{
  float *data = new float[4 * width * height];

  for(int x = 0; x < width; ++x)
  {
    for(int y = 0; y < height; ++y)
    {
      const int pos = 4 * (y * width + x);

      auto [r, g, b, a] = f(x, y);

      data[pos    ] = r;
      data[pos + 1] = g;
      data[pos + 2] = g;
      data[pos + 3] = a;
    }
  }

  GL_CHECK( glBindTexture(GL_TEXTURE_2D, tex) );
  GL_CHECK( glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, data) );

  delete [] data;
}

SimulationFactory::SimulationFactory(const int width, const int height)
  : width(width), height(height), globalSizeX(width / 32), globalSizeY(height / 32)
{
  copyProgram = compileAndLinkShader("shaders/simulation/copy.compute", GL_COMPUTE_SHADER);

  addSmokeSpotProgram = compileAndLinkShader("shaders/simulation/addSmokeSpot.compute", GL_COMPUTE_SHADER); 
  GL_CHECK( glUseProgram(addSmokeSpotProgram) );
  GLuint location = glGetUniformLocation(addSmokeSpotProgram, "texSize");
  GL_CHECK( glUniform2i(location, this->width, this->height) );

  simpleAdvectProgram = compileAndLinkShader("shaders/simulation/advect.compute", GL_COMPUTE_SHADER); 
  GL_CHECK( glUseProgram(simpleAdvectProgram) );
  location = glGetUniformLocation(simpleAdvectProgram, "texSize");
  GL_CHECK( glUniform2i(location, this->width, this->height) );

  maccormackProgram = compileAndLinkShader("shaders/simulation/mccormack.compute", GL_COMPUTE_SHADER);
  GL_CHECK( glUseProgram(maccormackProgram) );
  location = glGetUniformLocation(maccormackProgram, "texSize");
  GL_CHECK( glUniform2i(location, this->width, this->height) );

  divergenceProgram = compileAndLinkShader("shaders/simulation/divergence.compute", GL_COMPUTE_SHADER); 
  GL_CHECK( glUseProgram(divergenceProgram) );
  location = glGetUniformLocation(divergenceProgram, "texSize");
  GL_CHECK( glUniform2i(location, width, height) );

  jacobiProgram = compileAndLinkShader("shaders/simulation/jacobi.compute", GL_COMPUTE_SHADER); 
  GL_CHECK( glUseProgram(jacobiProgram) );
  location = glGetUniformLocation(jacobiProgram, "texSize");
  GL_CHECK( glUniform2i(location, width, height) );

  pressureProjectionProgram = compileAndLinkShader("shaders/simulation/pressure_projection.compute", GL_COMPUTE_SHADER); 
  GL_CHECK( glUseProgram(pressureProjectionProgram) );
  location = glGetUniformLocation(pressureProjectionProgram, "texSize");
  GL_CHECK( glUniform2i(location, width, height) );

  vorticityProgram = compileAndLinkShader("shaders/simulation/vorticity.compute", GL_COMPUTE_SHADER); 
  GL_CHECK( glUseProgram(vorticityProgram) );
  location = glGetUniformLocation(vorticityProgram, "texSize");
  GL_CHECK( glUniform2i(location, width, height) );

  applyVorticityProgram = compileAndLinkShader("shaders/simulation/applyVorticity.compute", GL_COMPUTE_SHADER); 
  GL_CHECK( glUseProgram(vorticityProgram) );
  location = glGetUniformLocation(vorticityProgram, "texSize");
  GL_CHECK( glUniform2i(location, width, height) );

  applyBuoyantForceProgram = compileAndLinkShader("shaders/simulation/buoyantForce.compute", GL_COMPUTE_SHADER); 
  GL_CHECK( glUseProgram(applyBuoyantForceProgram) );
  location = glGetUniformLocation(applyBuoyantForceProgram, "texSize");
  GL_CHECK( glUniform2i(location, width, height) );
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

void SimulationFactory::simpleAdvect(const GLuint velocities, const GLuint field_READ, const GLuint field_WRITE, const float dt)
{
  GL_CHECK( glUseProgram(simpleAdvectProgram) );

  GLuint location = glGetUniformLocation(simpleAdvectProgram, "dt");
  GL_CHECK( glUniform1f(location, dt) );

  GL_CHECK( glBindImageTexture(0, field_WRITE, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F) );
  GL_CHECK( glActiveTexture(GL_TEXTURE1) ); GL_CHECK( glBindTexture(GL_TEXTURE_2D, field_READ) );
  GL_CHECK( glActiveTexture(GL_TEXTURE2) ); GL_CHECK( glBindTexture(GL_TEXTURE_2D, velocities) );
  GL_CHECK( glDispatchCompute(globalSizeX, globalSizeY, 1) );
  GL_CHECK( glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT) );
}

void SimulationFactory::maccormackStep(const GLuint velocities, const GLuint field_n_READ, const GLuint field_n_hat_READ, const GLuint field_n_1_hat_READ, const GLuint field_WRITE, const float dt)
{
  GL_CHECK( glUseProgram(maccormackProgram) );

  GLuint location = glGetUniformLocation(maccormackProgram, "dt");
  GL_CHECK( glUniform1f(location, dt) );

  GL_CHECK( glBindImageTexture(0, field_WRITE, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F) );
  GL_CHECK( glActiveTexture(GL_TEXTURE1) ); GL_CHECK( glBindTexture(GL_TEXTURE_2D, field_n_READ) );
  GL_CHECK( glActiveTexture(GL_TEXTURE2) ); GL_CHECK( glBindTexture(GL_TEXTURE_2D, field_n_hat_READ) );
  GL_CHECK( glActiveTexture(GL_TEXTURE3) ); GL_CHECK( glBindTexture(GL_TEXTURE_2D, field_n_1_hat_READ) );
  GL_CHECK( glActiveTexture(GL_TEXTURE4) ); GL_CHECK( glBindTexture(GL_TEXTURE_2D, velocities) );
  GL_CHECK( glDispatchCompute(globalSizeX, globalSizeY, 1) );
  GL_CHECK( glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT) );
}

void SimulationFactory::divergence(const GLuint velocities, const GLuint divergence_WRITE)
{
  GL_CHECK( glUseProgram(divergenceProgram) );
  GL_CHECK( glBindImageTexture(0, divergence_WRITE, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F) );
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

void SimulationFactory::computeVorticity(const GLuint velocities_READ, const GLuint vorticity)
{
  GL_CHECK( glUseProgram(vorticityProgram) );
  GL_CHECK( glBindImageTexture(0, vorticity, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F) );
  GL_CHECK( glActiveTexture(GL_TEXTURE1) ); GL_CHECK( glBindTexture(GL_TEXTURE_2D, velocities_READ) );
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
