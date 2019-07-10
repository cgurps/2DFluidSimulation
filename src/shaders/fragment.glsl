#version 410 core

in vec2 tex_Coord;

out vec4 out_color;

uniform sampler2D tex;

void main()
{
  out_color = texture(tex, tex_Coord) * vec4(1.0, 1.0, 1.0, 1.0);
}
