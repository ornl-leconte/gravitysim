
#include "gs_math.h"
#include <stdlib.h>
#include <math.h>

#include "ccgl_gl.h"

/* math/physics calculations */


mat4_t perspective(float FOV_rads, float aspect, float Znear, float Zfar) {

    float top = Znear * tanf(FOV_rads / 2.0);
    float bottom = -top;
    float right = top * aspect;
    float left = -right;

    return mat4_create(
        2.0 * Znear / (right - left), 0.0, (right + left) / (right - left), 0.0,
        0.0, 2 * Znear / (top - bottom), (top + bottom) / (top - bottom), 0.0,
        0.0, 0.0, -(Zfar+Znear)/(Zfar-Znear), -2.0 * (Zfar * Znear) / (Zfar - Znear),
        0.0, 0.0,  -1.0, 1.0
    );
}

mat4_t scaler(float X, float Y, float Z) {
    return mat4_create(
             X,      0.0,      0.0,     0.0,
             0.0,      Y,      0.0,     0.0,
             0.0,      0.0,      Z,     0.0,
             0.0,      0.0,      0.0,     1.0
    );
}


mat4_t translator(float Xoff, float Yoff, float Zoff) {
    return mat4_create(
             1.0,      0.0,      0.0,     Xoff,
             0.0,      1.0,      0.0,     Yoff,
             0.0,      0.0,      1.0,     Zoff,
             0.0,      0.0,      0.0,     1.0
    );
}

mat4_t rotator(float yaw, float pitch, float roll) {
    float X = yaw, Y = pitch, Z = roll;
    mat4_t r_x = mat4_create(
             1.0,      0.0,      0.0,     0.0,
             0.0,  cosf(X), -sinf(X),     0.0,
             0.0,  sinf(X),  cosf(X),     0.0,
             0.0,      0.0,      0.0,     1.0
    );
    mat4_t r_y = mat4_create(
         cosf(Y),      0.0,  sinf(Y),     0.0,
             0.0,      1.0,      0.0,     0.0,
        -sinf(Y),      0.0,  cosf(Y),     0.0,
             0.0,      0.0,      0.0,     1.0
    );
    mat4_t r_z = mat4_create(
         cosf(Z), -sinf(Z),      0.0,     0.0,  
         sinf(Z),  cosf(Z),      0.0,     0.0, 
             0.0,      0.0,      1.0,     0.0,
             0.0,      0.0,      0.0,     1.0
    );

    return mat4_mul(r_z, mat4_mul(r_y, r_x));

}

// returns radians from center
float get_period(vec3_t point) {
    return atan2f(point.z, point.x);
}


mat4_t frustrum(float l, float r, float b, float t, float n, float f) {
    mat4_t res = MAT4_0;
    
    return res;
}

mat4_t look_at(vec3_t camera, vec3_t target, vec3_t camera_euler) {
    vec3_t za = vec3_normalized(vec3_sub(camera, target));
    vec3_t xa = vec3_normalized(vec3_cross(camera_euler, za));
    vec3_t ya = vec3_cross(za, xa);

    mat4_t orient = mat4_create(
        xa.x, xa.y, xa.z, 0.0,
        ya.x, ya.y, ya.z, 0.0,
        za.x, za.y, za.z, 0.0,
        0.0, 0.0, 0.0, 1.0
    );

    mat4_t trans = mat4_create(
        1.0, 0.0, 0.0, -camera.x,
        0.0, 1.0, 0.0, -camera.y,
        0.0, 0.0, 1.0, -camera.z,
        0.0, 0.0, 0.0, 1.0
    );

    return mat4_mul(orient, trans);
}


