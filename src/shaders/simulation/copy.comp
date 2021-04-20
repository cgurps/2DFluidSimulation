#version 430

#include "includes.comp"
#include "layout_size.comp"

layout(rgba16f, binding = 0) uniform image2D tex_WRITE;
layout(rgba16f, binding = 1) uniform image2D tex_READ;

void main()
{
  ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
  vec4 pixel = imageLoad(tex_READ, pixelCoords);

  imageStore(tex_WRITE, pixelCoords, pixel);
}
