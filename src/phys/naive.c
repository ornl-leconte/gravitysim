
#include "gs_physics.h"

void physics_loop_naive() {
    int i;
    
    for (i = 0; i < GS.N; ++i) {
        GS.F[i] = V4(0.0, 0.0, 0.0, 0.0);
    }

    for (i = 0; i < GS.N; ++i) {
        vec4_t i_p = GS.P[i];
        
        int j;
        for (j = 0; j < i; ++j) {
            vec4_t j_p = GS.P[j];

            // internal force calculation
            vec4_t force = calculate_force(i_p, j_p);

            // add it directionally so they are attracted to the medium point
            GS.F[i] = vec4_add(GS.F[i], force);
            GS.F[j] = vec4_sub(GS.F[j], force);
        }
    }
}



