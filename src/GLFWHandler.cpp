#include "GLFWHandler.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

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

static GLuint compile_shader(const std::string& s, GLenum type)
{
  std::ifstream shader_file(s);
  std::ostringstream shader_buffer;
  shader_buffer << shader_file.rdbuf();
  std::string shader_string = shader_buffer.str();
  const GLchar *shader_source = shader_string.c_str();

  GLuint shader_id = glCreateShader(type);
  glShaderSource(shader_id, 1, &shader_source, NULL);
  glCompileShader(shader_id);

  GLint status;
  glGetShaderiv(shader_id, GL_COMPILE_STATUS, &status);

  if(status == GL_TRUE) return shader_id;
  else
  {
    GLint max_length = 0;
    glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &max_length);
    char buffer[max_length];
    glGetShaderInfoLog(shader_id, max_length, &max_length, buffer);
    glDeleteShader(shader_id);
    std::cout << buffer << std::endl;
    exit(1);
  }
}

GLFWHandler::GLFWHandler(int width, int height)
  : width(width), height(height)
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

  //Compiling shaders
  GLuint vertex_shader = compile_shader("shaders/vertex.glsl", GL_VERTEX_SHADER);
  GLuint fragment_shader = compile_shader("shaders/fragment.glsl", GL_FRAGMENT_SHADER);
}

GLFWHandler::~GLFWHandler()
{
  glfwDestroyWindow(window);
  glfwTerminate();
}

void GLFWHandler::Run()
{
  while (!glfwWindowShouldClose(window))
  {
    glfwPollEvents();
    /*
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
    */
    glfwSwapBuffers(window);
  }
}
