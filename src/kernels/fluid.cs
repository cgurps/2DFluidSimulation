__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_LINEAR | CLK_ADDRESS_CLAMP_TO_EDGE;

__kernel void advect(read_only image2d_t velocities_READ,
    read_only image2d_t field_READ,
    write_only image2d_t field_WRITE,
    __constant float* deltaTime)
{
  const size_t x = get_global_id(0);
  const size_t y = get_global_id(1);

  float2 vel = read_imagef(velocities_READ, (int2)(x, y)).xy;
  float2 pos = (float2)(x, y);

  pos -= (*deltaTime) * vel;

  write_imagef(field_WRITE, (int2)(x, y), read_imagef(field_READ, sampler, (int2)(pos.x, pos.y)));
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

  divergence[y * (*width) + x] = - (2.0 / (*deltaTime)) * (fieldR.x - fieldL.x + fieldT.y - fieldB.y);
}

__kernel void jacobi(__global float* divergence,
    read_only image2d_t pressure_READ,
    write_only image2d_t pressure_WRITE,
    __constant int* width,
    __constant int* k)
{
  const size_t x = get_global_id(0);
  const size_t y = get_global_id(1);

  const size_t pos = y * (*width) + x;

  if(k == 0)
  {
    write_imagef(pressure_WRITE, (int2)(x, y), (float4)(0.25f * divergence[pos], 0.0, 0.0, 0.0));
  }
  else
  {
    write_imagef(pressure_WRITE, (int2)(x, y), (float4)(0.25f * (divergence[pos]
        + read_imagef(pressure_READ, sampler, (int2)(x + 2, y)).x
        + read_imagef(pressure_READ, sampler, (int2)(x - 2, y)).x
        + read_imagef(pressure_READ, sampler, (int2)(x, y + 2)).x
        + read_imagef(pressure_READ, sampler, (int2)(x, y - 2)).x), 0.0, 0.0, 0.0));
  }
}

__kernel void pressureProjection(read_only image2d_t pressure_READ,
    read_only image2d_t velocities_READ,
    write_only image2d_t velocities_WRITE,
    __constant float* deltaTime)
{
  const size_t x = get_global_id(0);
  const size_t y = get_global_id(1);

  float4 oldVel = read_imagef(velocities_READ, sampler, (int2)(x, y));

  oldVel.x = oldVel.x - 0.5f * (*deltaTime) * (read_imagef(pressure_READ, sampler, (int2)(x + 1, y)).x 
        - read_imagef(pressure_READ, sampler, (int2)(x - 1, y)).x);
  oldVel.y = oldVel.y - 0.5f * (*deltaTime) * (read_imagef(pressure_READ, sampler, (int2)(x, y + 1)).x 
        - read_imagef(pressure_READ, sampler, (int2)(x, y - 1)).x);

  write_imagef(velocities_WRITE, (int2)(x, y), oldVel);
}

__kernel void writeToTexture(write_only image2d_t texture)
{
  const size_t x = get_global_id(0);
  const size_t y = get_global_id(1);

  write_imagef(texture, (int2)(x, y), (float4)(1.0, 1.0, 0.0, 1.0));
}
