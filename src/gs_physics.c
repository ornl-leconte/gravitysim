
#include "gs_physics.h"

#include "gs_math.h"

#define SOFT_FACTOR 0.25


vec3_t calculate_force(particle_t a, particle_t b) {
    vec3_t r;
    r.x = b.x - a.x;
    r.y = b.y - a.y;
    r.z = b.z - a.z;
    float dist_sqr = r.x * r.x + r.y * r.y + r.z * r.z + SOFT_FACTOR * SOFT_FACTOR;
    float inv_dist_cube = 1.0 / sqrtf(dist_sqr * dist_sqr * dist_sqr);

    float s = gravity_coef * b.mass * inv_dist_cube;

    r.x *= s;
    r.y *= s;
    r.z *= s;
    return r;
}

vec3_t particle_get_position(particle_t a) {
    return V3(a.x, a.y, a.z);
}

/* control loop */

void physics_init() {
}

void physics_update_positions() {
    float dt = GS_looptime;
    
    int i;

    for (i = 0; i < n_particles; ++i) {
        if (particle_data.is_enabled[i]) {
            // acceleration = foce / mass (with timestep for discrete integration), because F = ma
            vec3_t acc = vec3_scale(particle_data.forces[i], 1.0 / particle_data.P[i].mass);

            // V(t) = integral{A(t) * dt}
            particle_data.velocities[i] = vec3_add(particle_data.velocities[i], vec3_scale(acc, dt));

            // P(t) = integral{V(t) * dt}
            vec3_t pos_update = vec3_add(particle_get_position(particle_data.P[i]), vec3_scale(particle_data.velocities[i], dt));
            particle_data.P[i].x = pos_update.x;
            particle_data.P[i].y = pos_update.y;
            particle_data.P[i].z = pos_update.z;
        }
    }
}

void physics_update_statistics() {
    physics_data.avg_pos = V3(0.0, 0.0, 0.0);
    physics_data.weighted_pos = V3(0.0, 0.0, 0.0);
    physics_data.std_pos = V3(0.0, 0.0, 0.0);

    float sum_mass = 0.0;
    int enabled = 0;
    int i;

    for (i = 0; i < n_particles; ++i) {
        if (particle_data.is_enabled[i]) {
            // keep count of how many are enabled
            enabled++;

            // update statistics
            physics_data.avg_pos = vec3_add(physics_data.avg_pos, particle_get_position(particle_data.P[i]));

            physics_data.weighted_pos = vec3_add(physics_data.weighted_pos, vec3_scale(particle_get_position(particle_data.P[i]), particle_data.P[i].mass));
            sum_mass += particle_data.P[i].mass;
        }
    }

    physics_data.avg_pos = vec3_scale(physics_data.avg_pos, 1.0 / enabled);
    physics_data.weighted_pos = vec3_scale(physics_data.weighted_pos, sum_mass);

    // calculate std
    for (i = 0; i < n_particles; ++i) {
        if (particle_data.is_enabled[i]) {
            vec3_t cdiff = vec3_sub(particle_get_position(particle_data.P[i]), physics_data.avg_pos);
            cdiff.x = cdiff.x * cdiff.x;
            cdiff.y = cdiff.y * cdiff.y;
            cdiff.z = cdiff.z * cdiff.z;
            physics_data.std_pos = vec3_add(physics_data.std_pos, cdiff);
        }
    }
    physics_data.std_pos.x = sqrtf(physics_data.std_pos.x);
    physics_data.std_pos.y = sqrtf(physics_data.std_pos.y);
    physics_data.std_pos.z = sqrtf(physics_data.std_pos.z);
    physics_data.std_pos = vec3_scale(physics_data.std_pos, 1.0 / (enabled - 1));


}



void physics_clamp_positions() {
    int i;
    for (i = 0; i < n_particles; ++i) {
        if (particle_data.is_enabled[i]) {
            particle_t cp = particle_data.P[i];
            float sz = MASS_TO_SIZE(cp.mass);
            if (cp.x + sz > 100.0) cp.x = 100.0 - sz;
            if (cp.y + sz > 100.0) cp.y = 100.0 - sz;
            if (cp.z + sz > 100.0) cp.z = 100.0 - sz;
            if (cp.x - sz < -100.0) cp.x = sz - 100.0;
            if (cp.y - sz < -100.0) cp.y = sz - 100.0;
            if (cp.z - sz < -100.0) cp.z = sz - 100.0;
            particle_data.P[i] = cp;
        }
    }
}



