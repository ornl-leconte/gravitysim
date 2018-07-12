
#include "gs_physics.h"

void physics_loop_naive() {
    int i;
    
    for (i = 0; i < n_particles; ++i) {
        particle_data.forces[i] = V4(0.0, 0.0, 0.0, 0.0);
    }

    for (i = 0; i < n_particles; ++i) {
        if (!particle_data.is_enabled[i]) continue;
        vec4_t i_p = particle_data.P[i];
        
        int j;
        for (j = 0; j < i; ++j) {
            if (!particle_data.is_enabled[j]) continue;

            vec4_t j_p = particle_data.P[j];

            // internal force calculation
            vec4_t force = calculate_force(i_p, j_p);

            // add it directionally so they are attracted to the medium point
            particle_data.forces[i] = vec4_add(particle_data.forces[i], force);
            particle_data.forces[j] = vec4_sub(particle_data.forces[j], force);
        }
    }
}



