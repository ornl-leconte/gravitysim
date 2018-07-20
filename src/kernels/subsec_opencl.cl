
// epsilon
#define SOFT_FACTOR 0.25f

#define PART3D_IDX(nx, ny, nz, x, y, z) ((x) + (nx) * (y) + (nx) * (ny) * (z))

float4 get_color(int nx, int ny, int nz, int xi, int yi, int zi) {
  float r = 0.0f, g = 0.0f, b = 0.0f;
  if (xi * 2 <= nx) {
    r = 0.8f;
  } else {
    g = 0.8f;
  }
  return (float4){ r, g, b, 1.0f };
}

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
out_c : store color data
out_p : store updated particle info

//

*/

// subsec_offsets has len nX*nY*nZ
// subsec_data has len N
// subsec_est has len nX * nY * nZ

__kernel void compute_system_subsec(int N, float dt, float G, __global float4 * in_P, __global float4 * g_V, __global float4 * out_C, __global float4 * out_P, int nX, int nY, int nZ, __global int * subsec_offsets, __global int * subsec_data, __global float4 * subsec_est) {

  int _i = get_global_id(0);
  if (_i >= N) return;

  int i = subsec_data[_i];
  
  float4 F = (float4)(0.0f, 0.0f, 0.0f, 0.0f);
  float4 P = in_P[i], V = g_V[i];
  float _mass = P.w;
  float my_size = mass_to_size(_mass);

  int cur_subsec = -1;
  int l;
  for (l = 0; l < nX * nY * nZ && cur_subsec == -1; ++l) {
    if (i == subsec_data[l]) {
      cur_subsec = l;
      break;
    }
  }
  //if (cur_subsec < 0) {
  //  return;
   // printf("error\n");
  //}

//(p, x, y, z) ((x) + nX * (y) + nX * nY * (z))
  int xi, yi, zi;
  xi = l % nX;
  yi = (l / nX) % nY;
  zi = l / (nX * nY);
  out_C[i] = get_color(nX, nY, nZ, xi, yi, zi);

  int x_min = xi - 1, x_max = xi + 1;
  int y_min = yi - 1, y_max = yi + 1;
  int z_min = zi - 1, z_max = zi + 1;

  if (x_min < 0) x_min = 0; if (x_max >= nX) x_max = nX - 1;
  if (y_min < 0) y_min = 0; if (y_max >= nY) y_max = nY - 1;
  if (z_min < 0) z_min = 0; if (z_max >= nZ) z_max = nZ - 1;

  int sxi, syi, szi;
  for (sxi = 0; sxi < nX; ++sxi)
  for (syi = 0; syi < nY; ++syi)
  for (szi = 0; szi < nZ; ++szi) {
    int target_subsec = PART3D_IDX(nX, nY, nZ, sxi, syi, szi);
      
    if (sxi >= x_min && sxi <= x_max && syi >= y_min && syi <= y_max && szi >= z_min && szi <= z_max) {
      // compute manually for each point
    
      int offset = subsec_offsets[target_subsec];
      int len;
      if (target_subsec == nX * nY * nZ - 1) {
          len = nX * nY * nZ - offset;
      } else {
          len = subsec_offsets[target_subsec + 1] - offset;
      }

      int _j;
      for (_j = 0; _j < len; ++_j) {
        int j = subsec_data[offset + _j];
        F += force_calc(P, in_P[j], G);
      }
    } else {
        F += force_calc(P, subsec_est[target_subsec], G);
    }
  }

  V = V + F * (dt / P.w);
  P = P + V * (dt);
  P.w = _mass;

  out_P[i] = P;
  //out_P[i] =(float4)(1.0f, 0.0f, 0.0f, 1.0f);
  g_V[i] = V;
  //out_C[i] = (float4)(1.0f,0.5f, 0.0f, 1.0f);
}

