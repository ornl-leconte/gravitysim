
// epsilon
#define SOFT_FACTOR 0.25f

// this is so that radius is proportionally related to volume (all balls should have constant density)
float mass_to_size(float mass) {
  return cbrt(mass);
}

// basic force calculation
float4 force_calc(float4 a, float4 b, float gravity_coef) {
    float4 r;
    r.x = b.x - a.x;
    r.y = b.y - a.y;
    r.z = b.z - a.z;
    float dist_sqr = r.x * r.x + r.y * r.y + r.z * r.z + SOFT_FACTOR * SOFT_FACTOR;

    float s = gravity_coef * a.w * b.w / dist_sqr;

    r.x *= s;
    r.y *= s;
    r.z *= s;
    r.w = 0.0;
    return r;
}


/*

in_p : readonly buffer for retrieving particle positions (USED BY ALL KERNELS)
g_vel : read/write for updating velocity
out_p : store updated particle info

*/

__kernel void compute_system(int N, float dt, float G, __global float4 * in_P, __global float4 * g_V, __global float4 * out_P) {

  int i = get_global_id(0), j;
  if (i >= N) return;
  float4 F = (float4)(0.0f, 0.0f, 0.0f, 0.0f);
  float4 P = in_P[i], V = g_V[i];
  float my_size = mass_to_size(P.w);
  
  for (j = 0; j < N; ++j) {
    if (i != j) {
      F += force_calc(P, in_P[j], G); //
    }
  }

  V = V + F * (dt / P.w);
  P = P + V * (dt);

  out_P[i] = P;
  g_V[i] = V;
}

