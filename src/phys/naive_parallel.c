#include "gs_physics.h"

#include <pthread.h>


#define NUM_THREADS 8

pthread_t * tid = NULL;
int * worker_id = NULL;

void * _sub_sec(void * worker_ptr) {
    int worker = *(int *)worker_ptr;
    int i;
    
    for (i = worker; i < n_particles; i += NUM_THREADS) {
        particle_data.forces[i] = V4(0.0, 0.0, 0.0, 0.0);
    }

    for (i = worker; i < n_particles; i += NUM_THREADS) {
        if (!particle_data.is_enabled[i]) continue;
        vec4_t i_p = particle_data.P[i];
        
        int j;
        for (j = 0; j < n_particles; ++j) {
            if (j == i || !particle_data.is_enabled[j]) continue;
            vec4_t j_p = particle_data.P[j];

            // internal force calculation
            vec4_t force = calculate_force(i_p, j_p);

            // add it directionally so they are attracted to the medium point
            particle_data.forces[i] = vec4_add(particle_data.forces[i], force);
        }
    }

    return NULL;
}


bool _plnp_hasinit = false;

void physics_loop_naive_parallel() {
    if (!_plnp_hasinit) {
        _plnp_hasinit = true;

        worker_id = (int *)malloc(sizeof(int) * NUM_THREADS);
        tid = (pthread_t *)malloc(sizeof(pthread_t) * NUM_THREADS);
    }


    int i;
    for (i = 0; i < NUM_THREADS; ++i) {
        worker_id[i] = i;
        pthread_create(&tid[i], NULL, _sub_sec, (void *)&worker_id[i]);
    }

    for (i = 0; i < NUM_THREADS; ++i) {
        pthread_join(tid[i], NULL);
    }
}




