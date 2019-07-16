#include "Smoke.h"
#include "GLUtils.h"

#include <string>
#include <sstream>
#include <fstream>
#include <cmath>

void Smoke::Init()
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

  //Velocities init function
  auto f1 = [this](int x, int y)
      {
        float xf = (float) x - 0.5f * (float) this->width;
        float yf = (float) y - 0.5f * (float) this->height;
        float norm = std::sqrt(xf * xf + yf * yf);
        float vx = (norm < 1e-5) ? 0.0f : - 200.0f * yf / norm;
        float vy = (norm < 1e-5) ? 0.0f :   200.0f * xf / norm;
        return std::make_tuple(50.0f, 0.0f,
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
  fillTextureWithFunctor(velocitiesTexture[0], width, height, f);

  divergenceTexture = createTexture2D(width, height);
  fillTextureWithFunctor(divergenceTexture, width, height, f);

  vorticity = createTexture2D(width, height);
  fillTextureWithFunctor(vorticity, width, height, f);

  pressureTexture[0] = createTexture2D(width, height);
  pressureTexture[1] = createTexture2D(width, height);
  fillTextureWithFunctor(pressureTexture[0], width, height, f);

  emptyTexture = createTexture2D(width, height);
  fillTextureWithFunctor(emptyTexture, width, height,
      [](int x, int y)
      {
        return std::make_tuple(0.0f, 0.0f, 0.0f, 0.0f);
      });
}

void Smoke::SetHandler(GLFWHandler* hand)
{
  handler = hand;
}

void Smoke::AddSplat()
{
}

void Smoke::AddMultipleSplat(const int nb)
{
}

void Smoke::RemoveSplat()
{
}

void Smoke::Update()
{
  GLint64 startTime, stopTime;
  GLuint queryID[2];

  // generate two queries
  glGenQueries(2, queryID);
  glQueryCounter(queryID[0], GL_TIMESTAMP); 

  /********** Adding Splat *********/
  auto rd = []() -> double
  {
    return (double) rand() / (double) RAND_MAX;
  };

  int x = width / 2;
  int y = 75;
  float d = rd();
  /*
  sFact.addSplat(density[READ],           std::make_tuple(x, y), std::make_tuple(0.0f, 0.41f, 0.55f), 0.1f);
  sFact.addSplat(temperature[READ],       std::make_tuple(x, y), std::make_tuple(rd() * 20.0f + 10.0f, 0.0f, 0.0f), 2.0f);
  sFact.addSplat(velocitiesTexture[READ], std::make_tuple(x, y), std::make_tuple(2.0f * rd() - 1.0f, 0.0f, 0.0f), 75.0f);
  */

  sFact.addSplat(density[READ],           std::make_tuple(x, 450), std::make_tuple(0.0f, 0.41f, 0.55f), 0.1f);
  sFact.addSplat(temperature[READ],       std::make_tuple(x, 450), std::make_tuple(rd() * 5.0f - 10.0f, 0.0f, 0.0f), 2.0f);
  sFact.addSplat(velocitiesTexture[READ], std::make_tuple(x, 450), std::make_tuple(2.0f * rd() - 1.0f, 0.0f, 0.0f), 15.0f);

  /********** Vorticity **********/
  sFact.copy(emptyTexture, vorticity);
  sFact.computeVorticity(velocitiesTexture[READ], vorticity);
  sFact.applyVorticity(velocitiesTexture[READ], vorticity, dt);

  /********** Buoyant Force **********/
  sFact.applyBuoyantForce(velocitiesTexture[READ], temperature[READ], density[READ], dt, 0.25f, 0.1f, 15.0f);

  /********** Convection **********/
  sFact.simpleAdvect(velocitiesTexture[0], velocitiesTexture[0], velocitiesTexture[1],   dt);
  sFact.simpleAdvect(velocitiesTexture[1], velocitiesTexture[1], velocitiesTexture[2], - dt);
  sFact.maccormackStep(velocitiesTexture[0], velocitiesTexture[0], velocitiesTexture[2], velocitiesTexture[1], velocitiesTexture[3], dt);

  std::swap(velocitiesTexture[0], velocitiesTexture[3]);

  /********** Divergence Computation **********/
  sFact.copy(emptyTexture, divergenceTexture);
  sFact.divergence(velocitiesTexture[READ], divergenceTexture);

  /********** Poisson Solving with Jacobi **********/
  sFact.copy(emptyTexture, pressureTexture[READ]);
  for(int k = 0; k < 25; ++k)
  {
    sFact.solvePressure(divergenceTexture, pressureTexture[READ], pressureTexture[WRITE]);
    std::swap(pressureTexture[READ], pressureTexture[WRITE]);
  }

  /********** Pressure Projection **********/
  sFact.pressureProjection(pressureTexture[READ], velocitiesTexture[READ], velocitiesTexture[WRITE]);

  std::swap(velocitiesTexture[READ], velocitiesTexture[WRITE]);

  /********** Fields Advection **********/
  sFact.simpleAdvect(velocitiesTexture[0], density[0], density[1],   dt);
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
  
  printf("\rCopy: %f ms", (stopTime - startTime) / 1000000.0);
  fflush(stdout);
}
