#version 430

#include "includes.comp"
#include "layout_size.comp"

layout(rgba16f, binding = 0) uniform image2D divergence;
layout(binding = 1) uniform sampler2D velocities_READ;

// The grid point 11 corresponds to the pixel coords 2 * pixelCoords
// We then fetch various neighboors to compute the divergence of points 11, 21, 12, 22
// and store them in a single point on the divergence texture with r -> 11, g -> 21, b -> 22, a -> 12

// 03 ------ 13 ------ 23 ------ 33
// |         |         |         |
// |         |         |         |
// 02 ------ 12 ------ 22 ------ 32
// |         |         |         |
// |         |         |         |
// 01 ------ 11 ------ 21 ------ 31
// |         |         |         |
// |         |         |         |
// 00 ------ 10 ------ 20 ------ 30


void main()
{
  const ivec2 tSize = TEXTURE_SIZE(velocities_READ);
  const ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);

  vec2 field11 = texelFetch(velocities_READ      , 2 * pixelCoords, 0               ).xy;
  vec2 field01 = texelFetchOffset(velocities_READ, 2 * pixelCoords, 0, ivec2(-1,  0)).xy;
  vec2 field21 = texelFetchOffset(velocities_READ, 2 * pixelCoords, 0, ivec2( 1,  0)).xy;
  vec2 field10 = texelFetchOffset(velocities_READ, 2 * pixelCoords, 0, ivec2( 0, -1)).xy;
  vec2 field12 = texelFetchOffset(velocities_READ, 2 * pixelCoords, 0, ivec2( 0,  1)).xy;
  vec2 field20 = texelFetchOffset(velocities_READ, 2 * pixelCoords, 0, ivec2( 1, -1)).xy;
  vec2 field02 = texelFetchOffset(velocities_READ, 2 * pixelCoords, 0, ivec2(-1,  1)).xy;
  vec2 field13 = texelFetchOffset(velocities_READ, 2 * pixelCoords, 0, ivec2( 0,  2)).xy;
  vec2 field23 = texelFetchOffset(velocities_READ, 2 * pixelCoords, 0, ivec2( 1,  2)).xy;
  vec2 field22 = texelFetchOffset(velocities_READ, 2 * pixelCoords, 0, ivec2( 1,  1)).xy;
  vec2 field31 = texelFetchOffset(velocities_READ, 2 * pixelCoords, 0, ivec2( 2,  0)).xy;
  vec2 field32 = texelFetchOffset(velocities_READ, 2 * pixelCoords, 0, ivec2( 2,  1)).xy;

  if(pixelCoords.x == 0) 
  {
    field01.x = - field11.x;
    field02.x = - field12.x;
  }
  if(pixelCoords.y == 0)
  {
    field10.y = - field11.y;
    field20.y = - field21.y;
  }
  if(2 * pixelCoords.x + 1 >= tSize.x - 1)
  {
    field31.x = - field21.x;
    field32.x = - field22.x;
  }
  if(2 * pixelCoords.y + 1 >= tSize.y - 1)
  {
    field13.y = - field12.y;
    field23.y = - field22.y;
  }

  const float r = 0.5 * (field21.x - field01.x + field12.y - field10.y);
  const float g = 0.5 * (field31.x - field11.x + field22.y - field20.y);
  const float b = 0.5 * (field32.x - field12.x + field23.y - field21.y);
  const float a = 0.5 * (field22.x - field02.x + field13.y - field11.y);

  imageStore(divergence, pixelCoords, vec4(r, g, b, a));
}
