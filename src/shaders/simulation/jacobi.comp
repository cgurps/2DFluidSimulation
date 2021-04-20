#version 430

#include "includes.comp"
#include "layout_size.comp"

layout(rgba16f, binding = 0) uniform image2D pressure_WRITE;
layout(binding = 1) uniform sampler2D pressure_READ;
layout(binding = 2) uniform sampler2D divergence;

void main()
{
  ivec2 tSize = TEXTURE_SIZE(pressure_READ);
  ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
  const ivec2 dx = ivec2(1, 0); 
  const ivec2 dy = ivec2(0, 1);

  float dC = texelFetch(divergence, pixelCoords, 0).x;

  float pL = texelFetchOffset(pressure_READ, pixelCoords, 0, - dx).x;
  float pR = texelFetchOffset(pressure_READ, pixelCoords, 0,   dx).x;
  float pB = texelFetchOffset(pressure_READ, pixelCoords, 0, - dy).x;
  float pT = texelFetchOffset(pressure_READ, pixelCoords, 0,   dy).x;

  float pC = texelFetch(pressure_READ, pixelCoords, 0).x;
  if(pixelCoords.x == 0) pL = pC;
  if(pixelCoords.y == 0) pB = pC;
  if(pixelCoords.x == tSize.x - 1) pR = pC;
  if(pixelCoords.y == tSize.y - 1) pT = pC;

  imageStore(pressure_WRITE, pixelCoords, vec4(0.25f * (pL + pR + pB + pT - dC), 0.0f, 0.0f, 1.0f));
}
