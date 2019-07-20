#include <iostream>

#include "GLFWHandler.h"
#include "SimpleFluid.h"
#include "Smoke.h"

int main(int argc, char** argv)
{
  srand(time(NULL));

  int windowWidth = 1000, windowHeight = 1000;
  int simWidth = 1024, simHeight = 1024;
  float dt = 0.1f;

  GLFWHandler handler(windowWidth, windowHeight);

  SimpleFluid sFluid(simWidth, simHeight, dt);
  sFluid.Init();
  sFluid.SetHandler(&handler);

  Smoke smoke(simWidth, simHeight, dt);
  smoke.Init();
  smoke.SetHandler(&handler);

  handler.AttachSimulation(&sFluid);
  handler.RegisterEvent();
  handler.Run();

  return 0;
}
