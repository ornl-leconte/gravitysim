/*

this handles rendering the scene

*/


#ifndef __RENDER_H__
#define __RENDER_H__



#include "ccgl_gl.h"

GLFWwindow *window;



struct {

    vec4_t light_pos;

    mat4_t floor_model;

    float cam_dist, cam_period, cam_pitch, cam_fov;

} scene;

int win_width, win_height;

void render_init();

void control_update();

bool render_update();

#endif

