
#include "gs_math.h"
#include <stdlib.h>

/* math/physics calculations */

inline float calculate_distance_squared(vec3_t a, vec3_t b) {
    return SQR(a.x - b.x) + SQR(a.y - b.y) + SQR(a.z - b.z);
}

inline float calculate_distance(vec3_t a, vec3_t b) {
    return SQRT(calculate_distance_squared(a, b));
}

inline float calculate_force(total_particle_state_t a, total_particle_state_t b) {
    return (gravity_coef * a.mass * b.mass) / calculate_distance_squared(a.position, b.position);
}


/* generative functions */


// between 0.0 and 1.0 (including 0.0 and 1.0)
float random_float() {
    return (float)rand() / (float)(RAND_MAX - 1);
}


/*


these points are centered around 'center' and have a max difference of center_range in each respective channel

They exist within the rectangular prism

*/

total_particle_state_t generate_state_default(float avg_mass, float mass_range, vec3_t center, vec3_t center_range, vec3_t avg_velocity, vec3_t velocity_range) {
    vec3_t new_pos = center;
    new_pos.x += (random_float() - 0.5) * center_range.x;
    new_pos.y += (random_float() - 0.5) * center_range.y;
    new_pos.z += (random_float() - 0.5) * center_range.z;
    float new_mass = avg_mass + (random_float() - 0.5) * mass_range;
    vec3_t new_vel = avg_velocity;
    new_vel.x += (random_float() - 0.5) * velocity_range.x;
    new_vel.y += (random_float() - 0.5) * velocity_range.y;
    new_vel.z += (random_float() - 0.5) * velocity_range.z;

    total_particle_state_t res;
    res.mass = new_mass;
    res.position = new_pos;
    res.velocity = new_vel;

    return res;
}


