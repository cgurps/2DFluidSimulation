#ifndef PROGRAMOPTIONS_H
#define PROGRAMOPTIONS_H

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/errors.hpp>

enum SimulationType
{
  SPLATS,
  SMOKE,
  CLOUDS
};

std::ostream& operator<<(std::ostream& os, const SimulationType& type);
std::istream& operator>>(std::istream& os, SimulationType& type);

struct ProgramOptions
{
  unsigned windowWidth, windowHeight;

  SimulationType simType;
  unsigned simWidth, simHeight;
  int RKorder;
  float dt;

  bool exportImages;
};

ProgramOptions parseOptions(int argc, char* argv[]);

#endif //PROGRAMOPTIONS_H
