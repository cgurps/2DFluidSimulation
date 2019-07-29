#ifndef GLFWHANDLER_H
#define GLFWHANDLER_H

#include <iostream>

#include "GLUtils.h"
#include "ProgramOptions.h"

class SimulationBase;

class GLFWHandler
{
  public:
    GLFWHandler() = delete; 
    GLFWHandler(ProgramOptions *options);

    ~GLFWHandler();

    void attachSimulation(SimulationBase* sim);

    void run();

    ProgramOptions *options;

    GLFWwindow* window;

    SimulationBase* simulation;

    int leftMouseButtonLastState = GLFW_RELEASE;
  private:
    void registerEvent();

    GLuint vertex_shader, fragment_shader, shader_program;
    GLuint vao, vbo, ebo;
};

#endif //GLFWHANDLER_H