// period is the period in the XZ plane
vec3_t camera_orbit(vec3_t center, float dist, float period, float pitch) {
    vec3_t camera_pos = center;
    camera_pos.x += dist * (cosf(pitch) * sinf(period));
    camera_pos.y += dist * (sinf(pitch));
    camera_pos.z += dist * (cosf(pitch) * cosf(period));
    return camera_pos;
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
    printf("[%+2.2f ,%+2.2f ,%+2.2f ,%+2.2f]\n[%+2.2f ,%+2.2f ,%+2.2f ,%+2.2f]\n[%+2.2f ,%+2.2f ,%+2.2f ,%+2.2f]\n[%+2.2f ,%+2.2f ,%+2.2f ,%+2.2f]\n",
        a.v[0][0], a.v[0][1], a.v[0][2], a.v[0][3],
        a.v[1][0], a.v[1][1], a.v[1][2], a.v[1][3],
        a.v[2][0], a.v[2][1], a.v[2][2], a.v[2][3],
        a.v[3][0], a.v[3][1], a.v[3][2], a.v[3][3]
    );
}


inline vec3_t vec3_cross(vec3_t a, vec3_t b) {

    float r_x = a.y * b.z - a.z * b.y;
    float r_y = a.z * b.x - a.x * b.z;
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
    return vec3_scale(a, vec3_normscale(a));
}

inline float vec3_normscale(vec3_t a) {
    return 1.0 / sqrt(a.x*a.x+a.y*a.y+a.z*a.z);
}

inline float calculate_distance_squared(vec3_t a, vec3_t b) {
    return SQR(a.x - b.x) + SQR(a.y - b.y) + SQR(a.z - b.z);
}

inline float calculate_distance(vec3_t a, vec3_t b) {
    return SQRT(calculate_distance_squared(a, b));
}

