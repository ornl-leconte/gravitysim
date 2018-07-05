
/*

math routines for gravity sim

*/

#ifndef __GS_MATH_H__
#define __GS_MATH_H__

#include <math.h>

#include "gravitysim.h"

#define SQR(x) ((x)*(x))
#define SQRT(x) (sqrt((x)))


inline float calculate_distance_squared(vec3_t a, vec3_t b);

// return distance between 'a' and 'b'
inline float calculate_distance(vec3_t a, vec3_t b);

// this uses the global variable 'gravity_coef'
inline float calculate_force(total_particle_state_t a, total_particle_state_t b);


// generates stuff

total_particle_state_t generate_state_default(float avg_mass, float mass_range, vec3_t center, vec3_t center_range, vec3_t avg_velocity, vec3_t velocity_range);

#endif


