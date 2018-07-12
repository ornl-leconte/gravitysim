
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

__kernel void compute_system(__global float4 * in_p, __global float4 * g_vel, __global float4 * out_p, float4 uni_grav, int n_particles, float dt, float G_const) {

  int i = get_global_id(0), j;
  if (i >= n_particles) return;
  float4 t_force = (float4)(0.0f, 0.0f, 0.0f, 0.0f);
  float4 my_p = in_p[i], my_vel = g_vel[i];
  float my_size = mass_to_size(my_p.w);
  
  for (j = 0; j < n_particles; ++j) {
    if (i != j) {
      t_force += force_calc(my_p, in_p[j], G_const); //
    }
  }
  
  t_force += uni_grav;

  // update relevant stuff
  float4 new_vel = my_vel + t_force * (dt / my_p.w);
  float4 new_pos = my_p + (dt) * new_vel;
  

  // clamping code
  if (new_pos.x + my_size > 100.0f) new_pos.x = 100.0f - my_size;
  if (new_pos.y + my_size > 100.0f) new_pos.y = 100.0f - my_size;
  if (new_pos.z + my_size > 100.0f) new_pos.z = 100.0f - my_size;

  if (new_pos.x - my_size < -100.0f) new_pos.x = my_size - 100.0f;
  if (new_pos.y - my_size < -100.0f) new_pos.y = my_size - 100.0f;
  if (new_pos.z - my_size < -100.0f) new_pos.z = my_size - 100.0f;

  out_p[i] = new_pos;
  g_vel[i] = new_vel;
}

