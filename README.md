<p align="center">
  <img width="400" height="400" src="images/splats.png">
  <img width="400" height="400" src="images/smoke.png">
</p>

# 2D Fluid Simulator using OpenGL

This project is an implementation of an eulerian fluid simulation on GPU using OpenGL 4.3 compute shaders capabilities.

## Getting Started
You will first need to clone the repository
```
git clone https://github.com/cgurps/2DFluidSimulation.git [PROJECT_FOLDER]
```
and then init the submodules
```
cd [PROJECT_FOLDER]
git submodule update --init
```
To compile the project, you will need OpenGL with a version above 4.3 in order to get compute shader capabilities. You will also need [Boost](https://www.boost.org/) installed on your machine. To project uses CMake to generate the Makefile needed for the compilation. You can use these commands to build the executable

```
mkdir build
cd build
cmake ..
make
```

You can query the program options using `-h`.

## Numerical Scheme
We solve the Navier-Stokes equation for incompressible fluids:
<p align="center">
  <img src="images/equations/NS.png">
</p>
As every eulerian approaches, the quantites (velocties, pressure, divergence, curl and so on) are stored in a square grid. The advection step uses a semi-Lagragian approach. The particle position is computed using a Runge Kutta method of order 4. The next step adds forces to the velocity field (such as vorticity confinement or buoyancy). After that, the intermediate field is made incompressible using a projection method based on the Helmholtz-Hodge decomposition. I solve the associated poisson equation using the Jacobi method. The time step is computed at each iteration with
<p align="center">
  <img src="images/equations/CFL.png">
</p>
The maximum of the velocity field is computed through a reduce method on the GPU.

## Implementation
Each quantities is represented by a texture of 16bits floating points on the GPU. For exact texels query, I use the texelFetch method (which runs faster than using texture2D) and then handle the boundary cases by hand. The bilinear interpolation for the advection step is also computed by hand for better accuracy. The implementation contains three main classes:
1. `GLFWHandler` is the GLFW wrapper that contains the OpenGL initilization and the main program loop
2. `SimulationBase` which is a pure virtual function that gives the interface for the simulation. The main loop of the program accesses the `shared_texture` variable and display the associated texture on screen. This is where the various textures are created and stored.
3. `SimulationFactory` which contains helpers for computing steps of the simulation (like advection, pressure projection, etc). This class does not allocate GPU memory, but is instead feeded by the simulation loop.

If you (ever) wish to play around this simulation, you should create a new class that inherits from `SimulationBase` and uses the `SimulationFactory` to compute whatever you need to compute. This new class must overload `Init()`, `Update()`, `AddSplat()`, `AddSplat(const int)` and `RemoveSplat()` for the simulation to work.

### Note on the Jacobi method
I implemented a variation on the original Jacobi method described in [Harris et al.](https://users.cg.tuwien.ac.at/bruckner/ss2004/seminar/A3b/Harris2003%20-%20Simulation%20of%20Cloud%20Dynamics%20on%20Graphics%20Hardware.pdf) called the Red-Black Jacobi method. The idea is to pack four values into a single texel. The packing is done both on the divergence and on the pressure values. The next figure (reproduced from the Figure 5 of [Harris et al.](https://users.cg.tuwien.ac.at/bruckner/ss2004/seminar/A3b/Harris2003%20-%20Simulation%20of%20Cloud%20Dynamics%20on%20Graphics%20Hardware.pdf)) shows the process
<p align="center">
  <img src="images/RB.png">
</p>
The resulting texture has half the size of the original one. Then, the Jacobi iteration updates first the black values (which only depend on the red one) and second the red values (using the computed black values). This almost divides the number of texel fetches by two, hence we can obtain the same order of convergence in approximatively half the time! The actual tricky part is to pack the divergence into one texel. This is done in one shader pass by grabing numerous adjacent values of the current texel (see the file `divRB.comp`).

## References
1. [@](http://jamie-wong.com/2016/08/05/webgl-fluid-simulation/): a simple tutorial on fluid simulation
2. [@](https://www.cs.ubc.ca/~rbridson/fluidsimulation/fluids_notes.pdf): this awesome books covers a lot of techniques for simulating fluids (classic!)
3. [@](https://www.cs.cmu.edu/~kmcrane/Projects/GPUFluid/): from GPU gems 3
4. [@](https://cg.informatik.uni-freiburg.de/intern/seminar/gridFluids_GPU_Gems.pdf): from GPU gems 1

## Nice github projects
1. [tunabrain/gpu-fluid](https://github.com/tunabrain/gpu-fluid): 2D fluid simulation on the GPU using an hydrid approach (FLIP)
2. [PavelDoGreat/WebGL-Fluid-Simulation](https://github.com/PavelDoGreat/WebGL-Fluid-Simulation): online fluid simulation

