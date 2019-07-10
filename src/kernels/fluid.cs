__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_LINEAR | CLK_ADDRESS_CLAMP_TO_EDGE;

__kernel void advect(read_only image2d_t velocities_READ,
    write_only image2d_t velocities_WRITE,
    __constant float* deltaTime)
{
  const size_t x = get_global_id(0);
  const size_t y = get_global_id(1);

  float2 vel = read_imagef(velocities_READ, (int2)(x, y)).xy;
  float2 pos = (float2)(x, y);

  pos -= (*deltaTime) * vel;

  write_imagef(velocities_WRITE, (int2)(x, y), read_imagef(velocities_READ, sampler, (int2)(pos.x, pos.y)));
}

__kernel void divergence(read_only image2d_t velocities_READ,
    __global float* divergence,
    __constant int* width,
    __constant float* deltaTime)
{
  const size_t x = get_global_id(0);
  const size_t y = get_global_id(1);
 
  float4 fieldL = read_imagef(velocities_READ, sampler, (int2)(x - 1, y));
  float4 fieldR = read_imagef(velocities_READ, sampler, (int2)(x + 1, y));
  float4 fieldT = read_imagef(velocities_READ, sampler, (int2)(x, y + 1));
  float4 fieldB = read_imagef(velocities_READ, sampler, (int2)(x, y - 1));

  divergence[y * (*width) + x] = 0.5f * (fieldR.x - fieldL.x + fieldT.y - fieldB.y);
}
