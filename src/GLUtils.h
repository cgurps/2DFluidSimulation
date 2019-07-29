#include "glad.h"

#ifdef __unix__
#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_GLX
#endif

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <string>

#include <boost/regex.hpp>

void APIENTRY MessageCallback(GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const void* userParam);

GLuint createTexture2D(const unsigned width, const unsigned height);
GLuint compileShader(const std::string& s, GLenum type);
GLuint compileAndLinkShader(const std::string& s, GLenum type);
std::string preprocessIncludes(const std::string source, const std::string shader_path, int level);
