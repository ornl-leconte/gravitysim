/*

physics calculations

*/


#ifndef __GS_PHYSICS_H__
#define __GS_PHYSICS_H__

#include "gravitysim.h"

vec3_t calculate_force(particle_t a, particle_t b);

vec3_t particle_get_position(particle_t a);


void physics_init();

void physics_update_positions();
void physics_update_statistics();
void physics_clamp_positions();


/* these are defined in phys/ folder */

void physics_loop_naive();
void physics_loop_naive_parallel();

#endif


