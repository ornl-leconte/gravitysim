
#include "gs_physics.h"

#include "gs_math.h"

#include "part.h"

#include <math.h>

#define SOFT_FACTOR 0.25f


vec4_t calculate_force(vec4_t a, vec4_t b) {
    vec4_t r;
    r.x = b.x - a.x;
    r.y = b.y - a.y;
    r.z = b.z - a.z;
    float dist_sqr = r.x * r.x + r.y * r.y + r.z * r.z + SOFT_FACTOR * SOFT_FACTOR;

    float s = GS.G * a.w * b.w / dist_sqr;

    r.x *= s;
    r.y *= s;
    r.z *= s;
    r.w = 0.0;
    return r;
}

vec4_t particle_get_position(vec4_t a) {
    return V4(a.x, a.y, a.z, 0.0);
}

/* control loop */

void physics_init() {
}

void physics_update_positions() {
    int i;
    for (i = 0; i < GS.N; ++i) {
        // acceleration = force / mass (with timestep for discrete integration), because F = ma
        // V(t) = integral{A(t) * dt}
        // P(t) = integral{V(t) * dt}
        vec4_t P = GS.P[i];
        vec4_t V = GS.V[i];
        V.w = 0.0f;
        GS.V[i] = vec4_add(V, vec4_scale(GS.F[i], GS.dt / P.w));
        GS.P[i] = vec4_add(P, vec4_scale(V, GS.dt));
    }
}


// collision data
struct {

    partioning_t partition;

    bool hasinit;

} pch_data = { .hasinit = false };

void _internal_collision(int i, int j) {
    //if (i == j) return;

    vec4_t i_p = GS.P[i], j_p = GS.P[j];

    float i_s = MASS_TO_SIZE(i_p.w);
    float j_s = MASS_TO_SIZE(j_p.w);

    float dist = calculate_distance(V4(i_p.x, i_p.y, i_p.z, 0.0), V4(j_p.x, j_p.y, j_p.z, 0.0));
    if (dist > 0.0 && dist <= i_s + j_s) {

        vec4_t diff_p = vec4_sub(j_p, i_p);
        diff_p.w = 0.0;
        vec4_t diff_v = vec4_sub(GS.V[j], GS.V[i]);
        diff_v.w = 0.0;

        float force_res = vec4_dot(diff_p, diff_v);
        //if true, i is moving towards j, so we are colliding and want to deflect
        if (force_res < 0.0) {
            // we shouldn't need the / 2.0, but it works well
            float overlap = (i_s + j_s) - dist;

            vec4_t penalty = vec4_scale(vec4_normalized(diff_p), GS.coll_B); //fabs(force_res) * 

            // these are coefficients based on how close each particle is
            float i_prop = overlap / (i_s);
            if (i_prop > 1.0f) i_prop = 1.0f;

            float j_prop = overlap / (j_s);
            if (j_prop > 1.0f) j_prop = 1.0f;

            GS.F[i] = vec4_sub(GS.F[i], vec4_scale(penalty, i_prop));
            GS.F[j] = vec4_add(GS.F[j], vec4_scale(penalty, j_prop));
            //cols++;
        }
    }
}

void physics_collision_handle() {
    
    if (!pch_data.hasinit) {

        partition_init(&pch_data.partition);

        pch_data.hasinit = true;
    }

    partition_xgrid(&pch_data.partition, 0.25);
    //partition_nothing(&pch_data.partition);

    int i;
    for (i = 0; i < pch_data.partition.sections_len; ++i) {
        int j;
        for (j = 0; j < pch_data.partition.sections[i].len; ++j) {
            // now we should only have to iterate on close sections
            int a_idx = pch_data.partition.sections[i].idx[j];

            int seek_sec_start = i - 2, seek_sec_end = i;
            if (seek_sec_start < 0) seek_sec_start = 0;

            int seek_sec;
            for (seek_sec = seek_sec_start; seek_sec <= seek_sec_end; ++seek_sec) {
                int k;
                for (k = 0; k < pch_data.partition.sections[seek_sec].len; ++k) {
                    int b_idx = pch_data.partition.sections[seek_sec].idx[k];
                    if (a_idx > b_idx) _internal_collision(a_idx, b_idx);
                }
            }
        }
    }

/*
    int i;
    int cols = 0;

    for (i = 0; i < GS.N; ++i) {
        int j;
        for (j = 0; j < i; ++j) {
            _internal_collision(i, j);
        }
    }
    */

}

float clamp_val(float x, float mn, float mx) {
    if (x < mn) return mn;
    if (x > mx) return mx;
    return x;
}

void physics_clamp_positions() {
    int i;
    for (i = 0; i < GS.N; ++i) {
        vec4_t cp = GS.P[i];
        float sz = MASS_TO_SIZE(cp.w);
        if (cp.x - sz < -100.0f) {
            cp.x = sz - 100.0f;
            GS.V[i].x = 0.25 * fabs(GS.V[i].x);
        }
        if (cp.x + sz > 100.0f) {
            cp.x = 100.0f - sz;
            GS.V[i].x = -0.25 * fabs(GS.V[i].x);
        }
        if (cp.y - sz < -100.0f) {
            cp.y = sz - 100.0f;
            GS.V[i].y = 0.25 * fabs(GS.V[i].y);
        }
        if (cp.y + sz > 100.0f) {
            cp.y = 100.0f - sz;
            GS.V[i].y = -0.25 * fabs(GS.V[i].y);
        }
        if (cp.z - sz < -100.0f) {
            cp.z = sz - 100.0f;
            GS.V[i].z = 0.25 * fabs(GS.V[i].z);
        }
        if (cp.z + sz > 100.0f) {
            cp.z = 100.0f - sz;
            GS.V[i].z = -0.25 * fabs(GS.V[i].z);
        }
        GS.P[i] = cp;
    }
}



