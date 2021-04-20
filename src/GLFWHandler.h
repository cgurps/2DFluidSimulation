#pragma once

/**
 * @dir src
 * @brief The source directory
 */

/**
 * @file GLFWHandler.h
 * @author Thomas Caissard (\c thomas.caissard@gmail.com)
 * @date 2019/08/05
 * @brief Handler for OpenGL main rendering loop
 */


#include <iostream>

#include "GLUtils.h"
#include "ProgramOptions.h"

/**
 *  @ref SimulationBase
 */
class SimulationBase;

/**
 * @class GLFWHandler
 * @brief Initilization and main loop for OpenGL
 */
class GLFWHandler
{
  public:
    /**
     * Default constructor deletion
     */
    GLFWHandler() = delete;

    /**
     * Default constructor
     * @param options Program options
     */
    GLFWHandler(ProgramOptions *options);

    /**
     * Default destructor
     */
    ~GLFWHandler();

    /**
     * Setter for the simulation class
     * @param sim Pointer to the simulation
     */
    void attachSimulation(SimulationBase* sim);

    /**
     * Main rendering loop
     */
    void run();

    /**
     * The program options
     */
    ProgramOptions *options;

    /**
     * The rendering window
     */
    GLFWwindow* window;

    /**
     * Pointer to the simulation class
     */
    SimulationBase* simulation;

    /**
     * Left button state memory
     */
    int leftMouseButtonLastState = GLFW_RELEASE;
  private:
    /**
     * Register all application events
     */
    void registerEvent();

    /**
     * Shader Program ID
     */
    GLuint shader_program;

    /**
     * EBO ID
     */
    GLuint ebo;
};
