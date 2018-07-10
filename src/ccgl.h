/*

ccgl.h - header for Cade's Cool Graphics Library (or at least a start for it)

Contains basic 3D manipulation (including API-agnostic datatypes and routines for loading files)

It also includes OpenGL specific code for programs, shaders, etc

And basic mat4, vec3 operations for transformations


Check gs_math.c and gs_math.h for most pure path routines


*/


#ifndef __CCGL_H__
#define __CCGL_H__




#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif


typedef struct _vec3_t {
    float x, y, z;
} vec3_t;

typedef struct _mat4_t {
    float v[4][4];
} mat4_t;


// quick creations
#define V3(x, y, z) ((vec3_t){(x), (y), (z)})


#define M4_I M4_A( \
    1, 0, 0, 0,    \
    0, 1, 0, 0,    \
    0, 0, 1, 0,    \
    0, 0, 0, 1     \
)

// unwrap (for function calls)
#define UV3(v3) (v3).x, (v3).y, (v3).z





#endif

