
#include "gs_physics.h"

#include "gs_math.h"

#define SOFT_FACTOR 0.25


vec4_t calculate_force(vec4_t a, vec4_t b) {
    vec4_t r;
    r.x = b.x - a.x;
    r.y = b.y - a.y;
    r.z = b.z - a.z;
    float dist_sqr = r.x * r.x + r.y * r.y + r.z * r.z + SOFT_FACTOR * SOFT_FACTOR;

    float s = gravity_coef * a.w * b.w / dist_sqr;

    r.x *= s;
    r.y *= s;
    r.z *= s;
    r.w = 0.0;
    return r;
}

vec4_t particle_get_position(vec4_t a) {
    return V4(a.x, a.y, a.z, 0.0);
}

/* control loop */

void physics_init() {
}

void physics_update_positions() {
    float dt = GS_looptime;
    
    int i;

    for (i = 0; i < n_particles; ++i) {
        if (particle_data.is_enabled[i]) {
            // acceleration = force / mass (with timestep for discrete integration), because F = ma
            vec4_t acc = vec4_scale(particle_data.forces[i], 1.0 / particle_data.P[i].w);

            // V(t) = integral{A(t) * dt}
            particle_data.velocities[i] = vec4_add(particle_data.velocities[i], vec4_scale(acc, dt));

            // P(t) = integral{V(t) * dt}
            vec4_t pos_update = vec4_add(particle_get_position(particle_data.P[i]), vec4_scale(particle_data.velocities[i], dt));
            particle_data.P[i].x = pos_update.x;
            particle_data.P[i].y = pos_update.y;
            particle_data.P[i].z = pos_update.z;
        }
    }
}

void physics_clamp_positions() {
    int i;
    for (i = 0; i < n_particles; ++i) {
        if (particle_data.is_enabled[i]) {
            vec4_t cp = particle_data.P[i];
            float sz = MASS_TO_SIZE(cp.w);
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



