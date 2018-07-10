
/*

gravitysim - 3D N body simulation

*/


#ifndef __GRAVITYSIM_H__
#define __GRAVITYSIM_H__

#include "gravitysimconfig.h"

#include "log.h"

#include "ccgl.h"

#include <stdbool.h>


// maximum number of particles in a simulation
#define MAX_NUM_PARTICLES 4096



typedef struct _vec3i_t {
    int x, y, z;
} vec3i_t;


float gravity_coef;

int n_particles;

struct {

    vec3_t * positions;

    vec3_t * velocities;

    float * masses;

} particle_data;



#endif

