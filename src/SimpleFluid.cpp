#include "SimpleFluid.h"
#include "GLUtils.h"

#include <string>
#include <sstream>
#include <fstream>
#include <cmath>

float rd()
{
  return (float) rand() / (float) RAND_MAX; 
}

SimpleFluid::~SimpleFluid()
{
  glDeleteTextures(4, velocitiesTexture);
  glDeleteTextures(4, density);
  glDeleteTextures(1, &divergenceCurlTexture);
  glDeleteTextures(1, &divRBTexture);
  glDeleteTextures(1, &pressureRBTexture);
}

void SimpleFluid::Init()
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
  density[3] = createTexture2D(options->simWidth, options->simHeight);
  fillTextureWithFunctor(density[0], options->simWidth, options->simHeight, f);

  velocitiesTexture[0] = createTexture2D(options->simWidth, options->simHeight);
  velocitiesTexture[1] = createTexture2D(options->simWidth, options->simHeight);
  velocitiesTexture[2] = createTexture2D(options->simWidth, options->simHeight);
  velocitiesTexture[3] = createTexture2D(options->simWidth, options->simHeight);
  fillTextureWithFunctor(velocitiesTexture[0], options->simWidth, options->simHeight, f);

  divergenceCurlTexture = createTexture2D(options->simWidth, options->simHeight);
  fillTextureWithFunctor(divergenceCurlTexture, options->simWidth, options->simHeight, f);

  divRBTexture = createTexture2D(options->simWidth / 2, options->simHeight / 2);
  fillTextureWithFunctor(divRBTexture, options->simWidth / 2, options->simHeight / 2, f);

  pressureRBTexture = createTexture2D(options->simWidth / 2, options->simHeight / 2);

  unsigned x = 300u; unsigned y = 512u;
  sFact.addSplat(velocitiesTexture[READ], std::make_tuple(x, y), std::make_tuple(80.0f, 7.0f, 0.0f), 1.0f);
  sFact.addSplat(density[READ], std::make_tuple(x, y), std::make_tuple(75.0 / 255.0, 89.0 / 255.0, 1.0), 2.5f);

  x = 700u; y = 512u;
  sFact.addSplat(velocitiesTexture[READ], std::make_tuple(x, y), std::make_tuple(- 80.0f, - 7.0f, 0.0f), 1.0f);
  sFact.addSplat(density[READ], std::make_tuple(x, y), std::make_tuple(1.0, 151.0 / 255.0, 60.0 / 255.0), 2.5f);
}

void SimpleFluid::AddSplat()
{
  glfwGetCursorPos(handler->window, &sOriginX, &sOriginY);
  sOriginX = (double) options->simWidth * sOriginX / (double) options->windowWidth;
  sOriginY = (double) options->simHeight * (1.0 - sOriginY / (double) options->windowHeight);

  addSplat = true;
}

void SimpleFluid::AddMultipleSplat(const int nb)
{
  nbSplat += nb;
}

void SimpleFluid::RemoveSplat()
{
  addSplat = false;
}

void SimpleFluid::Update()
{
  /********** Adding Splat *********/
  while(nbSplat > 0)
  {
    int x = std::clamp(static_cast<unsigned int>(options->simWidth * rd()), 50u, options->simWidth - 50);
    int y = std::clamp(static_cast<unsigned int>(options->simHeight * rd()), 50u, options->simHeight - 50);
    sFact.addSplat(velocitiesTexture[READ], std::make_tuple(x, y), std::make_tuple(100.0f * rd() - 50.0f, 100.0f * rd() - 50.0f, 0.0f), 50.0f);
    sFact.addSplat(density[READ], std::make_tuple(x, y), std::make_tuple(rd(), rd(), rd()), 2.5f);

    --nbSplat;
  }

  if(addSplat)
  {
    float vScale = 1.0f;
    double sX, sY;
    glfwGetCursorPos(handler->window, &sX, &sY);
    sX = (double) options->simWidth * sX / (double) options->windowWidth;
    sY = (double) options->simHeight * (1.0 - sY / (double) options->windowHeight);
    sFact.addSplat(velocitiesTexture[READ], std::make_tuple(sX, sY), std::make_tuple(vScale * (sX - sOriginX), vScale * (sY - sOriginY), 0.0f), 40.0f);
    sFact.addSplat(density[READ], std::make_tuple(sX, sY), std::make_tuple(rd(), rd(), rd()), 1.0f);

    sOriginX = sX;
    sOriginY = sY;
  }

  float vMax = sFact.maxReduce(velocitiesTexture[READ]);
  if(vMax > 1e-10f) options->dt = 5.0f / vMax;

  /********** Convection **********/
  //sFact.RKAdvect(velocitiesTexture[READ], velocitiesTexture[READ], velocitiesTexture[WRITE], options->dt);
  sFact.mcAdvect(velocitiesTexture[READ], velocitiesTexture);
  std::swap(velocitiesTexture[0], velocitiesTexture[3]);

  /********** Field Advection **********/
  //sFact.RKAdvect(velocitiesTexture[READ], density[READ], density[WRITE], options->dt);
  sFact.mcAdvect(velocitiesTexture[READ], density);
  std::swap(density[0], density[3]);

  /********** Red-Black Jacobi for the pressure projection *********/
  sFact.RBMethod(velocitiesTexture, divRBTexture, pressureRBTexture);
  std::swap(velocitiesTexture[READ], velocitiesTexture[WRITE]);

  /********** Updating the shared texture **********/
  shared_texture = density[READ];
}
