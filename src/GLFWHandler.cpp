#include "GLFWHandler.h"
#include "SimulationBase.h"
#include "lodepng.h"

#include <chrono>

/********** Event Callbacks **********/
static void glfwErrorCallback(int error, const char* description)
{
  std::cerr << "Error(" << error << "): " << description << std::endl;
}

static void mouseCallback(GLFWwindow* window, int button, int action, int)
{ 
  if(button == GLFW_MOUSE_BUTTON_LEFT)
  {
    GLFWHandler *handler = (GLFWHandler*) glfwGetWindowUserPointer(window);
    if(action == GLFW_PRESS || handler->leftMouseButtonLastState == GLFW_PRESS)
      handler->simulation->AddSplat();
    else if(action == GLFW_RELEASE)
      handler->simulation->RemoveSplat();
  }
}

static void keyCallback(GLFWwindow* window, int key, int, int action, int)
{
  if(key == GLFW_KEY_E && action == GLFW_PRESS)
  {
    GLFWHandler *handler = (GLFWHandler*) glfwGetWindowUserPointer(window);
    handler->simulation->AddMultipleSplat(100);
  }

  if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
  {
    glfwSetWindowShouldClose(window, GLFW_TRUE); 
  }
}

static void windowResizeCallback(GLFWwindow* window, int width, int height)
{
  GLFWHandler *handler = (GLFWHandler*) glfwGetWindowUserPointer(window);
  handler->options->windowWidth = width;
  handler->options->windowHeight = height;
}
/*************************************/

GLFWHandler::GLFWHandler(ProgramOptions *options)
  : options(options)
{
  glfwSetErrorCallback(glfwErrorCallback);

  if(!glfwInit())
  {
    std::cerr << "GLFW Init failed!" << std::endl;
    std::exit(1);
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

  window = glfwCreateWindow(options->windowWidth, options->windowHeight, "Fluid Simulation", NULL, NULL);
  if(!window)
  {
    std::cerr << "GLFW Window creation failed!" << std::endl;
    std::exit(1);
  }

  glfwMakeContextCurrent(window);

  registerEvent();

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    std::cerr << "Failed to initialize GLAD" << std::endl;
    std::exit(1);
  }

  printf("OpenGL version supported by this platform (%s)\n",
      glGetString(GL_VERSION));
  printf("Supported GLSL version is %s.\n",
      glGetString(GL_SHADING_LANGUAGE_VERSION));

  glfwSwapInterval(1);

  GLint flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
  if(flags & GL_CONTEXT_FLAG_DEBUG_BIT)
  {
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(MessageCallback, 0);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
  }
  else
  {
    std::cout << "Failed to init debug context" << std::endl;
  }

  /********** Configuring pipeline *********/
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  float vertices[] = 
  {
      // positions     // texture coords
      1.0f,   1.0f,    1.0f, 1.0f,
      1.0f, - 1.0f,    1.0f, 0.0f,
    - 1.0f,   1.0f,    0.0f, 1.0f,
    - 1.0f, - 1.0f,    0.0f, 0.0f
  };

  unsigned int indices[] =
  {
    0, 1, 2,
    3, 1, 2
  };

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*) 0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
  glEnableVertexAttribArray(1);
  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  /********** Linking shaders **********/
  vertex_shader = compileShader("shaders/vertex.glsl", GL_VERTEX_SHADER);
  fragment_shader = compileShader("shaders/fragment.glsl", GL_FRAGMENT_SHADER);

  shader_program = glCreateProgram();
  glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, fragment_shader);
  glBindFragDataLocation(shader_program, 0, "outColor");
  glLinkProgram(shader_program);
  glUseProgram(shader_program);
}

GLFWHandler::~GLFWHandler()
{
  glfwDestroyWindow(window);
  glfwTerminate();
}

void GLFWHandler::attachSimulation(SimulationBase* sim)
{
  simulation = sim;
  simulation->Init();
}

void GLFWHandler::registerEvent()
{
  glfwSetWindowUserPointer(window, this);  
  glfwSetMouseButtonCallback(window, mouseCallback);
  glfwSetKeyCallback(window, keyCallback);
  glfwSetWindowSizeCallback(window, windowResizeCallback);
}

