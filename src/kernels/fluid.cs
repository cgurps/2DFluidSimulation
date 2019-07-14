__constant sampler_t sampler = CLK_NORMALIZED_COORDS_TRUE | CLK_FILTER_LINEAR | CLK_ADDRESS_REPEAT;

float2 texCoords(float2 vec, int w, int h)
{
  return (float2)(vec.x / (float) w, vec.y / (float) h);
}

__kernel void advect(read_only image2d_t velocities_READ,
    read_only image2d_t field_READ,
    write_only image2d_t field_WRITE,
    __constant float* deltaTime)
{
  const size_t w = get_global_size(0);
  const size_t h = get_global_size(1);
  const size_t x = get_global_id(0);
  const size_t y = get_global_id(1);
  const float dt = *deltaTime;

  float2 tCoords = texCoords((float2)(x, y), w, h);

  float2 vel = read_imagef(velocities_READ, sampler, tCoords).xy;
  float2 pos = (float2)(x, y) - dt * vel;

  write_imagef(field_WRITE, (int2)(x, y), 
      read_imagef(field_READ, sampler, texCoords(pos, w, h)));
}

__kernel void divergence(read_only image2d_t velocities_READ,
    __global float* divergence,
    __constant float* deltaTime)
{
  const size_t w = get_global_size(0);
  const size_t h = get_global_size(1);
  const size_t x = get_global_id(0);
  const size_t y = get_global_id(1);
 
  float4 fieldL = read_imagef(velocities_READ, sampler, texCoords((float2)(x - 1, y), w, h));
  float4 fieldR = read_imagef(velocities_READ, sampler, texCoords((float2)(x + 1, y), w, h));
  float4 fieldT = read_imagef(velocities_READ, sampler, texCoords((float2)(x, y + 1), w, h));
  float4 fieldB = read_imagef(velocities_READ, sampler, texCoords((float2)(x, y - 1), w, h));

  divergence[y * w + x] = - (0.5f / (*deltaTime)) * (fieldR.x - fieldL.x + fieldT.y - fieldB.y);
}

__kernel void jacobi(__global float* divergence,
    read_only image2d_t pressure_READ,
    write_only image2d_t pressure_WRITE,
    __constant float* deltaTime
    )
{
  const size_t w = get_global_size(0);
  const size_t h = get_global_size(1);
  const size_t x = get_global_id(0);
  const size_t y = get_global_id(1);

  write_imagef(pressure_WRITE, (int2)(x, y), (float4)( 0.2f * (*deltaTime) * (divergence[y * w  + x]
      + read_imagef(pressure_READ, sampler, texCoords((float2)(x + 2, y), w, h)).x
      + read_imagef(pressure_READ, sampler, texCoords((float2)(x - 2, y), w, h)).x
      + read_imagef(pressure_READ, sampler, texCoords((float2)(x, y + 2), w, h)).x
      + read_imagef(pressure_READ, sampler, texCoords((float2)(x, y - 2), w, h)).x), 
        0.0, 0.0, 0.0));
}

__kernel void pressureProjection(read_only image2d_t pressure_READ,
    read_only image2d_t velocities_READ,
    write_only image2d_t velocities_WRITE,
    __constant float* deltaTime)
{
  const size_t w = get_global_size(0);
  const size_t h = get_global_size(1);
  const size_t x = get_global_id(0);
  const size_t y = get_global_id(1);

  float pL = read_imagef(pressure_READ, sampler, texCoords((float2)(x - 1, y), w, h)).x;
  float pR = read_imagef(pressure_READ, sampler, texCoords((float2)(x + 1, y), w, h)).x;
  float pT = read_imagef(pressure_READ, sampler, texCoords((float2)(x, y + 1), w, h)).x;
  float pB = read_imagef(pressure_READ, sampler, texCoords((float2)(x, y - 1), w, h)).x;
  float4 gradP = (float4)(pR - pL, pT - pB, 0.0f, 0.0f);

  float4 oldVel = read_imagef(velocities_READ, sampler, (int2)(x, y));

  write_imagef(velocities_WRITE, (int2)(x, y), oldVel - 0.5f * gradP);
}
