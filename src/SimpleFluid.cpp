#include "SimpleFluid.h"
#include "GLUtils.h"

#include <string>
#include <sstream>
#include <fstream>
#include <cmath>

SimpleFluid::~SimpleFluid()
{
  GL_CHECK( glDeleteTextures(4, velocitiesTexture) );
  GL_CHECK( glDeleteTextures(4, density) );
  GL_CHECK( glDeleteTextures(1, &divergenceCurlTexture) );
  GL_CHECK( glDeleteTextures(2, pressureTexture) );
  GL_CHECK( glDeleteTextures(1, &emptyTexture) );
}

void SimpleFluid::Init()
{
  /********** Texture Initilization **********/
  auto f = [](unsigned, unsigned)
      {
        return std::make_tuple(0.0f, 0.0f,
                               0.0f, 0.0f);
      };

  //Velocities init function
  auto f1 = [this](unsigned x, unsigned y)
      {
        float xf = (float) x - 0.5f * (float) this->options->simWidth;
        float yf = (float) y - 0.5f * (float) this->options->simHeight;
        float norm = std::sqrt(xf * xf + yf * yf);
        float vx = (y > this->options->simHeight / 2) ? 5.0f : - 5.0f;
        float vy = (norm < 1e-5) ? 0.0f : 20.0f * xf / norm;
        return std::make_tuple(vx, 0.0f,
                               0.0f, 0.0f);
      };

  auto f3 = [this](unsigned x, unsigned y)
      {
        if(y > this->options->simHeight / 2) return std::make_tuple(0.0f, 0.0f, 0.0f, 0.0f);
        else return std::make_tuple(1.0f, 1.0f, 1.0f, 0.0f);
      };

  density[0] = createTexture2D(options->simWidth, options->simHeight);
  density[1] = createTexture2D(options->simWidth, options->simHeight);
  density[2] = createTexture2D(options->simWidth, options->simHeight);
  density[3] = createTexture2D(options->simWidth, options->simHeight);
  fillTextureWithFunctor(density[0], options->simWidth, options->simHeight, f3);

  velocitiesTexture[0] = createTexture2D(options->simWidth, options->simHeight);
  velocitiesTexture[1] = createTexture2D(options->simWidth, options->simHeight);
  velocitiesTexture[2] = createTexture2D(options->simWidth, options->simHeight);
  velocitiesTexture[3] = createTexture2D(options->simWidth, options->simHeight);
  fillTextureWithFunctor(velocitiesTexture[0], options->simWidth, options->simHeight, f1);

  divergenceCurlTexture = createTexture2D(options->simWidth, options->simHeight);
  fillTextureWithFunctor(divergenceCurlTexture, options->simWidth, options->simHeight, f);

  pressureTexture[0] = createTexture2D(options->simWidth, options->simHeight);
  pressureTexture[1] = createTexture2D(options->simWidth, options->simHeight);
  fillTextureWithFunctor(pressureTexture[0], options->simWidth, options->simHeight, f);

  emptyTexture = createTexture2D(options->simWidth, options->simHeight);
  fillTextureWithFunctor(emptyTexture, options->simWidth, options->simHeight, f);
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
  nbSplat = nb;
}

void SimpleFluid::RemoveSplat()
{
  addSplat = false;
}

float rd()
{
  return (float) rand() / (float) RAND_MAX; 
}

void SimpleFluid::Update()
{
  /********** Adding Splat *********/
  while(nbSplat > 0)
  {
    int x = std::clamp(static_cast<unsigned int>(options->simWidth * rd()), 50u, options->simWidth - 50);
    int y = std::clamp(static_cast<unsigned int>(options->simHeight * rd()), 50u, options->simHeight - 50);
    sFact.addSplat(velocitiesTexture[READ], std::make_tuple(x, y), std::make_tuple(100.0f * rd() - 50.0f, 100.0f * rd() - 50.0f, 0.0f), 75.0f);
    sFact.addSplat(density[READ], std::make_tuple(x, y), std::make_tuple(rd(), rd(), rd()), 8.0f);

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
    sFact.addSplat(density[READ], std::make_tuple(sX, sY), std::make_tuple(rd(), rd(), rd()), 2.0f);

    sOriginX = sX;
    sOriginY = sY;
  }

  /********** Vorticity **********/
  sFact.divergenceCurl(velocitiesTexture[READ], divergenceCurlTexture);
  sFact.applyVorticity(velocitiesTexture[READ], divergenceCurlTexture, options->dt);

  /********** Convection **********/
  /*
  sFact.mcAdvect(velocitiesTexture[READ], velocitiesTexture, options->dt);
  std::swap(velocitiesTexture[0], velocitiesTexture[3]);
  */
  sFact.RK4Advect(velocitiesTexture[READ], velocitiesTexture[READ], velocitiesTexture[WRITE], options->dt);
  std::swap(velocitiesTexture[0], velocitiesTexture[1]);

  /********** Divergence & Curl **********/
  sFact.divergenceCurl(velocitiesTexture[READ], divergenceCurlTexture);

  /********** Poisson Solving with Jacobi **********/
  sFact.copy(emptyTexture, pressureTexture[READ]);
  for(int k = 0; k < 30; ++k)
  {
    sFact.solvePressure(divergenceCurlTexture, pressureTexture[READ], pressureTexture[WRITE]);
    std::swap(pressureTexture[READ], pressureTexture[WRITE]);
  }

  /********** Pressure Projection **********/
  sFact.pressureProjection(pressureTexture[READ], velocitiesTexture[READ], velocitiesTexture[WRITE]);
  std::swap(velocitiesTexture[READ], velocitiesTexture[WRITE]);

  /********** Fields Advection **********/
  sFact.RK4Advect(velocitiesTexture[READ], density[READ], density[WRITE], options->dt);
  std::swap(density[0], density[1]);

  /*
  sFact.mcAdvect(velocitiesTexture[READ], density, options->dt);
  std::swap(density[0], density[3]);
  */

  /********** Updating the shared texture **********/
  shared_texture = density[READ];
}
