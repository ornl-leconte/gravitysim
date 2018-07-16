
/*

subsection method is subsectioning the grid into N parts and then using the average for each part as an approximation for out-of-subsection particles

*/


#include "gs_physics.h"

#include <stdlib.h>
#include <string.h>

#include "ccgl_gl.h"

#include "part.h"

#define NUM_PARTITIONS (16)



struct {

    bool hasinit;
    
    partioning_t part;

} _pls = { false, NULL };


int subsec_split(const void * _a, const void * _b) {
    vec4_t a = GS.P[*(int *)_a], b = GS.P[*(int *)_b];
    return a.x > b.x;
}

void physics_loop_subsec() {
    if (!_pls.hasinit) {
        // init code
        partition_init(&_pls.part);

        _pls.hasinit = true;
    }

   // partition_nothing(&_pls.part);
    partition_xgrid(&_pls.part, 1.0);

    int _;
    for (_ = 0; _ < GS.N; ++_) {
        GS.F[_] = V4(0.0, 0.0, 0.0, 0.0);
    }

    int i_sec;
    for (i_sec = 0; i_sec < _pls.part.sections_len; ++i_sec) {
        section_t sec = _pls.part.sections[i_sec];

        int _i;
        for (_i = 0; _i < sec.len; ++_i) {
            int i = sec.idx[_i];

            GS.C[i] = get_nth_color(i_sec);

            int _j;
            for (_j = 0; _j < sec.len; ++_j) {
                int j = sec.idx[_j];
                if (i != j) {
                    // compute forces
                    GS.F[i] = vec4_add(GS.F[i], calculate_force(GS.P[i], GS.P[j]));
                }
            }
        }
    }

}



