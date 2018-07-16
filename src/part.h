/*

Partioning logic

*/


#ifndef __PART_H__
#define __PART_H__

// for determining where it is
typedef struct bounding_box_t {

    float xmin, ymin, zmin;
    float xmax, ymax, zmax;

} bounding_box_t;

// stores a list of indexes of which particle positinos are in the section
typedef struct _section_t {

    bounding_box_t b_b;

    int len;
    int * idx;

} section_t;


typedef struct _partioning_t {

    int sections_len;
    section_t * sections;
    
} partioning_t;


void partition_init(partioning_t * part);

void partition_nothing(partioning_t * part);

void partition_xgrid(partioning_t * part, float size);

#endif

