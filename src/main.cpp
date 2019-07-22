#include <iostream>

#include "ProgramOptions.h"

#include "GLFWHandler.h"
#include "SimpleFluid.h"
#include "Smoke.h"

int main(int argc, char** argv)
{
  srand(time(NULL));

  ProgramOptions options = parseOptions(argc, argv);

  GLFWHandler handler(&options);

  SimulationBase *sim;
  switch(options.simType)
  {
    case SPLATS:
    {
      SimpleFluid *sFluid = new SimpleFluid(&options);
      sFluid->Init();
      sFluid->SetHandler(&handler);
      sim = sFluid;
      break;
    }
    case SMOKE:
    {
      Smoke *smoke = new Smoke(&options);
      smoke->Init();
      sim = smoke;
      break;
    }
  }
  
  handler.AttachSimulation(sim);
  handler.RegisterEvent();
  handler.Run();

  return 0;
}
