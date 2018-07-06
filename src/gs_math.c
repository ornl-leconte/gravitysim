
#include "gs_math.h"
#include <stdlib.h>
#include <math.h>

/* math/physics calculations */

mat4_t perspective(float angle, float aspect, float Znear, float Zfar) {

    float rads = M_PI * angle / 180.0;

    float tan_hovy = tanf(rads / 2.0);

    return mat4_create(
        1.0 / (aspect * tan_hovy), 0.0, 0.0, 0.0,
        0.0, 1.0 / tan_hovy, 0.0, 0.0,
        0.0, 0.0, -(Zfar+Znear)/(Zfar-Znear), -1.0,
        0.0, 0.0, -2.0 * Zfar * Znear / (Zfar - Znear), 0.0
    );
}

mat4_t frustrum(float l, float r, float b, float t, float n, float f) {
    mat4_t res = MAT4_0;
    
    return res;
}

mat4_t look_at(vec3_t camera, vec3_t target, vec3_t camera_euler) {
    vec3_t f = vec3_normalized(vec3_sub(target, camera));
    vec3_t u = vec3_normalized(camera_euler);
    vec3_t s = vec3_normalized(vec3_cross(f, u));
    u = vec3_cross(s, f);

    mat4_t res = MAT4_0;

    res.v[0][0] = s.x;
    res.v[1][0] = s.y;
    res.v[2][0] = s.z;

    res.v[0][1] = u.x;
    res.v[1][1] = u.y;
    res.v[2][1] = u.z;

    res.v[0][2] = -f.x;
    res.v[1][2] = -f.y;
    res.v[2][2] = -f.z;

    res.v[3][0] = -vec3_dot(s, camera);
    res.v[3][1] = -vec3_dot(u, camera);
    res.v[3][2] = vec3_dot(f, camera);

    return res;
}

mat4_t mat4_mul(mat4_t a, mat4_t b) {
    mat4_t res;

    int i;
    for (i = 0; i < 4; ++i) {
        int j;
        for (j = 0; j < 4; ++j) {
            res.v[i][j] = 0.0;
            int k;
            for (k = 0; k < 4; ++k) {
                res.v[i][j] += a.v[i][k] * b.v[k][j];
            }
        }
    }

    return res;
}

mat4_t mat4_scale(mat4_t a, float b) {
    mat4_t r;
    int i;
    for (i = 0; i < 4; ++i) {
        int j;
        for (j = 0; j < 4; ++j) {
            r.v[i][j] = a.v[i][j] * b;
        }
    }
    return r;
}

void dump_mat4(mat4_t a) {
    printf("[%2.2f,%2.2f,%2.2f,%2.2f]\n[%2.2f,%2.2f,%2.2f,%2.2f]\n[%2.2f,%2.2f,%2.2f,%2.2f]\n[%2.2f,%2.2f,%2.2f,%2.2f]\n",
        a.v[0][0], a.v[0][1], a.v[0][2], a.v[0][3],
        a.v[1][0], a.v[1][1], a.v[1][2], a.v[1][3],
        a.v[2][0], a.v[2][1], a.v[2][2], a.v[2][3],
        a.v[3][0], a.v[3][1], a.v[3][2], a.v[3][3]
    );
}


inline vec3_t vec3_cross(vec3_t a, vec3_t b) {

    float r_x = a.y * b.z - a.z * b.y;
    float r_y = a.x * b.z - a.z * b.x;
    float r_z = a.x * b.y - a.y * b.x;

    return V3(r_x, r_y, r_z);
}

mat4_t mat4_transpose(mat4_t a) {
    return mat4_create(
        a.v[0][0], a.v[1][0], a.v[2][0], a.v[3][0],
        a.v[0][1], a.v[1][1], a.v[2][1], a.v[3][1],
        a.v[0][2], a.v[1][2], a.v[2][2], a.v[3][2],
        a.v[0][3], a.v[1][3], a.v[2][3], a.v[3][3]
    );
}

mat4_t mat4_create(
    float v00, float v01, float v02, float v03,
    float v10, float v11, float v12, float v13,
    float v20, float v21, float v22, float v23,
    float v30, float v31, float v32, float v33
) {
    mat4_t r;
    r.v[0][0] = v00;
    r.v[0][1] = v01;
    r.v[0][2] = v02;
    r.v[0][3] = v03;

    r.v[1][0] = v10;
    r.v[1][1] = v11;
    r.v[1][2] = v12;
    r.v[1][3] = v13;

    r.v[2][0] = v20;
    r.v[2][1] = v21;
    r.v[2][2] = v22;
    r.v[2][3] = v23;

    r.v[3][0] = v30;
    r.v[3][1] = v31;
    r.v[3][2] = v32;
    r.v[3][3] = v33;
    return r;
}

inline vec3_t vec3_add(vec3_t a, vec3_t b) {
    return V3(a.x+b.x, a.y+b.y, a.z+b.z);
}
inline vec3_t vec3_sub(vec3_t a, vec3_t b) {
    return V3(a.x-b.x, a.y-b.y, a.z-b.z);
}
inline float vec3_dot(vec3_t a, vec3_t b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

inline vec3_t vec3_scale(vec3_t a, float b) {
    return V3(a.x*b, a.y*b, a.z*b);
}

inline vec3_t vec3_normalized(vec3_t a) {
    return vec3_scale(a, 1.0 / sqrt(a.x*a.x+a.y*a.y+a.z*a.z));
}

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


