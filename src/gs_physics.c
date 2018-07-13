
#include "gs_physics.h"

#include "gs_math.h"

#include <math.h>

#define SOFT_FACTOR 0.25f


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

void physics_add_gravity() {
    int i;
    for (i = 0; i < n_particles; ++i) {
        if (particle_data.is_enabled[i]) {
            particle_data.forces[i] = vec4_add(particle_data.forces[i], universal_gravity);
        }
    }
}
void physics_update_positions() {
    float dt = GS_looptime;
    
    int i;

    for (i = 0; i < n_particles; ++i) {
        if (particle_data.is_enabled[i]) {
            // acceleration = force / mass (with timestep for discrete integration), because F = ma
            // V(t) = integral{A(t) * dt}
            // P(t) = integral{V(t) * dt}
            vec4_t new_vel = vec4_add(particle_data.velocities[i], vec4_scale(particle_data.forces[i], dt / particle_data.P[i].w));;
            particle_data.P[i] = vec4_add(particle_data.P[i], new_vel);
            particle_data.velocities[i] = new_vel;
        }
    }
}

void physics_collision_handle() {
    int i;
    int cols = 0;
    float _max = 0.0;
    for (i = 0; i < n_particles; ++i) {
        if (particle_data.is_enabled[i]) {
            int j;
            for (j = 0; j < i; ++j) {
                if (particle_data.is_enabled[j] && i != j) {
                    vec4_t i_p = particle_data.P[i];
                    float i_s = MASS_TO_SIZE(i_p.w);

                    vec4_t j_p = particle_data.P[j];
                    float j_s = MASS_TO_SIZE(j_p.w);

                    float dist = calculate_distance(V4(i_p.x, i_p.y, i_p.z, 0.0), V4(j_p.x, j_p.y, j_p.z, 0.0));
                    if (dist > 0.0 && dist <= i_s + j_s) {

                        vec4_t diff_p = vec4_sub(j_p, i_p);
                        diff_p.w = 0.0;
                        vec4_t diff_v = vec4_sub(particle_data.velocities[j], particle_data.velocities[i]);
                        diff_v.w = 0.0;
                        float force_res;
                        //if true, i is moving towards j, so we are colliding and want to deflect
                        if ((force_res = vec4_dot(diff_p, diff_v)) < 0.0) {
                            float overlap = i_s + j_s - dist / 2.0;

                            // second number is beta coefficient, 12.0 is pretty good
                            // basically:
                            // close to zero, but positive means weak pushback on collisions. They can go through each other
                            // med positive values (like 10.0 - 50.0) make them repel, and can form small clusters
                            // large values >>50.0 mean the particles start flying off of each other

                            // at beta = 0.0, no collision detection is really done; objects fly right beside each other or in through each other the same way.

                            // negative values of beta means the particle will get sucked through the one it is colliding with. Sometimes this means they can all get trapped together, but at large values this makes it very chaotic

                            float beta = 12.0;

                            vec4_t penalty = vec4_scale(vec4_normalized(diff_p), beta);

                            // these are coefficients based on how close each particle is
                            float i_prop = overlap / (i_s + 0.25);
                            //if (i_prop > 1.0f) i_prop = 1.0f;

                            float j_prop = overlap / (j_s + 0.25);
                            //if (j_prop > 1.0f) j_prop = 1.0f;

                            particle_data.forces[i] = vec4_sub(particle_data.forces[i], vec4_scale(penalty, i_prop));
                            particle_data.forces[j] = vec4_add(particle_data.forces[j], vec4_scale(penalty, j_prop));
                            /*
                            particle_data.forces[i] = vec4_add(particle_data.forces[i], vec4_scale(penalty, 1.0 / i_p.w));
                            particle_data.forces[j] = vec4_sub(particle_data.forces[j], vec4_scale(penalty, 1.0 / j_p.w));
*/
                            if (force_res > _max) _max = force_res;
                            cols++;
                        }

                        /* bouncy method
                        float prop = 1.0 - 0.1 * dist / (i_s + j_s);
                        vec4_t center = vec4_scale(vec4_add(i_p, j_p), 0.5);
                        vec4_t i_dir = vec4_sub(i_p, center);
                        center.w = 0.0;
                        i_dir.w = 0.0;
                        i_dir = vec4_normalized(i_dir);

                        vec4_t penalty = vec4_scale(i_dir, 1.0);

                        particle_data.forces[i] = vec4_add(particle_data.forces[i], vec4_scale(penalty, 1.0));
                        particle_data.forces[j] = vec4_sub(particle_data.forces[j], vec4_scale(penalty, 1.0));
                        */
                    }
                }
            }
        }
    }
    printf("collisions: %d, max: %f\n", cols, _max);
}

float clamp_val(float x, float mn, float mx) {
    if (x < mn) return mn;
    if (x > mx) return mx;
    return x;
}

void physics_clamp_positions() {
    int i;
    for (i = 0; i < n_particles; ++i) {
        if (particle_data.is_enabled[i]) {
            vec4_t cp = particle_data.P[i];
            float sz = MASS_TO_SIZE(cp.w);
            if (cp.x - sz < -100.0f) {
                cp.x = sz - 100.0f;
                particle_data.velocities[i].x = 0.25 * fabs(particle_data.velocities[i].x);
            }
            if (cp.x + sz > 100.0f) {
                cp.x = 100.0f - sz;
                particle_data.velocities[i].x = -0.25 * fabs(particle_data.velocities[i].x);
            }
            if (cp.y - sz < -100.0f) {
                cp.y = sz - 100.0f;
                particle_data.velocities[i].y = 0.25 * fabs(particle_data.velocities[i].y);
            }
            if (cp.y + sz > 100.0f) {
                cp.y = 100.0f - sz;
                particle_data.velocities[i].y = -0.25 * fabs(particle_data.velocities[i].y);
            }
            if (cp.z - sz < -100.0f) {
                cp.z = sz - 100.0f;
                particle_data.velocities[i].z = 0.25 * fabs(particle_data.velocities[i].z);
            }
            if (cp.z + sz > 100.0f) {
                cp.z = 100.0f - sz;
                particle_data.velocities[i].z = -0.25 * fabs(particle_data.velocities[i].z);
            }
            particle_data.P[i] = cp;
        }
    }
}



