#ifndef GLFWHANDLER_H
#define GLFWHANDLER_H

#include <iostream>

#include "CLUtils.h"

#include "glad.h"
#include <GLFW/glfw3.h>

void CheckOpenGLError(const char* stmt, const char* fname, int line);

#define GL_CHECK(stmt) do { \
        stmt; \
        CheckOpenGLError(#stmt, __FILE__, __LINE__); \
    } while (0)

class GLFWHandler
{
    public:
        GLFWHandler() = delete; 
        GLFWHandler(int width, int height);

        ~GLFWHandler();

        void Init();
        void Run();

        int width, height;

        GLFWwindow* window;
};

#endif //GLFWHANDLER_H
