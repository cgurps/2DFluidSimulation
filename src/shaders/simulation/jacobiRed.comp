#version 430

#include "includes.comp"
#include "layout_size.comp"

layout(rgba16f, binding = 0) uniform image2D pressure_WRITE;
layout(binding = 1) uniform sampler2D pressure_READ;
layout(binding = 2) uniform sampler2D divergence;

// The divergence and pressure is packed in an half-size texture.
// This red pass updates the top left and bottom right points (red points)
// using neighbooring black points.

// 03 ------ 13        23 ------ 33
// |          |        |          |
// |  (0, 1)  |        |  (1, 1)  |
// |          |        |          |
// 02 ------ 12        22 ------ 32
//                            
//                            
// 01 ------ 11        21 ------ 31
// |          |        |          |
// |  (0, 0)  |        |  (1, 0)  |
// |          |        |          |
// 00 ------ 10        20 ------ 30

void main()
{
  const ivec2 tSize = TEXTURE_SIZE(pressure_READ);
  const vec2 pixelCoords = gl_GlobalInvocationID.xy;
  const vec2 dx = vec2(1, 0); 
  const vec2 dy = vec2(0, 1);

  const vec4 dC = texelFetch(divergence, ivec2(pixelCoords), 0);

  const vec4 pC = texelFetch(pressure_READ, ivec2(pixelCoords), 0);
  const vec4 pL = TEXTURE_2D(pressure_READ, pixelToTexel(pixelCoords - dx, tSize));
  const vec4 pR = TEXTURE_2D(pressure_READ, pixelToTexel(pixelCoords + dx, tSize));
  const vec4 pB = TEXTURE_2D(pressure_READ, pixelToTexel(pixelCoords - dy, tSize));
  const vec4 pT = TEXTURE_2D(pressure_READ, pixelToTexel(pixelCoords + dy, tSize));

  const float g = 0.25 * (pC.x + pR.x + pB.z + pC.z - dC.y);
  const float a = 0.25 * (pL.z + pC.z + pT.x + pC.x - dC.w);

  imageStore(pressure_WRITE, ivec2(pixelCoords), vec4(pC.x, g, pC.z, a));
}
