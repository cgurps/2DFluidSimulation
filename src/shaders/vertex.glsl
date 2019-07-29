#version 410 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 tCoords;

out vec2 texCoord;

void main()
{
  texCoord = tCoords;
  gl_Position = vec4(position, 0.0, 1.0);
}
