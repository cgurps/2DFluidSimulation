#version 410 core

in vec2 texCoord;

out vec4 outColor;

uniform sampler2D tex;

void main()
{
  outColor = texture(tex, texCoord);
  //outColor = vec4(texCoord, texCoord);
}
