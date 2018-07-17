
#include "gs_physics.h"

#include "gs_math.h"

#include "ccgl_gl.h"

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
    r.w = 0.0f;
    return r;
}

vec4_t particle_get_position(vec4_t a) {
    return V4(a.x, a.y, a.z, 0.0f);
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
        GS.V[i] = vec4_add(V, vec4_scale(GS.F[i], GS.ph_dt / P.w));
        GS.P[i] = vec4_add(P, vec4_scale(V, GS.ph_dt));
    }
}


// collision data
struct {

    part_3d_t part;


    bool hasinit;

} pch_data = { .hasinit = false };

bool _internal_collision(int i, int j) {
    if (i == j) return false;

    vec4_t i_p = GS.P[i], j_p = GS.P[j];

    float i_s = MASS_TO_SIZE(i_p.w);
    float j_s = MASS_TO_SIZE(j_p.w);

    float dist = calculate_distance(V4(i_p.x, i_p.y, i_p.z, 0.0f), V4(j_p.x, j_p.y, j_p.z, 0.0f));
    if (dist > 0.0 && dist <= i_s + j_s) {

        vec4_t diff_p = vec4_sub(j_p, i_p);
        diff_p.w = 0.0;
        vec4_t diff_v = vec4_sub(GS.V[j], GS.V[i]);
        diff_v.w = 0.0;

        float force_res = vec4_dot(diff_p, diff_v);
        //if true, i is moving towards j, so we are colliding and want to deflect
        if (force_res > 0.0) {
            // we shouldn't need the / 2.0, but it works well
            float overlap = (i_s + j_s) - dist;

            vec4_t penalty = vec4_scale(vec4_normalized(diff_p), GS.coll_B); //fabs(force_res) * 

            // these are coefficients based on how close each particle is
            float i_prop = overlap / (i_s);
            if (i_prop > 1.0f) i_prop = 1.0f;
            if (i_prop < 0.1f) i_prop = 0.1f;

            float j_prop = overlap / (j_s);
            if (j_prop > 1.0f) j_prop = 1.0f;
            if (j_prop < 0.1f) j_prop = 0.1f;

            GS.F[i] = vec4_sub(GS.F[i], vec4_scale(penalty, i_prop));
            GS.F[j] = vec4_add(GS.F[j], vec4_scale(penalty, j_prop));
            //cols++;
            return true;
        }
    }
    return false;
}

void physics_collision_handle() {
    
    if (!pch_data.hasinit) {

        part_3d_init(&pch_data.part);

        pch_data.hasinit = true;
    }


    if (GS.coll_B <= 0.0000001f) return;

    if (GS.N >= 2 * 12 * 12 * 12) {
        part_3d_xyzprop(&pch_data.part, 12, 12, 12);
    } else if (GS.N >= 2 * 8 * 4 * 4) {
        part_3d_xyzprop(&pch_data.part, 8, 4, 4);
    } else {
        part_3d_xyzprop(&pch_data.part, 1, 1, 1);
    }

    int cols = 0;

    int xi, yi, zi;
    for (xi = 0; xi < pch_data.part.Xdim; ++xi)
    for (yi = 0; yi < pch_data.part.Ydim; ++yi)
    for (zi = 0; zi < pch_data.part.Zdim; ++zi) {
        int node_num = PART3D_IDX(pch_data.part, xi, yi, zi);
        section_t seci = pch_data.part.sections[PART3D_IDX(pch_data.part, xi, yi, zi)];

        if (seci.len == 0) continue;

        int _i;
        for (_i = 0; _i < seci.len; ++_i) {
            int i = seci.idx[_i];

            int xj, yj, zj;

            int xj_min = xi - 1, yj_min = yi - 1, zj_min = zi - 1;
            int xj_max = xi + 1, yj_max = yi + 1, zj_max = zi + 1;

            if (xj_min < 0) xj_min = 0;
            if (yj_min < 0) yj_min = 0;
            if (zj_min < 0) zj_min = 0;

            if (xj_max >= pch_data.part.Xdim) xj_max = pch_data.part.Xdim - 1;
            if (yj_max >= pch_data.part.Ydim) yj_max = pch_data.part.Ydim - 1;
            if (zj_max >= pch_data.part.Zdim) zj_max = pch_data.part.Zdim - 1;

            for (xj = xj_min; xj <= xj_max; ++xj) 
            for (yj = yj_min; yj <= yj_max; ++yj) 
            for (zj = zj_min; zj <= zj_max; ++zj) {

                section_t secj = pch_data.part.sections[PART3D_IDX(pch_data.part, xj, yj, zj)];
                if (secj.len == 0) continue;

                int _j;
                for (_j = 0; _j < secj.len; ++_j) {
                    int j = secj.idx[_j];
                    //if (i > j) 
                        cols += _internal_collision(i, j);
                }
            }
        }
    }
    printf("collisions: %d\n", cols);
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



