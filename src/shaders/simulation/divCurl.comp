#version 430

#include "includes.comp"
#include "layout_size.comp"

layout(rgba16f, binding = 0) uniform image2D divergence;
layout(binding = 1) uniform sampler2D velocities_READ;

void main()
{
  const ivec2 tSize = TEXTURE_SIZE(velocities_READ);
  const ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);

  const ivec2 dx = ivec2(1, 0); 
  const ivec2 dy = ivec2(0, 1);

  vec2 fieldL = texelFetchOffset(velocities_READ, pixelCoords, 0, - dx).xy;
  vec2 fieldR = texelFetchOffset(velocities_READ, pixelCoords, 0,   dx).xy;
  vec2 fieldB = texelFetchOffset(velocities_READ, pixelCoords, 0, - dy).xy;
  vec2 fieldT = texelFetchOffset(velocities_READ, pixelCoords, 0,   dy).xy;

  vec2 fieldC = texelFetch(velocities_READ, pixelCoords, 0).xy;
  if(pixelCoords.x == 0) fieldL.x = - fieldC.x;
  if(pixelCoords.y == 0) fieldB.y = - fieldC.y;
  if(pixelCoords.x >= tSize.x - 1) fieldR.x = - fieldC.x;
  if(pixelCoords.y >= tSize.y - 1) fieldT.y = - fieldC.y;

  float div = 0.5f * (fieldR.x - fieldL.x + fieldT.y - fieldB.y);
  float curl = 0.5f * (fieldR.y - fieldL.y - fieldT.x + fieldB.x);

  imageStore(divergence, pixelCoords, vec4(div, curl, 0.0f, 0.0f));
}
