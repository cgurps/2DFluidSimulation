#include "ProgramOptions.h"
#include "GLFWHandler.h"
#include "SimpleFluid.h"
#include "Smoke.h"
#include "Clouds.h"

int main(int argc, char** argv)
{
  srand(time(NULL));

  ProgramOptions options = parseOptions(argc, argv);

  GLFWHandler handler(&options);

  /*********** SIMULATION CHOICE ***********/
  SimulationBase *sim;
  switch(options.simType)
  {
    case SPLATS:
    {
      sim = new SimpleFluid(&options, &handler);
      break;
    }
    case SMOKE:
    {
      sim = new Smoke(&options, &handler);
      break;
    }
    case CLOUDS:
    {
      sim = new Clouds(&options, &handler);
      break;
    }
  }
  
  handler.AttachSimulation(sim);
  handler.RegisterEvent();
  handler.Run();

  return 0;
}
