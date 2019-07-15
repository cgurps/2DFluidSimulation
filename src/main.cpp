#include <iostream>

#include "GLFWHandler.h"
#include "SimpleFluid.h"

int main(int argc, char** argv)
{
    int windowWidth = 600, windowHeight = 600;
    int simWidth = 1024, simHeight = 1024;
    float dt = 0.05f;

    GLFWHandler handler(windowWidth, windowHeight);

    SimpleFluid sFluid(simWidth, simHeight, dt);
    sFluid.Init();
    sFluid.SetHandler(&handler);

    handler.AttachSimulation(&sFluid);
    handler.RegisterEvent();
    handler.Run();

    return 0;
}