float calculate_force(vec3_t a_pos, float a_mass, vec3_t b_pos, float b_mass) {
    return gravity_coef * a_mass * b_mass / (calculate_distance_squared(a_pos, b_pos));
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

float float_gen_default(float fa, float fr) {
    return fa + (random_float() - 0.5) * fr;
}

vec3_t vec3_gen_default(float xa, float xr, float ya, float yr, float za, float zr) {
    vec3_t res;
    res.x = float_gen_default(xa, xr);
    res.y = float_gen_default(ya, yr);
    res.z = float_gen_default(za, zr);
    return res;
}


/* control loop */

void physics_init() {
}

// naive, O(n_particles^2)
void physics_loop_basic() {

    float cur_dt = GS_looptime;//glfwGetTime() - last_loop_time;
    int i;

    for (i = 0; i < n_particles; ++i) {
        particle_data.forces[i] = V3(0.0, 0.0, 0.0);
    }

    physics_data.weighted_pos = V3(0.0, 0.0, 0.0);
    physics_data.avg_pos = V3(0.0, 0.0, 0.0);
    physics_data.std_pos = V3(0.0, 0.0, 0.0);

    int col = 0;
    int enabled = 0;

    int splits = 0, joins = 0; 

    for (i = 0; i < n_particles; ++i) {

        int j;
        for (j = n_particles - 1; j > i; --j) {
            if (!(particle_data.is_enabled[i] && particle_data.is_enabled[j])) {
                continue;
            }
        
            vec3_t i_pos = particle_data.positions[i];
            float i_mass = particle_data.masses[i];
            vec3_t j_pos = particle_data.positions[j];
            float j_mass = particle_data.masses[j];
            vec3_t force_dir = vec3_sub(j_pos, i_pos);
            bool operation_happened = false;

            float coef = 1.0;
            if (calculate_distance(i_pos, j_pos) / 0.75 <= MASS_TO_SIZE(i_mass) + MASS_TO_SIZE(j_mass)) {
                // collision has happened
                col++;

                if (calculate_distance(i_pos, j_pos) / 0.56 <= MASS_TO_SIZE(i_mass) + MASS_TO_SIZE(j_mass) && (calculate_distance(particle_data.velocities[i], particle_data.velocities[i]) >= 1.0) && (MASS_TO_SIZE(i_mass) >= 2.0 || MASS_TO_SIZE(j_mass) >= 2.0) &&(n_particles - particle_data._num_enabled) >= 1) {
                    // split them apart if this is the case
                    splits++;
                    int bigger_idx;
                    float bigger_mass;
                    vec3_t bigger_force;
                    vec3_t bigger_position;
                    if (MASS_TO_SIZE(i_mass) > MASS_TO_SIZE(j_mass)) {
                        bigger_idx = i;
                    } else {
                        bigger_idx = j;
                    }
                    bigger_mass = particle_data.masses[bigger_idx];
                    bigger_force = particle_data.forces[bigger_idx];
                    bigger_position = particle_data.positions[bigger_idx];

                    int new_idx = -1;
                    int k;
                    for (k = 0; k < n_particles; ++k){
                        if (!particle_data.is_enabled[k]) {
                            new_idx = k;
                            break;
                        }
                    }
                    if (new_idx < 0) {
                        log_error("couldn't find available particle to split");
                    }

                    particle_data.is_enabled[new_idx] = true;
                    particle_data.masses[new_idx] = bigger_mass / 2.0;
                    particle_data.forces[new_idx] = bigger_force;
                    particle_data.positions[new_idx] = bigger_position;
                    float new_size = MASS_TO_SIZE(particle_data.masses[new_idx]);
                    particle_data.positions[new_idx] = vec3_add(particle_data.positions[new_idx], 
                        vec3_scale(V3(0.0, 0.0, 0.0), new_size));
                } else {
                    joins++;
                    // join them together
                    float i_mass_prop = i_mass / (i_mass + j_mass);
                    particle_data.positions[i] = vec3_add(vec3_scale(i_pos, i_mass_prop), vec3_scale(j_pos, 1.0 - i_mass_prop));

                    vec3_t i_f = particle_data.forces[i], j_f = particle_data.forces[j];
                    vec3_t n_f = V3(0.0, 0.0, 0.0);

                    float j_c = j_mass / (j_mass + i_mass);

                    n_f.x = 1.0 * i_f.x + j_f.x * j_c;
                    n_f.y = 1.0 * i_f.y + j_f.y * j_c;
                    n_f.z = 1.0 * i_f.z + j_f.z * j_c;

                    particle_data.forces[i] = n_f;

                    particle_data.masses[i] += j_mass;
                    particle_data.is_enabled[j] = false;
                    particle_data._num_enabled--;
                    operation_happened = true;
                }
            }
            if (!operation_happened) {
                float force_mag = coef * calculate_force(i_pos, i_mass, j_pos, j_mass);
                if (force_mag > 0.0) {

                    particle_data.forces[i] = vec3_add(particle_data.forces[i], vec3_scale(force_dir, force_mag));
                    particle_data.forces[j] = vec3_add(particle_data.forces[j], vec3_scale(force_dir, -force_mag));
                }
            }
        }
    }

    float sum_mass = 0.0;

    for (i = 0; i < n_particles; ++i) {
        if (particle_data.is_enabled[i]) {
            enabled++;
            // acceleration = foce / mass (with timestep for discrete integration)
            vec3_t c_acc_d = vec3_scale(particle_data.forces[i], 1.0 / particle_data.masses[i]);

            // integrate it
            particle_data.velocities[i] = vec3_add(particle_data.velocities[i], vec3_scale(c_acc_d, cur_dt));
            // update position
            particle_data.positions[i] = vec3_add(particle_data.positions[i], vec3_scale(particle_data.velocities[i], cur_dt));

            // update statistics
            physics_data.avg_pos = vec3_add(physics_data.avg_pos, particle_data.positions[i]);

            physics_data.weighted_pos = vec3_add(physics_data.weighted_pos, vec3_scale(particle_data.positions[i], particle_data.masses[i]));
            sum_mass += particle_data.masses[i];

        }
    }

    physics_data.avg_pos = vec3_scale(physics_data.avg_pos, 1.0 / enabled);
    physics_data.weighted_pos = vec3_scale(physics_data.weighted_pos, sum_mass);

    for (i = 0; i < n_particles; ++i) {
        if (particle_data.is_enabled[i]) {
            vec3_t cdiff = vec3_sub(particle_data.positions[i], physics_data.avg_pos);
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

    physics_data.splits = splits;
    physics_data.joins = joins;

}


