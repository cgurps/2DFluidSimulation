#version 430

#include "includes.comp"
#include "layout_size.comp"

layout(rgba16f, binding = 0) uniform image2D pressure_WRITE;
layout(binding = 1) uniform sampler2D pressure_READ;
layout(binding = 2) uniform sampler2D divergence;

// The divergence and pressure is packed in an half-size texture.
// This black pass updates the bottom left and top right points (black points)
// using neighbooring red points.
// For example result(22) = p(12) + p(32) + p(23) + (21) - div(22) 
//                        = p(0, 1).g + p(1, 1).a + p(1, 1).g + p(1, 0).a - div(1, 1).g;

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

  const float r = 0.25 * (pL.y + pC.y + pB.w + pC.w - dC.x);
  const float b = 0.25 * (pC.w + pR.w + pC.y + pT.y - dC.z);

  imageStore(pressure_WRITE, ivec2(pixelCoords), vec4(r, pC.y, b, pC.w));
}
