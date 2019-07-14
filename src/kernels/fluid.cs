const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_CLAMP_TO_EDGE;

__kernel void advect(read_only  image2d_t velocities_READ,
                     read_only  image2d_t field_READ,
                     write_only image2d_t field_WRITE,
                     __constant float*    deltaTime)
{
  const int2 xy = {get_global_id(0), get_global_id(1)};

  float2 vel = read_imagef(velocities_READ, xy).xy;
  float2 pos = convert_float2(xy);// - (*deltaTime) * vel;

  float4 temp1 = read_imagef(field_READ, xy);
  float4 temp2 = read_imagef(field_READ, sampler, pos);

  if(xy.x == 100 && xy.y == 101) printf("%.15f %.15f", temp1.x - temp2.x, temp1.y - temp2.y);

  write_imagef(field_WRITE, xy, read_imagef(field_READ, sampler, pos));
}

__kernel void divergence(read_only  image2d_t velocities_READ,
                         __global   float*    divergence,
                         __constant float*    deltaTime)
{
  const int2 xy = {get_global_id(0), get_global_id(1)};
  const int2 dx = {1, 0}; const int2 dy = {0, 1};
  const size_t w = get_global_size(0);

  float4 fieldR = read_imagef(velocities_READ, sampler, xy + dx);
  float4 fieldL = read_imagef(velocities_READ, sampler, xy - dx);
  float4 fieldT = read_imagef(velocities_READ, sampler, xy + dy);
  float4 fieldB = read_imagef(velocities_READ, sampler, xy - dy);

  divergence[xy.y * w + xy.x] = - (0.5f / (*deltaTime)) * (fieldR.x - fieldL.x + fieldT.y - fieldB.y);
}

__kernel void jacobi(__global   float*    divergence,
                     read_only  image2d_t pressure_READ,
                     write_only image2d_t pressure_WRITE,
                     __constant float*    deltaTime)
{
  const int2 xy = {get_global_id(0), get_global_id(1)};
  const int2 d2x = {2, 0}; const int2 d2y = {0, 2};
  const size_t w = get_global_size(0);

  write_imagef(pressure_WRITE, xy, (float4)( 0.2f * (*deltaTime) * (divergence[xy.y * w  + xy.x]
      + read_imagef(pressure_READ, sampler, xy + d2x).x
      + read_imagef(pressure_READ, sampler, xy - d2x).x
      + read_imagef(pressure_READ, sampler, xy + d2y).x
      + read_imagef(pressure_READ, sampler, xy - d2y).x), 
        0.0, 0.0, 0.0));
}

__kernel void pressureProjection(read_only  image2d_t pressure_READ,
                                 read_only  image2d_t velocities_READ,
                                 write_only image2d_t velocities_WRITE,
                                 __constant float*    deltaTime)
{
  const int2 xy = {get_global_id(0), get_global_id(1)};
  const int2 dx = {1, 0}; const int2 dy = {0, 1};
  const size_t w = get_global_size(0);

  float pR = read_imagef(pressure_READ, sampler, xy + dx).x;
  float pL = read_imagef(pressure_READ, sampler, xy - dx).x;
  float pT = read_imagef(pressure_READ, sampler, xy + dy).x;
  float pB = read_imagef(pressure_READ, sampler, xy - dy).x;
  float4 gradP = (float4)(pR - pL, pT - pB, 0.0f, 0.0f);

  float4 oldVel = read_imagef(velocities_READ, sampler, xy);

  write_imagef(velocities_WRITE, xy, oldVel - 0.5f * gradP);
}
