#include "GLUtils.h"

#include <iostream>
#include <fstream>
#include <sstream>

void APIENTRY MessageCallback(GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const void* userParam)
{
  if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return; 

  std::cout << "---------------" << std::endl;
  std::cout << "Debug message (" << id << "): " <<  message << std::endl;

  switch (source)
  {
      case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
      case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
      case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
      case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
      case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
      case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
  } std::cout << std::endl;

  switch (type)
  {
      case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
      case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
      case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break; 
      case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
      case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
      case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
      case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
      case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
      case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
  } std::cout << std::endl;
  
  switch (severity)
  {
      case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
      case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
      case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
      case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
  } std::cout << std::endl;
  std::cout << std::endl; 
}



GLuint createTexture2D(const unsigned width, const unsigned height)
{

  GLuint tex;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  std::vector<float> data(4 * width * height, 0.0f);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, data.data());
  glGenerateMipmap(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D, 0);

  return tex;
}

GLuint compileShader(const std::string& s, GLenum type)
{
  std::cout << "Compiling " << s << "...";
  std::ifstream shader_file(s);
  std::ostringstream shader_buffer;
  shader_buffer << shader_file.rdbuf();
  std::string shader_string = preprocessIncludes(shader_buffer.str(), "shaders/simulation/", 32);
  const GLchar *shader_source = shader_string.c_str();

  GLuint shader_id = glCreateShader(type);
  glShaderSource(shader_id, 1, &shader_source, NULL);
  glCompileShader(shader_id);

  GLint status;
  glGetShaderiv(shader_id, GL_COMPILE_STATUS, &status);

  if(status == GL_TRUE)
  {
    std::cout << " OK" << std::endl;
    return shader_id;
  }
  else
  {
    exit(1);
  }
}

GLuint compileAndLinkShader(const std::string& s, GLenum type)
{
  GLuint shader = compileShader(s, type);
  GLuint program = glCreateProgram();
  glAttachShader(program, shader);
  glLinkProgram(program);

  return program;
}

std::string preprocessIncludes( const std::string source, const std::string shader_path, int level /*= 0 */ )
{
  static const boost::regex re("^[ ]*#[ ]*include[ ]+[\"<](.*)[\">].*");
  std::stringstream input;
  std::stringstream output;
  input << source;

  size_t line_number = 1;
  boost::smatch matches;

  std::string line;
  while(std::getline(input,line))
  {
    if (boost::regex_search(line, matches, re))
    {
      std::string include_file = matches[1];
      std::string include_string;

      std::ifstream shader_file(shader_path + include_file);
      std::ostringstream shader_buffer;
      shader_buffer << shader_file.rdbuf();
      include_string = shader_buffer.str();

      output << preprocessIncludes(include_string, shader_path, level + 1) << std::endl;
    }
    else
    {
      output << line << std::endl;
    }
    ++line_number;
  }

  return output.str();
}
