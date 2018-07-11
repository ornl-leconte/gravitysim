#include "gs_physics.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct _sub_sec_t {
    int worker, offset, num;
} sub_sec_t;


#define NUM_THREADS 8

sub_sec_t * assignments = NULL;
vec3_t * force_updates = NULL;
pthread_t * tid = NULL;

void * _sub_sec(void * sub_sec_v) {
    sub_sec_t ss = *(sub_sec_t *)sub_sec_v;
    int worker = ss.worker, offset = ss.offset, num = ss.num;
    int i;
    
    for (i = offset; i < offset + num; ++i) {
        particle_data.forces[i] = V3(0.0, 0.0, 0.0);
    }

    for (i = offset; i < offset + num; ++i) {
        if (!particle_data.is_enabled[i]) continue;
        particle_t i_p = particle_data.P[i];
        
        int j;
        for (j = n_particles - 1; j > i; --j) {
            if (!particle_data.is_enabled[j]) continue;

            particle_t j_p = particle_data.P[j];

            // internal force calculation
            vec3_t force = calculate_force(i_p, j_p);

            // add it directionally so they are attracted to the medium point
            particle_data.forces[i] = vec3_add(particle_data.forces[i], force);
            // can't edit this directly because we don't "own" it
            force_updates[worker * n_particles + j] = vec3_sub(force_updates[worker * n_particles + j], force);
        }
    }

    return NULL;
}


bool _plnp_hasinit = false;

void physics_loop_naive_parallel() {
    if (!_plnp_hasinit) {
        _plnp_hasinit = true;

        assignments = (sub_sec_t *)malloc(sizeof(sub_sec_t) * NUM_THREADS);
        tid = (pthread_t *)malloc(sizeof(pthread_t) * NUM_THREADS);
        force_updates = (vec3_t *)malloc(sizeof(vec3_t) * NUM_THREADS * n_particles);
    }


    int i;
    for (i = 0; i < NUM_THREADS * n_particles; ++i) {
        force_updates[i] = V3(0.0, 0.0, 0.0);
    }

    for (i = 0; i < NUM_THREADS; ++i) {
        assignments[i].worker = i;
        assignments[i].offset = i * n_particles / NUM_THREADS;
        assignments[i].num = n_particles / NUM_THREADS;
        if (assignments[i].offset + assignments[i].num > n_particles) {
            assignments[i].num = n_particles - assignments[i].offset;
        }
        pthread_create(&tid[i], NULL, _sub_sec, (void *)&assignments[i]);
    }

    for (i = 0; i < NUM_THREADS; ++i) {
        pthread_join(tid[i], NULL);
    }

    for (i = 0; i < NUM_THREADS * n_particles; ++i) {
        int c_p = i % n_particles;
        particle_data.forces[c_p] = vec3_add(particle_data.forces[c_p], force_updates[i]);
    }
}




