#include <iostream>

#include "GLFWHandler.h"
#include "OpenCLWrapper.h"
#include "SimpleFluid.h"

int main(int argc, char** argv)
{
    int windowWidth = 800, windowHeight = 600;
    int simWidth = 1024, simHeight = 1024;

    GLFWHandler handler(windowWidth, windowHeight);

    OpenCLWrapper ocl(simWidth, simHeight, &handler);

    SimpleFluid sFluid(&ocl);
    sFluid.Init();

    handler.AttachSimulation(&sFluid);
    handler.Run();

    return 0;
}
