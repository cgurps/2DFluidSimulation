#include <iostream>

#include "GLFWHandler.h"
#include "OpenCLWrapper.h"
#include "SimpleFluid.h"

int main(int argc, char** argv)
{
    int windowWidth = 800, windowHeight = 600;
    int simWidth = 512, simHeight = 512;

    GLFWHandler handler(windowWidth, windowHeight);
    handler.Init();

    OpenCLWrapper ocl(simWidth, simHeight);

    SimpleFluid sFluid(&ocl);
    sFluid.Init();

    handler.Run();

    return 0;
}
