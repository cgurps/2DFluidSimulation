#version 430

#include "includes.comp"
#include "layout_size.comp"

layout(rgba16f, binding = 0) uniform image2D velocities_WRITE;
layout(binding = 1) uniform sampler2D velocities_READ;
layout(binding = 2) uniform sampler2D pressure_READ;

// 01 ------ 11
// |          |
// |  (0, 0)  |
// |          |
// 00 ------ 10

void main()
{
  const ivec2 tSize = TEXTURE_SIZE(pressure_READ);
  const vec2 pixelCoords = gl_GlobalInvocationID.xy;
  const vec2 dx = vec2(1, 0); 
  const vec2 dy = vec2(0, 1);

  const vec4 pC = texelFetch(pressure_READ, ivec2(pixelCoords), 0);
  const vec4 pL = TEXTURE_2D(pressure_READ, pixelToTexel(pixelCoords - dx, tSize));
  const vec4 pR = TEXTURE_2D(pressure_READ, pixelToTexel(pixelCoords + dx, tSize));
  const vec4 pB = TEXTURE_2D(pressure_READ, pixelToTexel(pixelCoords - dy, tSize));
  const vec4 pT = TEXTURE_2D(pressure_READ, pixelToTexel(pixelCoords + dy, tSize));

  const vec2 rGrad = 0.5 * vec2(pC.y - pL.y, pC.w - pB.w);
  const vec2 gGrad = 0.5 * vec2(pR.x - pC.x, pC.z - pB.z);
  const vec2 bGrad = 0.5 * vec2(pR.w - pC.w, pT.y - pC.y);
  const vec2 aGrad = 0.5 * vec2(pC.z - pL.z, pT.x - pC.x);

  const ivec2 pCoords = 2 * ivec2(pixelCoords);
  const vec2 rVel = texelFetch(      velocities_READ, pCoords, 0             ).xy;
  const vec2 gVel = texelFetchOffset(velocities_READ, pCoords, 0, ivec2(1, 0)).xy;
  const vec2 bVel = texelFetchOffset(velocities_READ, pCoords, 0, ivec2(1, 1)).xy;
  const vec2 aVel = texelFetchOffset(velocities_READ, pCoords, 0, ivec2(0, 1)).xy;

  imageStore(velocities_WRITE, pCoords              , vec4(rVel - rGrad, 0.0, 0.0));
  imageStore(velocities_WRITE, pCoords + ivec2(1, 0), vec4(gVel - gGrad, 0.0, 0.0));
  imageStore(velocities_WRITE, pCoords + ivec2(1, 1), vec4(bVel - bGrad, 0.0, 0.0));
  imageStore(velocities_WRITE, pCoords + ivec2(0, 1), vec4(aVel - aGrad, 0.0, 0.0));
}
