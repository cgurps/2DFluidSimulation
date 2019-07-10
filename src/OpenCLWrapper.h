#ifndef OPENCLWRAPPER_H
#define OPENCLWRAPPER_H

#include "CLUtils.h"

class OpenCLWrapper
{
  public:
    OpenCLWrapper() = delete;
    OpenCLWrapper(int width, int height);

    int width, height;

    cl::Context context;
    cl::Device device;
    cl::CommandQueue command_queue;
};

#endif //OPENCLWRAPPER_H
