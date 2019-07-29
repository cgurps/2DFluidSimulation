#include "Clouds.h"
#include "GLUtils.h"

#include <string>
#include <sstream>
#include <fstream>
#include <cmath>

Clouds::~Clouds()
{
  glDeleteTextures(4, velocitiesTexture);
  glDeleteTextures(4, density);
  glDeleteTextures(1, &divergenceCurlTexture);
  glDeleteTextures(2, pressureTexture);
  glDeleteTextures(1, &emptyTexture);
}

void Clouds::Init()
{
  /********** Texture Initilization **********/
  auto f = [](unsigned, unsigned)
      {
        return std::make_tuple(0.0f, 0.0f, 0.0f, 0.0f);
      };  

  auto f1 = [](unsigned, unsigned y)
      {
        if(y <= 5) return std::make_tuple(1.0f, 0.0f, 0.0f, 0.0f);
        else return std::make_tuple(0.0f, 0.0f, 0.0f, 0.0f);
      };

  auto f2 = [](unsigned, unsigned y)
      {
        if(y <= 5) return std::make_tuple(20.0f, 0.0f, 0.0f, 0.0f);
        else return std::make_tuple(0.0f, 0.0f, 0.0f, 0.0f);
      };

  density[0] = createTexture2D(options->simWidth, options->simHeight);
  density[1] = createTexture2D(options->simWidth, options->simHeight);
  density[2] = createTexture2D(options->simWidth, options->simHeight);
  fillTextureWithFunctor(density[0], options->simWidth, options->simHeight, f1);

  potentialTemperature[0] = createTexture2D(options->simWidth, options->simHeight);
  potentialTemperature[1] = createTexture2D(options->simWidth, options->simHeight);
  potentialTemperature[2] = createTexture2D(options->simWidth, options->simHeight);
  fillTextureWithFunctor(potentialTemperature[0], options->simWidth, options->simHeight, f2);

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

void Clouds::AddSplat()
{
}

void Clouds::AddMultipleSplat(const int)
{
}

void Clouds::RemoveSplat()
{
}

void Clouds::Update()
{
  /********** Adding Clouds Origin *********/
  auto rd = []() -> double
  {
    return (double) rand() / (double) RAND_MAX;
  };

  int x = options->simWidth / 2; int y = 75;

  /*
  sFact.addSplat(density[READ],           std::make_tuple(x, y), std::make_tuple(0.12f, 0.31f, 0.7f), 1.0f);
  sFact.addSplat(potentialTemperature[READ],       std::make_tuple(x, y), std::make_tuple(rd() * 20.0f + 10.0f, 0.0f, 0.0f), 8.0f);
  sFact.addSplat(velocitiesTexture[READ], std::make_tuple(x, y), std::make_tuple(2.0f * rd() - 1.0f, 0.0f, 0.0f), 75.0f);
  */

  /********** Convection **********/
  sFact.mcAdvect(velocitiesTexture[READ], velocitiesTexture);
  std::swap(velocitiesTexture[0], velocitiesTexture[2]);

  /********** Advections **********/
  sFact.mcAdvect(velocitiesTexture[READ], density);
  std::swap(density[0], density[2]);
  sFact.mcAdvect(velocitiesTexture[READ], potentialTemperature);
  std::swap(potentialTemperature[0], potentialTemperature[2]);

  /********** Buoyant Force **********/
  sFact.applyBuoyantForce(velocitiesTexture[READ], potentialTemperature[READ], density[READ], 0.25f, 0.1f, 15.0f);

  /********** Divergence & Curl **********/
  sFact.divergenceCurl(velocitiesTexture[READ], divergenceCurlTexture);

  /********** Vorticity **********/
  sFact.applyVorticity(velocitiesTexture[READ], divergenceCurlTexture);

  /********** Updating Thermodynamics *********/
  sFact.updateQAndTheta(density[READ], potentialTemperature);

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

  /********** Updating the shared texture **********/
  shared_texture = density[READ];
}
