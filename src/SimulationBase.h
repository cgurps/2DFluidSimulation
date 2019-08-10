#ifndef SIMULATIONBASE_H
#define SIMULATIONBASE_H

/**
 * @dir src
 * @brief The source directory
 */

/**
 * @file SimulationBase.h
 * @author Thomas Caissard (\c thomas.caissard@gmail.com)
 * @date 2019/08/05
 * @brief The pure virtual class representing the base simulation
 */

#include "GLUtils.h"
#include "ProgramOptions.h"
#include "GLFWHandler.h"
#include "SimulationFactory.h"

/**
 * @class SimulationBase
 * @brief The pure virtual class of the simulation
 */
class SimulationBase
{
  public:
    /**
     * Default constructor of the mother class
     * @param options the program options
     * @param handler the OpenGL handler
     */
    SimulationBase(ProgramOptions *options, GLFWHandler *handler)
      : options(options), handler(handler), sFact(SimulationFactory(options))
    {}

    /**
     * Default destructor
     */
    virtual ~SimulationBase() {}

    /**
     * Pure virtual initialization of the simulation.
     */
    virtual void Init() = 0;

    /**
     * Pure virtual Update of the simulation
     */
    virtual void Update() = 0;

    /**
     * Pure virtual single splat adding for the simulation
     */
    virtual void AddSplat() = 0;

    /**
     * Pure virtual multiple splat adding for the simulation
     */
    virtual void AddMultipleSplat(const int nb) = 0;

    /**
     * Pure virtual splat removal for the simulation
     */
    virtual void RemoveSplat() = 0;

    /**
     * The program options
     */
    ProgramOptions *options;

    /**
     * The shared texture for the OpenGL rendering
     */
    GLuint shared_texture;

    /**
     * The OpenGL renderer class
     */
    GLFWHandler *handler; 

    /**
     * Simulation factory class
     */
    SimulationFactory sFact;
};

#endif //SIMULATIONBASE_H
