
/*

gravitysim - 3D N body simulation

*/


#ifndef __GRAVITYSIM_H__
#define __GRAVITYSIM_H__

#include "gravitysimconfig.h"
#include "log.h"

#include "ccgl.h"
#include "controls.h"
#include "gs_math.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>


// maximum number of particles in a simulation
#define MAX_NUM_PARTICLES 4096

typedef struct _vec3i_t {
    int x, y, z;
} vec3i_t;


// this is used for all sorts of things. For example 'shared_data_dir'/src/phys can get you kernels, etc
char * shared_data_dir;

// always added
vec4_t universal_gravity;

float gravity_coef;

int n_particles;

#define MASS_TO_SIZE(m) (cbrtf(m))

struct {

    vec4_t * P;

    // use packed stuff
    vec4_t * velocities;

    vec4_t * forces;

    bool * is_enabled;

    int _num_enabled;

} particle_data;


struct {

    bool is_paused;

} sim_data;


float GS_looptime;

#endif

