
#include "gs_physics.h"

void physics_loop_naive() {
    int i;
    
    for (i = 0; i < n_particles; ++i) {
        particle_data.forces[i] = V3(0.0, 0.0, 0.0);
    }

    for (i = 0; i < n_particles; ++i) {
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
            particle_data.forces[j] = vec3_sub(particle_data.forces[j], force);
        }
    }
}



