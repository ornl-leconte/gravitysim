
#include "part.h"
#include "gs_physics.h"

#include <stdlib.h>

#include "qsort.h"

#define sort_by_x(a, b) (GS.P[*a].x<GS.P[*b].x)
#define sort_by_y(a, b) (GS.P[*a].y<GS.P[*b].y)
#define sort_by_z(a, b) (GS.P[*a].z<GS.P[*b].z)

// *  QSORT(char*, arr, n, islt);

void partition_init(partioning_t * part) {
    part->sections_len = 0;
    part->sections = NULL;
}

void part_3d_init(part_3d_t * part) {
    part->sections = NULL;
    part->Xdim = 0;
    part->Ydim = 0;
    part->Zdim = 0;
}

void part_3d_nothing(part_3d_t * part) {
    int old_len = part->Xdim * part->Ydim * part->Zdim;
    part->Xdim = 1;
    part->Ydim = 1;
    part->Zdim = 1;
    part->sections = (section_t *)realloc(part->sections, sizeof(section_t) * 1);
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

void part_3d_test(part_3d_t * part) {
    int old_len = part->Xdim * part->Ydim * part->Zdim;
    part->Xdim = 2;
    part->Ydim = 1;
    part->Zdim = 1;
    int new_len = part->Xdim * part->Ydim * part->Zdim;
    part->sections = (section_t *)realloc(part->sections, sizeof(section_t) * new_len);

    part->sections[0].len = 0;
    part->sections[1].len = 0;

    if (new_len > old_len) {
        part->sections[0].idx = NULL;
        part->sections[1].idx = NULL;
    }

    int i;
    for (i = 0; i < GS.N; ++i) {
        int idx = GS.P[i].x > 0.0;
        part->sections[idx].len++;
        part->sections[idx].idx = (int *)realloc(part->sections[idx].idx, part->sections[idx].len * sizeof(int));
        part->sections[idx].idx[part->sections[idx].len - 1] = i;
    }
}

void part_3d_grid(part_3d_t * part, int xN, int yN, int zN) {
    int old_len = part->Xdim * part->Ydim * part->Zdim;
    part->Xdim = xN;
    part->Ydim = yN;
    part->Zdim = zN;
    int new_len = part->Xdim * part->Ydim * part->Zdim;
    part->sections = (section_t *)realloc(part->sections, sizeof(section_t) * new_len);
    int i;

    for (i = 0; i < new_len; ++i) {
        part->sections[i].len = 0;
        if (i >= old_len) part->sections[i].idx = NULL;
    }

    for (i = 0; i < GS.N; ++i) {
        vec4_t p = GS.P[i];
        int xi = (int)floor(xN * (p.x + 100.0) / 200.0);
        int yi = (int)floor(yN * (p.y + 100.0) / 200.0);
        int zi = (int)floor(zN * (p.z + 100.0) / 200.0);
        if (xi < 0) xi = 0;
        if (yi < 0) yi = 0;
        if (zi < 0) zi = 0;

        if (xi >= xN) xi = xN - 1;
        if (yi >= yN) yi = yN - 1;
        if (zi >= zN) zi = zN - 1;

        int idx = PART3D_IDX((*part), xi, yi, zi);

        part->sections[idx].len++;
        part->sections[idx].idx = (int *)realloc(part->sections[idx].idx, part->sections[idx].len * sizeof(int));
        part->sections[idx].idx[part->sections[idx].len - 1] = i;
    }
}

// for xprop
struct {

    int * sorted;

} _p3dxp_data = { .sorted = NULL };

/*
int sort_by_x(const void * _a, const void * _b) {
    vec4_t a = GS.P[*(int *)_a];
    vec4_t b = GS.P[*(int *)_b];
    if (a.x > b.x) {
        return -1;
    } else if (a.x < b.x) {
        return 1;
    } else {
        return 0;
    }
}

int sort_by_y(const void * _a, const void * _b) {
    vec4_t a = GS.P[*(int *)_a];
    vec4_t b = GS.P[*(int *)_b];
    if (a.y > b.y) {
        return -1;
    } else if (a.y < b.y) {
        return 1;
    } else {
        return 0;
    }
}

int sort_by_z(const void * _a, const void * _b) {
    vec4_t a = GS.P[*(int *)_a];
    vec4_t b = GS.P[*(int *)_b];
    if (a.z > b.z) {
        return -1;
    } else if (a.z < b.z) {
        return 1;
    } else {
        return 0;
    }
}
*/

// extended, as in each slice has a y and z component
void part_3d_xyzprop(part_3d_t * part, int xN, int yN, int zN) {
    int old_len = part->Xdim * part->Ydim * part->Zdim;
    part->Xdim = xN;
    part->Ydim = yN;
    part->Zdim = zN;
    int new_len = part->Xdim * part->Ydim * part->Zdim;
    part->sections = (section_t *)realloc(part->sections, sizeof(section_t) * new_len);

    _p3dxp_data.sorted = (int *)realloc(_p3dxp_data.sorted, GS.N * sizeof(int));
    int i;
    for (i = 0; i < GS.N; ++i) _p3dxp_data.sorted[i] = i;

//    qsort(_p3dxp_data.sorted, GS.N, sizeof(int), sort_by_x);
    QSORT(int, _p3dxp_data.sorted, GS.N, sort_by_x);

    for (i = 0; i < new_len; ++i) {
        part->sections[i].len = 0;
        if (i >= old_len) part->sections[i].idx = NULL;
    }

    int xS = (int)ceilf((float)GS.N / xN);
    int yS = (int)ceilf((float)xS / yN);
    int zS = (int)ceilf((float)yS / zN);
    if (xS < 1) xS = 1;
    if (yS < 1) yS = 1;
    if (zS < 1) zS = 1;

    int x_idx, y_idx, z_idx;

    // actual sizes
    int axS, ayS;

    int cur_assign = 0;

    for (x_idx = 0; x_idx < xN; x_idx++) {
        axS = GS.N - xS * x_idx;
        if (axS > xS) axS = xS;
        //for (i = 0; i < xS && x_idx * xS + i < GS.N; ++i) cur_x_group[i] = _p3dxp_data.sorted[x_idx * xS + i];
        //axS = i;
        //qsort(_p3dxp_data.sorted + x_idx * xS, axS, sizeof(int), sort_by_y);
        QSORT(int, _p3dxp_data.sorted + x_idx * xS, axS, sort_by_y);

        for (y_idx = 0; y_idx < yN; ++y_idx) {
            ayS = axS - yS * y_idx;
            if (ayS > yS) ayS = yS;
            //for (i = 0; i < yS && y_idx * yS + i < axS; ++i) cur_y_group[i] = cur_x_group[y_idx * yS + i];
            //ayS = i;
            //qsort(cur_y_group, ayS, sizeof(int), sort_by_z);
            //qsort(_p3dxp_data.sorted + x_idx * xS + y_idx * yS, ayS, sizeof(int), sort_by_z);
            QSORT(int, _p3dxp_data.sorted + x_idx * xS + y_idx * yS, ayS, sort_by_z);
            for (z_idx = 0; z_idx < zN; ++z_idx) {
                int sec_idx = PART3D_IDX((*part), x_idx, y_idx, z_idx);

                vec4_t est = V4(0.0f, 0.0f, 0.0f, 0.0f);
                float total_mass = 0.0f;                
                for (i = 0; i < zS && z_idx * zS + i < ayS; ++i) {
                    int place = _p3dxp_data.sorted[cur_assign];
                    part->sections[sec_idx].len++;
                    part->sections[sec_idx].idx = (int *)realloc(part->sections[sec_idx].idx, sizeof(int) * part->sections[sec_idx].len);
                    part->sections[sec_idx].idx[part->sections[sec_idx].len - 1] = place;
                    cur_assign++;
                    total_mass += GS.P[place].w;
                    est = vec4_add(est, vec4_scale(GS.P[place], GS.P[place].w));
                }
                est.x = est.x / total_mass;
                est.y = est.y / total_mass;
                est.z = est.z / total_mass;
                est.w = total_mass;
                part->sections[sec_idx].est = est;
            }
        }
    }
    if (cur_assign != GS.N) { printf("ERR: NOT ALL HANDLED\n"); exit(1); }

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



