
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
extern char * shared_data_dir;

#define MASS_TO_SIZE(m) (cbrtf(m))

// global struct with all the major components
struct {

    // how many particles currently there?
    int N;

    // particle data, x, y, z, and mass
    // x, y, z (and indeed all gravitysim's position data) is taken to be in meters
    // mass is in kilograms
    vec4_t * P;

    // the .w are nothing on these two
    // velocities
    // x, y, and z are in mass/seconds
    vec4_t * V;

    // current forces. This array should be zero'd each time
    // forces in (kg * meters) / (seconds^2), i.e. in newtons
    vec4_t * F;


    // gravitational constant, in units of (meters^3)/(kg*(seconds^2))
    // and for real life, G = 6.67408e-11
    float G;

    // collision beta coef.
    // See gs_physics.c where this is used for a description of how it works.
    // good value is 12.0f
    float coll_B;

    // time step, or delta time this current loop
    float dt;

    // time since starting
    float tt;

    // how many frames have been computed?
    int n_frames;

    bool is_paused;

} GS;


// updates the number of particles
void update_N(int N);


#endif

