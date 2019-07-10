#version 410 core

in vec2 position;

uniform mat4 MVP;

out vec2 tex_Coord;

void main()
{
  tex_Coord = position * 0.5f + 0.5f;
  gl_Position = MVP * vec4(position, 0.0, 1.0);
}
