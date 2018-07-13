#include "gs_physics.h"

#include <pthread.h>


#define NUM_THREADS 4

vec4_t * forces_updates = NULL;
pthread_t * tid = NULL;
int * worker_id = NULL;

void * _sub_sec(void * worker_ptr) {
    int worker = *(int *)worker_ptr;
    int i;
    
    for (i = worker; i < GS.N; i += NUM_THREADS) {
        GS.F[i] = V4(0.0, 0.0, 0.0, 0.0);
    }

    for (i = worker; i < GS.N; i += NUM_THREADS) {
        vec4_t i_p = GS.P[i];
        
        int j;
        for (j = 0; j < i; ++j) {
            vec4_t j_p = GS.P[j];

            // internal force calculation
            vec4_t force = calculate_force(i_p, j_p);

            // add it directionally so they are attracted to the medium point
            GS.F[i] = vec4_add(GS.F[i], force);
            forces_updates[worker * GS.N + j] = vec4_sub(forces_updates[worker * GS.N + j], force);
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


    forces_updates = (vec4_t *)realloc((void *)forces_updates, sizeof(vec4_t) * NUM_THREADS * GS.N);


    int i;
    for (i = 0; i < NUM_THREADS * GS.N; ++i) {
        forces_updates[i] = V4(0.0, 0.0, 0.0, 0.0);
    }

    for (i = 0; i < NUM_THREADS; ++i) {
        worker_id[i] = i;
        pthread_create(&tid[i], NULL, _sub_sec, (void *)&worker_id[i]);
    }

    for (i = 0; i < NUM_THREADS; ++i) {
        pthread_join(tid[i], NULL);
    }

    for (i = 0; i < NUM_THREADS * GS.N; ++i) {
        GS.F[i % GS.N] = vec4_add(GS.F[i % GS.N], forces_updates[i]);
    }
}




