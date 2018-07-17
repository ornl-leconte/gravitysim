/*

Partioning logic

*/


#ifndef __PART_H__
#define __PART_H__


#include "gravitysim.h"


#define PART3D_IDX(p, x, y, z) ((x) + p.Xdim * (y) + p.Xdim * p.Ydim * (z))

// for determining where it is
typedef struct bounding_box_t {

    float xmin, ymin, zmin;
    float xmax, ymax, zmax;

} bounding_box_t;

// stores a list of indexes of which particle positinos are in the section
typedef struct _section_t {

    bounding_box_t b_b;

    vec4_t est;

    int len;
    int * idx;

} section_t;


typedef struct _partioning_t {

    int sections_len;
    section_t * sections;
    
} partioning_t;

typedef struct _part_3d_t {


    int Xdim, Ydim, Zdim;

    // total length is Xdim * Ydim * Zdim
    section_t * sections;

} part_3d_t;


void partition_init(partioning_t * part);

void partition_nothing(partioning_t * part);

void partition_xgrid(partioning_t * part, float size);


void part_3d_init(part_3d_t * part);

void part_3d_nothing(part_3d_t * part);

void part_3d_test(part_3d_t * part);

void part_3d_grid(part_3d_t * part, int xN, int yN, int zN);

void part_3d_xprop(part_3d_t * part, int xN);

void part_3d_xyzprop(part_3d_t * part, int xN, int yN, int zN);

#endif

