
/*

gravitysim - 3D N body simulation

*/


#ifndef __GRAVITYSIM_H__
#define __GRAVITYSIM_H__

#include "gravitysimconfig.h"

#include "log.h"

#include "ccgl.h"

#include "gs_math.h"

#include <stdbool.h>


// maximum number of particles in a simulation
#define MAX_NUM_PARTICLES 4096

typedef struct _vec3i_t {
    int x, y, z;
} vec3i_t;

float gravity_coef;

int n_particles;

#define MASS_TO_SIZE(m) (cbrtf(m))

struct {

    vec3_t * positions;

    vec3_t * velocities;

    vec3_t * forces;

    float * masses;

    bool * is_enabled;

    int _num_enabled;

} particle_data;

struct {

    vec3_t weighted_pos;

    vec3_t avg_pos, std_pos;

    int splits, joins;

} physics_data;

struct {

    bool is_paused;

} sim_data;


double GS_looptime;

#endif

