#include "GLFWHandler.h"

void CheckOpenGLError(const char* stmt, const char* fname, int line)
{
    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
    {
      std::string error;
      switch(err)
      {
        case GL_INVALID_OPERATION:      error="INVALID_OPERATION";      break;
        case GL_INVALID_ENUM:           error="INVALID_ENUM";           break;
        case GL_INVALID_VALUE:          error="INVALID_VALUE";          break;
        case GL_OUT_OF_MEMORY:          error="OUT_OF_MEMORY";          break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:  error="INVALID_FRAMEBUFFER_OPERATION";  break;
      }

      printf("OpenGL error %s, at %s:%i - for %s\n", error.c_str(), fname, line, stmt);
      exit(1);
    }
}

static void glfwErrorCallback(int error, const char* description)
{
    std::cerr << "Error: " << description << std::endl;
}

static void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

GLFWHandler::GLFWHandler(int width, int height)
    : width(width), height(height)
{
}

GLFWHandler::~GLFWHandler()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

void GLFWHandler::Init()
{
    glfwSetErrorCallback(glfwErrorCallback);

    if(!glfwInit())
        std::cerr << "GLFW Init failed!" << std::endl;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Fluid Simulation", NULL, NULL);
    if(!window)
        std::cerr << "GLFW Window creation failed!" << std::endl;

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        std::cerr << "Failed to initialize GLAD" << std::endl;

    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
}

void GLFWHandler::Run()
{
    while (!glfwWindowShouldClose(window))
  {
    glfwPollEvents();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.2, 0.2, 0.2, 0.0);
    glEnable(GL_DEPTH_TEST);
    // bind shader
    glUseProgram(rparams.prg);
    // get uniform locations
    int mat_loc = glGetUniformLocation(rparams.prg, "matrix");
    int tex_loc = glGetUniformLocation(rparams.prg, "tex");
    // bind texture
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(tex_loc, 0);
    glBindTexture(GL_TEXTURE_2D, rparams.tex);
    glGenerateMipmap(GL_TEXTURE_2D);
    // set project matrix
    glUniformMatrix4fv(mat_loc, 1, GL_FALSE, matrix);
    // now render stuff
    glBindVertexArray(rparams.vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glfwSwapBuffers(window);
  }
}
