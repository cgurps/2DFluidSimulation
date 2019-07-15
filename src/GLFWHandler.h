#ifndef GLFWHANDLER_H
#define GLFWHANDLER_H

#include <iostream>

#include "GLUtils.h"

class SimulationBase;

class GLFWHandler
{
  public:
    GLFWHandler() = delete; 
    GLFWHandler(int width, int height);

    ~GLFWHandler();

    void AttachSimulation(SimulationBase* sim);
    void RegisterEvent();

    void Run();

    int width, height;

    GLFWwindow* window;

    SimulationBase* simulation;

    int leftMouseButtonLastState = GLFW_RELEASE;
  private:
    GLuint vertex_shader, fragment_shader, shader_program;
    GLuint vao, vbo;
};

#endif //GLFWHANDLER_H
