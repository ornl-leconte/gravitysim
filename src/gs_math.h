
/*

math routines for gravity sim

*/

#ifndef __GS_MATH_H__
#define __GS_MATH_H__

#include <math.h>

#include "gravitysim.h"

#define SQR(x) ((x)*(x))
#define SQRT(x) (sqrt((x)))


#define MAT4_0 mat4_create(0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0)

#define MAT4_I mat4_create( \
        1.0, 0.0, 0.0, 0.0, \
        0.0, 1.0, 0.0, 0.0, \
        0.0, 0.0, 1.0, 0.0, \
        0.0, 0.0, 0.0, 1.0 \
    )

vec3_t vec3_add(vec3_t a, vec3_t b);
vec3_t vec3_sub(vec3_t a, vec3_t b);
float vec3_dot(vec3_t a, vec3_t b);
vec3_t vec3_scale(vec3_t a, float b);

mat4_t mat4_scale(mat4_t a, float b);

mat4_t perspective(float rads, float aspect, float Znear, float Zfar);

mat4_t rotator(float yaw, float pitch, float roll);

mat4_t scaler(float X, float Y, float Z);

float get_period(vec3_t point);

mat4_t translator(float Xoff, float Yoff, float Zoff);

mat4_t mat4_transpose(mat4_t a);

mat4_t mat4_mul(mat4_t a, mat4_t b);

vec3_t vec3_normalized(vec3_t a) ;
vec3_t vec3_cross(vec3_t a, vec3_t b);

mat4_t look_at(vec3_t camera, vec3_t target, vec3_t camera_euler);

float calculate_distance_squared(vec3_t a, vec3_t b);

void dump_mat4(mat4_t a);

// return distance between 'a' and 'b'
inline float calculate_distance(vec3_t a, vec3_t b);
inline float vec3_normscale(vec3_t a);

mat4_t mat4_create(
    float v00, float v01, float v02, float v03,
    float v10, float v11, float v12, float v13,
    float v20, float v21, float v22, float v23,
    float v30, float v31, float v32, float v33
);


float float_gen_default(float fa, float fr);
// generates stuff

vec3_t vec3_gen_default(float xa, float xr, float ya, float yr, float za, float zr);

// physics routines (master)
void physics_loop_basic();


#endif


