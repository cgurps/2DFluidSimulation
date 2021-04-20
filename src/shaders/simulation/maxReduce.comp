#version 430

#include "includes.comp"
#include "layout_size.comp"

layout(rgba16f, binding = 0) uniform image2D oTex;
layout(binding = 1) uniform sampler2D iTex;

void main()
{
  ivec2 tSize = TEXTURE_SIZE(iTex);
  ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
 
  vec4 a = TEXTURE_2D(iTex, pixelToTexel(2 * pixelCoords              , tSize)); 
  vec4 b = TEXTURE_2D(iTex, pixelToTexel(2 * pixelCoords + ivec2(1, 0), tSize)); 
  vec4 c = TEXTURE_2D(iTex, pixelToTexel(2 * pixelCoords + ivec2(0, 1), tSize)); 
  vec4 d = TEXTURE_2D(iTex, pixelToTexel(2 * pixelCoords + ivec2(1, 1), tSize)); 

  imageStore(oTex, pixelCoords, max(max(max(a, b), c), d));
}

