
/*

gravitysim - 3D N body simulation

*/


#ifndef __GRAVITYSIM_H__
#define __GRAVITYSIM_H__

#include "gravitysimconfig.h"

#include "log.h"

#include <stdbool.h>


// maximum number of particles in a simulation
#define MAX_NUM_PARTICLES 65536

typedef struct vec3_t {
    float x, y, z;
} vec3_t;

typedef struct _vec3i_t {
    int x, y, z;
} vec3i_t;

typedef struct mat4_t {
    float v[4][4];
} mat4_t;


// quick creations
#define V3(x, y, z) ((vec3_t){(x), (y), (z)})




#define M4_I M4_A( \
    1, 0, 0, 0,    \
    0, 1, 0, 0,    \
    0, 0, 1, 0,    \
    0, 0, 0, 1     \
)

// unwrap (for function calls)
#define UV3(v3) (v3).x, (v3).y, (v3).z


typedef struct total_particle_state_t {

    // mass, in a unit
    float mass;

    // coordinates
    vec3_t position, velocity;
    
} total_particle_state_t;

float gravity_coef;

int n_particles;
total_particle_state_t * particles;




#endif

