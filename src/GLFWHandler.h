#ifndef GLFWHANDLER_H
#define GLFWHANDLER_H

#include <iostream>

#include "CLUtils.h"

#include "GLUtils.h"

class SimulationBase;

class GLFWHandler
{
  public:
    GLFWHandler() = delete; 
    GLFWHandler(int width, int height);

    ~GLFWHandler();

    void AttachSimulation(SimulationBase* sim);

    void Run();

    int width, height;

    GLFWwindow* window;

  private:
    GLuint vertex_shader, fragment_shader, shader_program;
    GLuint vao, vbo;

    SimulationBase* simulation;
};

#endif //GLFWHANDLER_H
