
#include "gs_math.h"
#include <stdlib.h>
#include <math.h>

#include "ccgl_gl.h"

/* math/physics calculations */


vec4_t get_nth_color(int n) {
    return V4(1.0, 0.0, 0.0, 1.0);    
}

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
float get_period(vec4_t point) {
    return atan2f(point.z, point.x);
}


mat4_t frustrum(float l, float r, float b, float t, float n, float f) {
    mat4_t res = MAT4_0;
    
    return res;
}

mat4_t look_at(vec4_t camera, vec4_t target, vec4_t camera_euler) {
    vec4_t za = vec4_normalized(vec4_sub(camera, target));
    vec4_t xa = vec4_normalized(vec4_cross_3(camera_euler, za));
    vec4_t ya = vec4_cross_3(za, xa);

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
vec4_t camera_orbit(vec4_t center, float dist, float period, float pitch) {
    vec4_t camera_pos = center;
    camera_pos.x += dist * (cosf(pitch) * sinf(period));
    camera_pos.y += dist * (sinf(pitch));
    camera_pos.z += dist * (cosf(pitch) * cosf(period));
    camera_pos.w = 0.0;
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


vec4_t vec4_cross_3(vec4_t a, vec4_t b) {

    float r_x = a.y * b.z - a.z * b.y;
    float r_y = a.z * b.x - a.x * b.z;
    float r_z = a.x * b.y - a.y * b.x;

    return V4(r_x, r_y, r_z, 0.0);
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

vec4_t vec4_add_2(vec4_t a, vec4_t b, vec4_t c) {
    return V4(a.x+b.x+c.x, a.y+b.y+c.y, a.z+b.z+c.z, a.w+b.w+c.w);
}
vec4_t vec4_add(vec4_t a, vec4_t b) {
    return V4(a.x+b.x, a.y+b.y, a.z+b.z, a.w+b.w);
}
vec4_t vec4_sub(vec4_t a, vec4_t b) {
    return V4(a.x-b.x, a.y-b.y, a.z-b.z, a.w-b.w);
}

float vec4_dot(vec4_t a, vec4_t b) {
    return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
}

vec4_t vec4_scale(vec4_t a, float b) {
    return V4(a.x*b, a.y*b, a.z*b, a.w*b);
}

vec4_t vec4_normalized(vec4_t a) {
    return vec4_scale(a, 1.0 / sqrt(a.x*a.x+a.y*a.y+a.z*a.z+a.w*a.w));
}


float calculate_distance_squared(vec4_t a, vec4_t b) {
    return SQR(a.x - b.x) + SQR(a.y - b.y) + SQR(a.z - b.z) + SQR(a.w - b.w);
}

float calculate_distance(vec4_t a, vec4_t b) {
    return SQRT(calculate_distance_squared(a, b));
}


/* generative functions */


// between 0.0 and 1.0 (including 0.0 and 1.0)
float random_float() {
    return (float)rand() / (float)(RAND_MAX - 1);
}

