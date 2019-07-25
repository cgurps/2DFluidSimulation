#include "GLFWHandler.h"
#include "SimulationBase.h"
#include "lodepng.h"

static void glfwErrorCallback(int error, const char* description)
{
  std::cerr << "Error(" << error << "): " << description << std::endl;
}

GLFWHandler::GLFWHandler(ProgramOptions *options)
  : options(options)
{
  glfwSetErrorCallback(glfwErrorCallback);

  if(!glfwInit())
    std::cerr << "GLFW Init failed!" << std::endl;

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

  window = glfwCreateWindow(options->windowWidth, options->windowHeight, "Fluid Simulation", NULL, NULL);
  if(!window)
    std::cerr << "GLFW Window creation failed!" << std::endl;

  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    std::cerr << "Failed to initialize GLAD" << std::endl;

  printf("OpenGL version supported by this platform (%s)\n",
      glGetString(GL_VERSION));
  printf("Supported GLSL version is %s.\n",
      glGetString(GL_SHADING_LANGUAGE_VERSION));

  glfwSwapInterval(1);

  GL_CHECK(glGenVertexArrays(1, &vao));
  GL_CHECK(glBindVertexArray(vao));

  std::cout << "VAO Created." << std::endl;

  /*********** VBO ***********/
  /** These corresponds to the 
   * 2D screen coordinates of the texture 
   **/
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

  vertex_shader = compileShader("shaders/vertex.glsl", GL_VERTEX_SHADER);
  fragment_shader = compileShader("shaders/fragment.glsl", GL_FRAGMENT_SHADER);

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
  simulation->Init();
}

/********** Event Callbacks **********/

static void mouseCallBack(GLFWwindow* window, int button, int action, int mods)
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

static void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods)
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

void GLFWHandler::RegisterEvent()
{
  glfwSetWindowUserPointer(window, this);  
  glfwSetMouseButtonCallback(window, mouseCallBack);
  glfwSetKeyCallback(window, keyCallBack);
  glfwSetWindowSizeCallback(window, windowResizeCallback);
}

void GLFWHandler::Run()
{
  /********** MVP MATRIX **********/
  const float matrix[16] =
  {
      1.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 1.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f
  };

  int mat_loc = glGetUniformLocation(shader_program, "MVP");
  int tex_loc = glGetUniformLocation(shader_program, "tex");

  /********** LINEAR SAMPLER FOR TEXTURE RENDERING **********/
  GLuint linearSampler;
  GL_CHECK( glGenSamplers(1, &linearSampler) );
  GL_CHECK( glSamplerParameteri(linearSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR) );
  GL_CHECK( glSamplerParameteri(linearSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR) );

  /********** COMPUTE SHADER TIMINGS ***********/
  GLint64 startTime, stopTime;
  GLuint queryID[2];

  /********** SAVING IMAGES **********/
  std::vector<unsigned char*> buffers;

  /********** RENDERING & SIMULATION LOOP ***********/
  while (!glfwWindowShouldClose(window))
  {
    /********** GENERATING QUERIES FOR TIMINGS **********/
    GL_CHECK(glGenQueries(2, queryID));
    GL_CHECK(glQueryCounter(queryID[0], GL_TIMESTAMP)); 

    /********** UPDATING THE SIMULATION **********/
    simulation->Update();

    /********** COMPUTE SHADER EXECUTION TIME *********/
    GL_CHECK(glQueryCounter(queryID[1], GL_TIMESTAMP));
    GLint stopTimerAvailable = 0;
    while (!stopTimerAvailable) {
        GL_CHECK(glGetQueryObjectiv(queryID[1],
                              GL_QUERY_RESULT_AVAILABLE,
                              &stopTimerAvailable));
    }
    GL_CHECK(glGetQueryObjectui64v(queryID[0], GL_QUERY_RESULT, (GLuint64*) &startTime));
    GL_CHECK(glGetQueryObjectui64v(queryID[1], GL_QUERY_RESULT, (GLuint64*) &stopTime));
  
    printf("\r%.3f ms", (stopTime - startTime) / 1000000.0);
    fflush(stdout);

    glfwPollEvents();

    /********** RENDERING THE TEXTURE **********/
    int displayW, displayH;
    glfwGetFramebufferSize(window, &displayW, &displayH);
    GL_CHECK(glViewport(0, 0, displayW, displayH));

    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    GL_CHECK(glClearColor(0.0, 0.0, 0.0, 1.0));
    GL_CHECK(glUseProgram(shader_program));
    GL_CHECK(glActiveTexture(GL_TEXTURE0));
    GL_CHECK(glUniform1i(tex_loc, 0));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, simulation->shared_texture));
    GL_CHECK(glBindSampler(0, linearSampler));
    GL_CHECK(glUniformMatrix4fv(mat_loc, 1, GL_FALSE, matrix));
    GL_CHECK(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

    if(options->exportImages)
    {
      unsigned char *colors = new unsigned char[3 * options->simWidth * options->simHeight];
      GL_CHECK(glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, colors));
      buffers.push_back(colors);
    }

    glfwSwapBuffers(window);
  }

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
      delete [] colors;
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
