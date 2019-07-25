#include "GLUtils.h"

#include <iostream>
#include <fstream>
#include <sstream>

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

GLuint createTexture2D(const unsigned width, const unsigned height)
{
  GLuint tex;
  GL_CHECK( glGenTextures(1, &tex) );
  GL_CHECK( glActiveTexture(GL_TEXTURE0) );
  GL_CHECK( glBindTexture(GL_TEXTURE_2D, tex) );
  GL_CHECK( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE) );
  GL_CHECK( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE) );
  GL_CHECK( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST) );
  GL_CHECK( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST) );
  GL_CHECK( glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL) );
  GL_CHECK( glBindTexture(GL_TEXTURE_2D, 0));

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
    GLint max_length = 0;
    glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &max_length);
    char buffer[max_length];
    glGetShaderInfoLog(shader_id, max_length, &max_length, buffer);
    glDeleteShader(shader_id);
    std::cout << std::endl << buffer << std::endl;
    exit(1);
  }
}

GLuint compileAndLinkShader(const std::string& s, GLenum type)
{
  GLuint shader = compileShader(s, type);
  GLuint program = glCreateProgram();
  GL_CHECK( glAttachShader(program, shader) );
  GL_CHECK( glLinkProgram(program) );

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
