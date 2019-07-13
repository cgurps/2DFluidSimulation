#ifndef CLUTILS_H
#define CLUTILS_H

#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_TARGET_OPENCL_VERSION 120

#ifdef __APPLE__
#define CL_SILENCE_DEPRECATION
#include <OpenGL/OpenGL.h>
#endif

#include "cl2.h"
#include <stdbool.h>

//https://gist.github.com/allanmac/9328bb2d6a99b86883195f8f78fd1b93
char const * clGetErrorString(cl_int const err);

cl_int cl_assert(cl_int const code, char const * const file, int const line, bool const abort);

#define cl(...)    cl_assert((cl##__VA_ARGS__), __FILE__, __LINE__, true);
#define cl_ok(err) cl_assert(err, __FILE__, __LINE__, true);

#endif //CLUTILS_H
