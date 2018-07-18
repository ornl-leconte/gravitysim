/*

this handles rendering the scene

*/


#ifndef __RENDER_H__
#define __RENDER_H__



#include "ccgl_gl.h"

GLFWwindow *window;



struct {

    vec4_t light_pos;

    vec4_t cam_pos;

    mat4_t floor_model;

    float cam_dist, cam_period, cam_pitch, cam_fov;

} scene;


struct {
    // window size
    int win_width, win_height;

    // viewport size
    int vp_width, vp_height;

    // frame buffer length, 0 = no vsync, 1 = vsync, others are extra buffering
    int buffering;

    int framerate;

    bool show;

} render;

void render_init();

void control_update();

bool render_update();

#endif

