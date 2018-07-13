

// this is CUDA C++ code, so "extern "C"" is needed

extern "C" {

#include "gs_physics.h"

#include <stdio.h>
// from OpenCL
//__kernel void compute_system(__global float4 * in_p, __global float4 * g_vel, __global float4 * out_p, float4 uni_grav, int n_particles, float dt, float G_const) {

struct {

    vec4_t * GPU_in_p, *GPU_g_vel, * GPU_out_p;

} cuda_data;


#define CUDACHK(st) gpu_assert((st), __FILE__, __LINE__)

void gpu_assert(cudaError_t code, const char *file, int line) {
    if (code != cudaSuccess) {
        printf("CUDA::ERROR: (at %s:%d) (code %d): %d\n", file, line, code, cudaGetErrorString(code));
    }
}

#ifndef SOFT_FACTOR
#define SOFT_FACTOR 0.25
#endif


/* utils for vec4 code */

__device__ vec4_t cvec4_add(vec4_t a, vec4_t b) {
    return V4(a.x+b.x, a.y+b.y, a.z+b.z, a.w+b.w);
}

__device__ vec4_t cvec4_scale(vec4_t a, float b) {
    return V4(a.x*b, a.y*b, a.z*b, a.w*b);
}

__device__ float cuda_mass_to_size(float mass) {
    return cbrtf(mass);
}

__device__ vec4_t cuda_force_calc(vec4_t a, vec4_t b, float g_coef) {
    vec4_t r;
    r.x = b.x - a.x;
    r.y = b.y - a.y;
    r.z = b.z - a.z;
    float dist_sqr = r.x * r.x + r.y * r.y + r.z * r.z + SOFT_FACTOR * SOFT_FACTOR;
    float s = g_coef * a.w * b.w / dist_sqr;
    r.x *= s;
    r.y *= s;
    r.z *= s;
    r.w = 0.0;
    return r;
}


__global__ void cuda_kernel_compute_system(int n_part, vec4_t * in_p, vec4_t * g_vel, vec4_t * out_p, float G_const, vec4_t uni_grav, float dt) {
    int i = blockIdx.x*blockDim.x + threadIdx.x, j;
    if (i >= n_part) return;
    vec4_t t_force = V4(0.0, 0.0, 0.0, 0.0);
    vec4_t my_p = in_p[i], my_vel = g_vel[i];
    float my_size = cuda_mass_to_size(my_p.w);
    
    for (j = 0; j < n_part; ++j) {
      if (i != j) {
        t_force = cvec4_add(t_force, cuda_force_calc(my_p, in_p[j], G_const)); //
      }
    }
    
    t_force = cvec4_add(t_force, uni_grav);
  
    // update relevant stuff
    vec4_t new_vel = cvec4_add(my_vel, cvec4_scale(t_force, dt / my_p.w));
    vec4_t new_pos = cvec4_add(my_p, cvec4_scale(new_vel, dt));
  
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

int _plnc_hasinit = false;

void physics_loop_naive_cuda() {
    if (!_plnc_hasinit) {
        // first time initialization code
        CUDACHK(cudaMalloc((void **)cuda_data.GPU_in_p, n_particles * sizeof(vec4_t)));
        CUDACHK(cudaMalloc((void **)cuda_data.GPU_g_vel, n_particles * sizeof(vec4_t)));        
        CUDACHK(cudaMalloc((void **)cuda_data.GPU_out_p, n_particles * sizeof(vec4_t)));        

        _plnc_hasinit = true;
    }

    CUDACHK(cudaMemcpy(cuda_data.GPU_in_p, particle_data.P, n_particles * sizeof(vec4_t), cudaMemcpyHostToDevice));
    CUDACHK(cudaMemcpy(cuda_data.GPU_g_vel, particle_data.velocities, n_particles * sizeof(vec4_t), cudaMemcpyHostToDevice));

    int local_size = 256;
    int global_size = (n_particles / local_size + ((n_particles % local_size) != 0)) * local_size;

    cuda_kernel_compute_system<<<global_size, local_size>>>(n_particles, cuda_data.GPU_in_p, cuda_data.GPU_g_vel, cuda_data.GPU_out_p, gravity_coef, universal_gravity, GS_looptime);

    CUDACHK(cudaDeviceSynchronize());
    CUDACHK(cudaPeekAtLastError());

    CUDACHK(cudaMemcpy(particle_data.P, cuda_data.GPU_out_p, n_particles * sizeof(vec4_t), cudaMemcpyDeviceToHost));
    CUDACHK(cudaMemcpy(particle_data.velocities, cuda_data.GPU_g_vel, n_particles * sizeof(vec4_t), cudaMemcpyDeviceToHost));


}

}

