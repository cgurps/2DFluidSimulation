#include "GLFWHandler.h"
#include "SimulationBase.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

static void glfwErrorCallback(int error, const char* description)
{
  std::cerr << "Error(" << error << "): " << description << std::endl;
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

  printf("OpenGL version supported by this platform (%s): \n",
      glGetString(GL_VERSION));
  printf("Supported GLSL version is %s.\n",
      glGetString(GL_SHADING_LANGUAGE_VERSION));

  glfwSwapInterval(1);

  GL_CHECK(glGenVertexArrays(1, &vao));
  GL_CHECK(glBindVertexArray(vao));

  std::cout << "VAO Created." << std::endl;

  float vertices[] = {
    -1.0f, -1.0f,
    -1.0f,  1.0f,
    1.0f, -1.0f,
    1.0f,  1.0f
  };

  GL_CHECK(glGenBuffers(1, &vbo));
  GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vbo));
  GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));

  std::cout << "VBO Created." << std::endl;

  vertex_shader = compile_shader("shaders/vertex.glsl", GL_VERTEX_SHADER);
  fragment_shader = compile_shader("shaders/fragment.glsl", GL_FRAGMENT_SHADER);

  shader_program = glCreateProgram();
  GL_CHECK(glAttachShader(shader_program, vertex_shader));
  GL_CHECK(glAttachShader(shader_program, fragment_shader));
  GL_CHECK(glBindFragDataLocation(shader_program, 0, "out_color") );
  GL_CHECK(glLinkProgram(shader_program));
  GL_CHECK(glUseProgram(shader_program));

  GLint posAttrib = glGetAttribLocation(shader_program, "position");
  GL_CHECK(glEnableVertexAttribArray(posAttrib));
  GL_CHECK(glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0));
}

GLFWHandler::~GLFWHandler()
{
  glfwDestroyWindow(window);
  glfwTerminate();
}

void GLFWHandler::AttachSimulation(SimulationBase* sim)
{
  simulation = sim;
}

void GLFWHandler::Run()
{
  const float matrix[16] =
  {
      1.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 1.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f
  };

  while (!glfwWindowShouldClose(window))
  {
    simulation->Update();

    glfwPollEvents();

    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    GL_CHECK(glClearColor(0.0, 0.0, 0.0, 0.0));
    GL_CHECK(glEnable(GL_DEPTH_TEST));
    // bind shader
    GL_CHECK(glUseProgram(shader_program));
    // get uniform locations
    int mat_loc = glGetUniformLocation(shader_program, "MVP");
    int tex_loc = glGetUniformLocation(shader_program, "tex");
    // bind texture
    GL_CHECK(glActiveTexture(GL_TEXTURE0));
    GL_CHECK(glUniform1i(tex_loc, 0));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, simulation->shared_texture));
    //glGenerateMipmap(GL_TEXTURE_2D);
    // set project matrix
    GL_CHECK(glUniformMatrix4fv(mat_loc, 1, GL_FALSE, matrix));
    // now render stuff
    GL_CHECK(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
    /*
    GL_CHECK(glBindVertexArray(vao));
    GL_CHECK(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));
    GL_CHECK(glBindVertexArray(0));
    */

    glfwSwapBuffers(window);
  }
}
