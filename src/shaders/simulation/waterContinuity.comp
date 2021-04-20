#version 450

#include "includes.comp"
#include "layout_size.comp"

layout(rgba16f, binding = 0) uniform image2D qTex;
layout(rgba16f, binding = 1) uniform image2D pTemp;
layout(binding = 2) uniform sampler2D pAdvectedTemp;

#define G 9.80665
#define P0 101325.0
#define T0 290.0
#define LAPSE_RATE 10.0
#define RD 287.0
#define kappa 0.286
#define L 2.501

void main()
{
  vec2 tSize = TEXTURE_SIZE(pAdvectedTemp);
  ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);

  // Water Continuity
  vec4 theta = imageLoad(pTemp, pixelCoords);
  vec4 q = imageLoad(qTex, pixelCoords);

  float z = float(pixelCoords.y) / tSize.y;
  float p = P0 * pow(1.0 - z *  LAPSE_RATE / T0, G / (LAPSE_RATE / RD));
  float t = pow(P0 / p, kappa) / theta.x;
  float qvs = (380.16 / p) * exp(17.67 * t / (t + 243.5));
  float deltaQ = min(qvs - q.x, q.y);
  
  q.x += deltaQ;
  q.y -= deltaQ;

  // Thermodynamics
  vec4 thetaAdv = texelFetch(pAdvectedTemp, pixelCoords, 0);
  t = pow(P0 / p, kappa) / thetaAdv.x;
  qvs = (380.16 / p) * exp(17.67 * t / (t + 243.5));
  deltaQ = min(qvs - q.x, q.y);
  thetaAdv.x += (RD * L / kappa) * pow(P0 / p, kappa) * deltaQ;

  imageStore(qTex, pixelCoords, q);
  imageStore(pTemp, pixelCoords, thetaAdv);
}
