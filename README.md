<p align="center">
  <img width="350" height="350" src="images/simple_fluid.png">
  <img width="350" height="350" src="images/splats.png">
</p>

# 2D Fluid Simulator using OpenGL

This project is an implementation of an eulerian fluid simulation on GPU using OpenGL 4.3 compute shaders capabilities.

## Getting Started
You will need OpenGL with a version above 4.3 in order to get compute shader capabilities. You will also need [GLFW](https://www.glfw.org/) installed on your machine. To project uses CMake to generate the Makefile needed for the compilation. You can use these commands to build the executable

```
mkdir build
cd build
cmake ..
make
```
