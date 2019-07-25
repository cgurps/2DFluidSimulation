#include "Smoke.h"
#include "GLUtils.h"

#include <string>
#include <sstream>
#include <fstream>
#include <cmath>

Smoke::~Smoke()
{
  GL_CHECK( glDeleteTextures(4, velocitiesTexture) );
  GL_CHECK( glDeleteTextures(4, density) );
  GL_CHECK( glDeleteTextures(1, &divergenceCurlTexture) );
  GL_CHECK( glDeleteTextures(2, pressureTexture) );
  GL_CHECK( glDeleteTextures(1, &emptyTexture) );
}

void Smoke::Init()
{
  /********** Texture Initilization **********/
  auto f = [](unsigned, unsigned)
      {
        return std::make_tuple(0.0f, 0.0f,
                               0.0f, 0.0f);
      };

  density[0] = createTexture2D(options->simWidth, options->simHeight);
  density[1] = createTexture2D(options->simWidth, options->simHeight);
  density[2] = createTexture2D(options->simWidth, options->simHeight);
  fillTextureWithFunctor(density[0], options->simWidth, options->simHeight, f);

  temperature[0] = createTexture2D(options->simWidth, options->simHeight);
  temperature[1] = createTexture2D(options->simWidth, options->simHeight);
  temperature[2] = createTexture2D(options->simWidth, options->simHeight);
  fillTextureWithFunctor(temperature[0], options->simWidth, options->simHeight, f);

  velocitiesTexture[0] = createTexture2D(options->simWidth, options->simHeight);
  velocitiesTexture[1] = createTexture2D(options->simWidth, options->simHeight);
  velocitiesTexture[2] = createTexture2D(options->simWidth, options->simHeight);
  fillTextureWithFunctor(velocitiesTexture[0], options->simWidth, options->simHeight, f);

  divergenceCurlTexture = createTexture2D(options->simWidth, options->simHeight);
  fillTextureWithFunctor(divergenceCurlTexture, options->simWidth, options->simHeight, f);

  pressureTexture[0] = createTexture2D(options->simWidth, options->simHeight);
  pressureTexture[1] = createTexture2D(options->simWidth, options->simHeight);
  fillTextureWithFunctor(pressureTexture[0], options->simWidth, options->simHeight, f);

  emptyTexture = createTexture2D(options->simWidth, options->simHeight);
  fillTextureWithFunctor(emptyTexture, options->simWidth, options->simHeight, f);
}

void Smoke::AddSplat()
{
}

void Smoke::AddMultipleSplat(const int)
{
}

void Smoke::RemoveSplat()
{
}

void Smoke::Update()
{
  /********** Adding Smoke Origin *********/
  auto rd = []() -> double
  {
    return (double) rand() / (double) RAND_MAX;
  };

  int x = options->simWidth / 2; int y = 75;

  sFact.addSplat(density[READ],           std::make_tuple(x, y), std::make_tuple(0.12f, 0.31f, 0.7f), 1.0f);
  sFact.addSplat(temperature[READ],       std::make_tuple(x, y), std::make_tuple(rd() * 20.0f + 10.0f, 0.0f, 0.0f), 8.0f);
  sFact.addSplat(velocitiesTexture[READ], std::make_tuple(x, y), std::make_tuple(2.0f * rd() - 1.0f, 0.0f, 0.0f), 75.0f);

  /********** Divergence & Curl **********/
  sFact.divergenceCurl(velocitiesTexture[READ], divergenceCurlTexture);

  /********** Vorticity **********/
  sFact.applyVorticity(velocitiesTexture[READ], divergenceCurlTexture, options->dt);

  /********** Buoyant Force **********/
  sFact.applyBuoyantForce(velocitiesTexture[READ], temperature[READ], density[READ], options->dt, 0.25f, 0.1f, 15.0f);

  /********** Convection **********/
  sFact.mcAdvect(velocitiesTexture[READ], velocitiesTexture, options->dt);
  std::swap(velocitiesTexture[0], velocitiesTexture[2]);

  /********** Poisson Solving with Jacobi **********/
  sFact.copy(emptyTexture, pressureTexture[READ]);
  for(int k = 0; k < 25; ++k)
  {
    sFact.solvePressure(divergenceCurlTexture, pressureTexture[READ], pressureTexture[WRITE]);
    std::swap(pressureTexture[READ], pressureTexture[WRITE]);
  }

  /********** Pressure Projection **********/
  sFact.pressureProjection(pressureTexture[READ], velocitiesTexture[READ], velocitiesTexture[WRITE]);
  std::swap(velocitiesTexture[READ], velocitiesTexture[WRITE]);

  /********** Fields Advection **********/
  sFact.mcAdvect(velocitiesTexture[READ], density, options->dt);
  std::swap(density[0], density[2]);
  sFact.mcAdvect(velocitiesTexture[READ], temperature, options->dt);
  std::swap(temperature[0], temperature[2]);

  /********** Updating the shared texture **********/
  shared_texture = density[READ];
}
