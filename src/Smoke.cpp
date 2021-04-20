#include "Smoke.h"
#include "GLUtils.h"

#include <string>
#include <sstream>
#include <fstream>
#include <cmath>

Smoke::~Smoke()
{
  glDeleteTextures(4, velocitiesTexture);
  glDeleteTextures(4, density);
  glDeleteTextures(4, temperature);
  glDeleteTextures(1, &pressureRBTexture);
  glDeleteTextures(1, &divRBTexture);
  glDeleteTextures(1, &divergenceCurlTexture);
}

void Smoke::Init()
{
  density[0] = createTexture2D(options->simWidth, options->simHeight);
  density[1] = createTexture2D(options->simWidth, options->simHeight);
  density[2] = createTexture2D(options->simWidth, options->simHeight);
  density[3] = createTexture2D(options->simWidth, options->simHeight);

  temperature[0] = createTexture2D(options->simWidth, options->simHeight);
  temperature[1] = createTexture2D(options->simWidth, options->simHeight);
  temperature[2] = createTexture2D(options->simWidth, options->simHeight);
  temperature[3] = createTexture2D(options->simWidth, options->simHeight);

  velocitiesTexture[0] = createTexture2D(options->simWidth, options->simHeight);
  velocitiesTexture[1] = createTexture2D(options->simWidth, options->simHeight);
  velocitiesTexture[2] = createTexture2D(options->simWidth, options->simHeight);
  velocitiesTexture[3] = createTexture2D(options->simWidth, options->simHeight);

  divergenceCurlTexture = createTexture2D(options->simWidth, options->simHeight);

  divRBTexture = createTexture2D(options->simWidth / 2, options->simHeight / 2);

  pressureRBTexture = createTexture2D(options->simWidth / 2, options->simHeight / 2);
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

  const int x = options->simWidth / 2;
  const int y = 75;

  sFact.addSplat(density[READ],           std::make_tuple(x, y), std::make_tuple(0.12f, 0.31f, 0.7f), 0.5f);
  sFact.addSplat(temperature[READ],       std::make_tuple(x, y), std::make_tuple(rd() * 20.0f + 10.0f, 0.0f, 0.0f), 3.0f);
  sFact.addSplat(velocitiesTexture[READ], std::make_tuple(x, y), std::make_tuple(2.0f * rd() - 1.0f, 0.0f, 0.0f), 5.0f);

  float vMax = sFact.maxReduce(velocitiesTexture[READ]);
  if(vMax > 1e-5f) options->dt = 5.0f / (vMax + options->dt);

  /********** Convection **********/
  //sFact.RKAdvect(velocitiesTexture[READ], velocitiesTexture[READ], velocitiesTexture[WRITE], options->dt);
  sFact.mcAdvect(velocitiesTexture[READ], velocitiesTexture);
  std::swap(velocitiesTexture[0], velocitiesTexture[3]);

  /********** Fields Advection **********/
  //sFact.RKAdvect(velocitiesTexture[READ], density[READ], density[WRITE], options->dt);
  sFact.mcAdvect(velocitiesTexture[READ], density);
  std::swap(density[0], density[3]);

  //sFact.RKAdvect(velocitiesTexture[READ], temperature[READ], temperature[WRITE], options->dt);
  sFact.mcAdvect(velocitiesTexture[READ], temperature);
  std::swap(temperature[0], temperature[3]);

  /********** Buoyant Force **********/
  sFact.applyBuoyantForce(velocitiesTexture[READ], temperature[READ], density[READ], 0.25f, 0.1f, 10.0f);

  /********** Red-Black Jacobi for the pressure projection *********/
  sFact.RBMethod(velocitiesTexture, divRBTexture, pressureRBTexture);
  std::swap(velocitiesTexture[READ], velocitiesTexture[WRITE]);

  /********** Updating the shared texture **********/
  shared_texture = density[READ];
}
