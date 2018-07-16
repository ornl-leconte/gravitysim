
#include "part.h"
#include "gs_physics.h"

#include <stdlib.h>

void partition_init(partioning_t * part) {
    part->sections_len = 0;
    part->sections = NULL;
}

// this just groups it all as one
void partition_nothing(partioning_t * part) {
    int old_len = part->sections_len;
    
    part->sections_len = 1;
    part->sections = (section_t *)realloc(part->sections, sizeof(section_t) * part->sections_len);


    part->sections[0].len = GS.N;
    if (old_len >= 1) {
        part->sections[0].idx = (int *)realloc(part->sections[0].idx, sizeof(int) * GS.N);
    } else {
        part->sections[0].idx = (int *)malloc(sizeof(int) * GS.N);
    }

    int i;
    for (i = 0; i < GS.N; ++i) {
        part->sections[0].idx[i] = i;
    }
}

// simple grid boi
void partition_xgrid(partioning_t * part, float size) {
    int i;
    int old_len = part->sections_len;

    part->sections_len = (int)ceil(200.0 / size); 
    part->sections = (section_t *)realloc(part->sections, sizeof(section_t) * part->sections_len);

    for (i = 0; i < part->sections_len; ++i) {
        part->sections[i].len = 0;
        if (i >= old_len) {
            part->sections[i].idx = NULL;
        }
    }

    for (i = 0; i < GS.N; ++i) {
        int sec_num = (int)floor(part->sections_len * (GS.P[i].x + 100.0) / 200.0);
        if (sec_num < 0) sec_num = 0;
        if (sec_num >= part->sections_len) sec_num = part->sections_len - 1;
        part->sections[sec_num].len++;
        part->sections[sec_num].idx = (int *)realloc(part->sections[sec_num].idx, sizeof(int) * part->sections[sec_num].len);
        part->sections[sec_num].idx[part->sections[sec_num].len - 1] = i;
    }
}



