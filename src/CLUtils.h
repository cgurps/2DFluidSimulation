#ifndef CLUTILS_H
#define CLUTILS_H

#define __CL_ENABLE_EXCEPTIONS

#ifdef __APPLE__
#define CL_SILENCE_DEPRECATION
#include <OpenGL/OpenGL.h>
#endif

#include "cl.h"
#include <stdbool.h>

//https://gist.github.com/allanmac/9328bb2d6a99b86883195f8f78fd1b93
char const * clGetErrorString(cl_int const err);

cl_int cl_assert(cl_int const code, char const * const file, int const line, bool const abort);

#define cl(...)    cl_assert((cl##__VA_ARGS__), __FILE__, __LINE__, true);
#define cl_ok(err) cl_assert(err, __FILE__, __LINE__, true);

#endif //CLUTILS_H
