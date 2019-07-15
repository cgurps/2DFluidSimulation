#include "SimpleFluid.h"
#include "GLUtils.h"

#include <string>
#include <sstream>
#include <fstream>
#include <cmath>

void SimpleFluid::Init()
{
  GLint work_grp_cnt[3];
  GL_CHECK( glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_grp_cnt[0]) );
  GL_CHECK( glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_grp_cnt[1]) );
  GL_CHECK( glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_grp_cnt[2]) );

  GLint work_grp_size[3];
  GL_CHECK( glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &work_grp_size[0]) );
  GL_CHECK( glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &work_grp_size[1]) );
  GL_CHECK( glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &work_grp_size[2]) );

  GLint work_grp_inv;
  GL_CHECK( glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &work_grp_inv) );
 
  printf("Max global (total) work group size x:%i y:%i z:%i\n",
    work_grp_cnt[0], work_grp_cnt[1], work_grp_cnt[2]);
  printf("Max local (in one shader) work group sizes x:%i y:%i z:%i\n",
    work_grp_size[0], work_grp_size[1], work_grp_size[2]);
  printf("Max local work group invocations %i\n", work_grp_inv);

  /********** Texture Initilization **********/
  auto f = [](int x, int y)
      {
        return std::make_tuple(0.0f, 0.0f,
                               0.0f, 0.0f);
      };

  auto f1 = [this](int x, int y)
      {
        return std::make_tuple(10.0f, 0.0f,
                               0.0f, 0.0f);
      };

  density[0] = createTexture2D(width, height);
  density[1] = createTexture2D(width, height);
  density[2] = createTexture2D(width, height);
  density[3] = createTexture2D(width, height);
  fillTextureWithFunctor(density[0], width, height, f);

  temperature[0] = createTexture2D(width, height);
  temperature[1] = createTexture2D(width, height);
  temperature[2] = createTexture2D(width, height);
  temperature[3] = createTexture2D(width, height);
  fillTextureWithFunctor(temperature[0], width, height, f);

  velocitiesTexture[0] = createTexture2D(width, height);
  velocitiesTexture[1] = createTexture2D(width, height);
  velocitiesTexture[2] = createTexture2D(width, height);
  velocitiesTexture[3] = createTexture2D(width, height);
  fillTextureWithFunctor(velocitiesTexture[0], width, height, f1);

  divergenceTexture = createTexture2D(width, height);

  vorticity = createTexture2D(width, height);

  pressureTexture[0] = createTexture2D(width, height);
  pressureTexture[1] = createTexture2D(width, height);

  emptyTexture = createTexture2D(width, height);
  fillTextureWithFunctor(emptyTexture, width, height,
      [](int x, int y)
      {
        return std::make_tuple(0.0f, 0.0f, 0.0f, 0.0f);
      });
}

void SimpleFluid::SetHandler(GLFWHandler* hand)
{
  handler = hand;
}

void SimpleFluid::AddSplat()
{
  addSplat = true;
}

void SimpleFluid::RemoveSplat()
{
  addSplat = false;
}

void SimpleFluid::Update()
{
  GLint64 startTime, stopTime;
  GLuint queryID[2];

  // generate two queries
  glGenQueries(2, queryID);
  glQueryCounter(queryID[0], GL_TIMESTAMP); 

  /********** Adding Spot *********/
  if(addSplat)
  {
    double sX, sY;
    glfwGetCursorPos(handler->window, &sX, &sY);
    sX = (double) width * sX / (double) handler->width;
    sY = (double) width * (1.0 - sY / (double) handler->height);
    printf("Adding splat at (%f, %f)\n", sX, sY);
    sFact.addSmokeSpot(temperature[READ], density[READ], std::make_tuple(sX, sY));
  }

  /********** Vorticity **********/
  sFact.computeVorticity(velocitiesTexture[READ], vorticity);
  sFact.applyVorticity(velocitiesTexture[READ], vorticity, dt);

  /********** Buoyant Force **********/
  sFact.applyBuoyantForce(velocitiesTexture[READ], temperature[READ], density[READ], dt, 0.5f, 0.5f, 15.0f);

  /********** Convection **********/
  sFact.simpleAdvect(velocitiesTexture[0], velocitiesTexture[0], velocitiesTexture[1], dt);
  sFact.simpleAdvect(velocitiesTexture[1], velocitiesTexture[1], velocitiesTexture[2], - dt);
  sFact.maccormackStep(velocitiesTexture[0], velocitiesTexture[0], velocitiesTexture[2], velocitiesTexture[1], velocitiesTexture[3], dt);

  std::swap(velocitiesTexture[0], velocitiesTexture[3]);

  /********** Divergence Computation **********/
  sFact.divergence(velocitiesTexture[READ], divergenceTexture);

  /********** Poisson Solving with Jacobi **********/
  sFact.solvePressure(divergenceTexture, emptyTexture, pressureTexture[READ], dt);
  for(int k = 0; k < 30; ++k)
  {
    sFact.solvePressure(divergenceTexture, pressureTexture[READ], pressureTexture[WRITE], dt);
    std::swap(pressureTexture[READ], pressureTexture[WRITE]);
  }

  /********** Pressure Projection **********/
  sFact.pressureProjection(pressureTexture[READ], velocitiesTexture[READ], velocitiesTexture[WRITE]);

  std::swap(velocitiesTexture[READ], velocitiesTexture[WRITE]);

  /********** Fields Advection **********/
  sFact.simpleAdvect(velocitiesTexture[0], density[0], density[1], dt);
  sFact.simpleAdvect(velocitiesTexture[1], density[1], density[2], - dt);
  sFact.maccormackStep(velocitiesTexture[0], density[0], density[2], density[1], density[3], dt);

  std::swap(density[0], density[3]);

  sFact.simpleAdvect(velocitiesTexture[0], temperature[0], temperature[1], dt);
  sFact.simpleAdvect(velocitiesTexture[1], temperature[1], temperature[2], - dt);
  sFact.maccormackStep(velocitiesTexture[0], temperature[0], temperature[2], temperature[1], temperature[3], dt);

  std::swap(temperature[0], temperature[3]);

  /********** Copying to render texture **********/
  shared_texture = density[READ];

  glQueryCounter(queryID[1], GL_TIMESTAMP);
  GLint stopTimerAvailable = 0;
  while (!stopTimerAvailable) {
      glGetQueryObjectiv(queryID[1],
                            GL_QUERY_RESULT_AVAILABLE,
                            &stopTimerAvailable);
  }
  // get query results
  glGetQueryObjectui64v(queryID[0], GL_QUERY_RESULT, (GLuint64*) &startTime);
  glGetQueryObjectui64v(queryID[1], GL_QUERY_RESULT, (GLuint64*) &stopTime);
  
  printf("Copy: %f ms\n", (stopTime - startTime) / 1000000.0);
}