void GLFWHandler::run()
{
  int tex_loc = glGetUniformLocation(shader_program, "tex");
  glUseProgram(shader_program);
  glUniform1i(tex_loc, 0);

  /********** Linear Sampler for rendering the texture **********/
  GLuint linearSampler;
  glGenSamplers(1, &linearSampler);
  glSamplerParameteri(linearSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glSamplerParameteri(linearSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glSamplerParameteri(linearSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glSamplerParameteri(linearSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glSamplerParameteri(linearSampler, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  /********** Compute shader timings ***********/
  GLint64 startTime, stopTime;
  GLuint queryID[2];

  std::chrono::high_resolution_clock::time_point 
    start = std::chrono::high_resolution_clock::now();
  double sumOfDeltaT = 0.0;

  /********** Array of images for the export **********/
  std::vector<unsigned char*> buffers;

  /********** Rendering & Simulation Loop ***********/
  while (!glfwWindowShouldClose(window))
  {
    /********** Generating queries for timing **********/
    glGenQueries(2, queryID);
    glQueryCounter(queryID[0], GL_TIMESTAMP); 

    /********** Updating the simulation **********/
    simulation->Update();

    /********** Compute shader execution time *********/
    glQueryCounter(queryID[1], GL_TIMESTAMP);
    GLint stopTimerAvailable = 0;
    while (!stopTimerAvailable) {
        glGetQueryObjectiv(queryID[1],
                              GL_QUERY_RESULT_AVAILABLE,
                              &stopTimerAvailable);
    }
    glGetQueryObjectui64v(queryID[0], GL_QUERY_RESULT, (GLuint64*) &startTime);
    glGetQueryObjectui64v(queryID[1], GL_QUERY_RESULT, (GLuint64*) &stopTime);

    std::chrono::high_resolution_clock::time_point 
      current = std::chrono::high_resolution_clock::now();
    sumOfDeltaT += options->dt;
    std::chrono::duration<double, std::milli> timeSpan = current - start;
  
    printf("\rDelta from real time: %.4f s (%.3f ms, %.5f dt)"
        , sumOfDeltaT - timeSpan.count() / 1000.0
        , (stopTime - startTime) / 1000000.0
        , options->dt);
    fflush(stdout);

    glfwPollEvents();

    /********** Rendering the texture **********/
    int displayW, displayH;
    glfwGetFramebufferSize(window, &displayW, &displayH);
    glViewport(0, 0, displayW, displayH);

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shader_program);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, simulation->shared_texture);
    glBindSampler(0, linearSampler);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glBindSampler(0, 0);

    /********** Saving texture for the export **********/
    if(options->exportImages)
    {
      unsigned char *colors = new unsigned char[3 * options->simWidth * options->simHeight];
      glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, colors);
      buffers.push_back(colors);
    }

    glfwSwapBuffers(window);
  }

  /********** Exporting the frames to the disk **********/
  if(options->exportImages)
  {
    auto storeImage = [](const char* path, const unsigned char* colors, unsigned int w, unsigned int h)
    {
      std::cout << "Storing " << path << std::endl;

      unsigned char reversed[3 * w * h];
      for(unsigned int x = 0; x < w; ++x)
      {
        for(unsigned int y = 0; y < h; ++y)
        {
          reversed[3 * (y * w + x)] = colors[ 3 * ( (h - y - 1) * w + x )];
          reversed[3 * (y * w + x) + 1] = colors[3 * ( (h - y - 1) * w + x ) + 1];
          reversed[3 * (y * w + x) + 2] = colors[3 * ( (h - y - 1) * w + x ) + 2];
        }
      }

      unsigned error = lodepng_encode24_file(path, reversed, w, h);
      if(error) std::cout << "Encode Error: " << error << ": " << lodepng_error_text(error) << std::endl;
      delete[] colors;
    };

    for(unsigned int i = 0; i < buffers.size(); ++i)
    {
      char path[1024];
      if(i < 10)
        sprintf(path, "frame_00%i.png", i);
      else if (i < 100)
        sprintf(path, "frame_0%i.png", i);
      else
        sprintf(path, "frame_%i.png", i); 

      storeImage(path, buffers[i], options->simWidth, options->simHeight);
    }
  }
}
