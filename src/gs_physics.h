/*

physics calculations

*/


#ifndef __GS_PHYSICS_H__
#define __GS_PHYSICS_H__

#include "gravitysim.h"


float GS_looptime;

vec4_t calculate_force(vec4_t a, vec4_t b);

vec4_t particle_get_position(vec4_t a);


// this can be turned off if the application handles it implicitly
struct {
    
    bool need_recalc_position;

} physics_exts;

void physics_init();

void physics_update_positions();
void physics_clamp_positions();


/* these are defined in phys/ folder */

void physics_loop_naive();
void physics_loop_naive_parallel();


#ifdef HAVE_OPENCL
void physics_loop_naive_opencl();
#endif

#endif


