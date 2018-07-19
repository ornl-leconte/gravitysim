
/*

subsection method is subsectioning the grid into N parts and then using the average for each part as an approximation for out-of-subsection particles

*/


#include "gs_physics.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ccgl_gl.h"

#include "part.h"


struct {

    bool hasinit;
    
    part_3d_t part;

} _pls = { false };


void physics_loop_subsec() {
    if (!_pls.hasinit) {
        // init code
        part_3d_init(&_pls.part);

        _pls.hasinit = true;
    }

   // partition_nothing(&_pls.part);
    //partition_xgrid(&_pls.part, 1.0);
    //part_3d_nothing(&_pls.part);
    //part_3d_test(&_pls.part);
    //part_3d_grid(&_pls.part, 10, 10, 10);
    //part_3d_xprop(&_pls.part, 10);
    // this works best, 10,10,10 is typically slower
    
    if (GS.N >= 5 * 16 * 16 * 16) {
        part_3d_xyzprop(&_pls.part, 16, 16, 16);
    } else if (GS.N >= 5 * 16 * 8 * 8) {
        part_3d_xyzprop(&_pls.part, 16, 8, 8);
    } else if (GS.N >= 5 * 8 * 4 * 4) {
        part_3d_xyzprop(&_pls.part, 8, 4, 4);
    } else {
        part_3d_xyzprop(&_pls.part, 1, 1, 1);
    }

    int _;
    for (_ = 0; _ < GS.N; ++_) {
        GS.F[_] = V4(0.0, 0.0, 0.0, 0.0);
    }


    int xi, yi, zi;
    for (xi = 0; xi < _pls.part.Xdim; ++xi)
    for (yi = 0; yi < _pls.part.Ydim; ++yi)
    for (zi = 0; zi < _pls.part.Zdim; ++zi) {
        int node_num = PART3D_IDX(_pls.part, xi, yi, zi);
        section_t seci = _pls.part.sections[PART3D_IDX(_pls.part, xi, yi, zi)];

        if (seci.len == 0) continue;

        int _i;
        for (_i = 0; _i < seci.len; ++_i) {
            int i = seci.idx[_i];

            GS.C[i] = get_part_color(xi, _pls.part.Xdim, yi, _pls.part.Ydim, zi, _pls.part.Zdim);

            int xj, yj, zj;

            int xj_min = xi - 1, yj_min = yi - 1, zj_min = zi - 1;
            int xj_max = xi + 1, yj_max = yi + 1, zj_max = zi + 1;

            if (xj_min < 0) xj_min = 0;
            if (yj_min < 0) yj_min = 0;
            if (zj_min < 0) zj_min = 0;

            if (xj_max >= _pls.part.Xdim) xj_max = _pls.part.Xdim - 1;
            if (yj_max >= _pls.part.Ydim) yj_max = _pls.part.Ydim - 1;
            if (zj_max >= _pls.part.Zdim) zj_max = _pls.part.Zdim - 1;

            for (xj = 0; xj < _pls.part.Xdim; ++xj) 
            for (yj = 0; yj < _pls.part.Ydim; ++yj) 
            for (zj = 0; zj < _pls.part.Zdim; ++zj) {
                section_t secj = _pls.part.sections[PART3D_IDX(_pls.part, xj, yj, zj)];

                if (secj.len == 0) continue;

                if (xj >= xj_min && xj <= xj_max && yj >= yj_min && yj <= yj_max && zj >= zj_min && zj <= zj_max) {
                    int _j;
                    for (_j = 0; _j < secj.len; ++_j) {
                        int j = secj.idx[_j];

                        GS.F[i] = vec4_add(GS.F[i], calculate_force(GS.P[i], GS.P[j]));
                    }
                } else {
                    GS.F[i] = vec4_add(GS.F[i], calculate_force(GS.P[i], secj.est));
                }
            }
        }
    }
}



