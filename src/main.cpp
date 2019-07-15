#include <iostream>

#include "GLFWHandler.h"
#include "SimpleFluid.h"

int main(int argc, char** argv)
{
    int windowWidth = 600, windowHeight = 600;
    int simWidth = 1024, simHeight = 1024;

    GLFWHandler handler(windowWidth, windowHeight);

    SimpleFluid sFluid(simWidth, simHeight);
    sFluid.Init();

    handler.AttachSimulation(&sFluid);
    handler.Run();

    return 0;
}
