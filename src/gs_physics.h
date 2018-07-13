/*

physics calculations

*/


#ifndef __GS_PHYSICS_H__
#define __GS_PHYSICS_H__

#include "gravitysim.h"


vec4_t calculate_force(vec4_t a, vec4_t b);

vec4_t particle_get_position(vec4_t a);


// this can be turned off if the application handles it implicitly
struct {
    
    bool need_recalc_position;

    bool need_add_gravity;

    bool need_collision_handle;

    bool need_clamp;

} physics_exts;

void physics_init();


void physics_add_gravity();
void physics_update_positions();
void physics_collision_handle();
void physics_clamp_positions();

/* these are defined in phys/ folder */

void physics_loop_naive();
void physics_loop_naive_parallel();


#ifdef HAVE_OPENCL
void physics_loop_naive_opencl();
#endif


#ifdef HAVE_CUDA
void physics_loop_naive_cuda();
#endif


#endif


