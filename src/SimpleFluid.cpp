#include "SimpleFluid.h"
#include "GLUtils.h"

#include <string>
#include <sstream>
#include <fstream>
#include <cmath>
#include <functional>
#include <tuple>

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
  GL_CHECK( glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, data) );

  delete [] data;
}

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

  dummyTexture[READ] = createTexture2D(width, height);
  dummyTexture[WRITE] = createTexture2D(width, height);
  fillTextureWithFunctor(dummyTexture[0], width, height, 
      [](int x, int y)
      {
        if(x % 100 < 50 && y % 100 < 50) return std::make_tuple(1.0f, 1.0f, 1.0f, 1.0f);  
        else return std::make_tuple(0.0f, 0.0f, 0.0f, 0.0f);
      });

  velocitiesTexture[READ] = createTexture2D(width, height);
  velocitiesTexture[WRITE] = createTexture2D(width, height);
  auto f = [this](int x, int y)
      {
        return std::make_tuple(std::sin(2.0 * M_PI * (double) y / this->height), std::sin(2.0 * M_PI * (double) x / this->width), 0.0f, 0.0f);
      };
  fillTextureWithFunctor(velocitiesTexture[0], width, height, f);

  copyProgram = compileAndLinkShader("shaders/simulation/copy.compute", GL_COMPUTE_SHADER);

  advectProgram = compileAndLinkShader("shaders/simulation/advect.compute", GL_COMPUTE_SHADER);

  GLint location;
  GL_CHECK( glUseProgram(advectProgram) );
  location = glGetUniformLocation(advectProgram, "texSize");
  GL_CHECK( glUniform2i(location, width, height) );
  location = glGetUniformLocation(advectProgram, "dt");
  GL_CHECK( glUniform1f(location, 2.0f) );
}

void SimpleFluid::Update()
{
  GL_CHECK( glUseProgram(advectProgram) );
  GL_CHECK( glBindImageTexture(0, dummyTexture[WRITE], 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F) );
  GL_CHECK( glActiveTexture(GL_TEXTURE1) );
  GL_CHECK( glBindTexture(GL_TEXTURE_2D, dummyTexture[READ]) );
  GL_CHECK( glActiveTexture(GL_TEXTURE2) );
  GL_CHECK( glBindTexture(GL_TEXTURE_2D, velocitiesTexture[READ]) );
  GL_CHECK( glDispatchCompute(width, height, 1) );

  std::swap(dummyTexture[0], dummyTexture[1]);

  GL_CHECK( glUseProgram(copyProgram) );
  GL_CHECK( glBindImageTexture(0, shared_texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F) );
  GL_CHECK( glBindImageTexture(1, dummyTexture[READ], 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F) );
  GL_CHECK( glDispatchCompute(width, height, 1) );
}
