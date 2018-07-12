


float4 force_calc(float4 a, float4 b, float gravity_coef) {
    float4 r;
    r.x = b.x - a.x;
    r.y = b.y - a.y;
    r.z = b.z - a.z;
    float dist_sqr = r.x * r.x + r.y * r.y + r.z * r.z + 0.25f * 0.25f;

    float s = gravity_coef * a.w * b.w / dist_sqr;

    r.x *= s;
    r.y *= s;
    r.z *= s;
    r.w = 0.0;
    return r;
}


__kernel void compute_system(__global float4 * in_p, __global float4 * out_force, int n_particles, float G_const) {
  int i = get_global_id(0), j;
  if (i >= n_particles) return;
  float4 t_force = (float4)(0.0f, 0.0f, 0.0f, 0.0f), c_force;
  float4 my_p = in_p[i];
  for (j = 0; j < n_particles; ++j) {
    if (i != j) {
      c_force = force_calc(my_p, in_p[j], G_const); //
      t_force += c_force;
    }
  }
  out_force[i] = t_force;
}
