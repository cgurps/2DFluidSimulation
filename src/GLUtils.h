#include "glad.h"

#ifdef __unix__
#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_GLX
#endif

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <string>

void CheckOpenGLError(const char* stmt, const char* fname, int line);

#define GL_CHECK(stmt) do { \
  stmt; \
  CheckOpenGLError(#stmt, __FILE__, __LINE__); \
} while (0)

GLuint createTexture2D(const int width, const int height);
GLuint compileShader(const std::string& s, GLenum type);
GLuint compileAndLinkShader(const std::string& s, GLenum type);
