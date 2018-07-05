/*

this handles rendering the scene

*/


#ifndef __RENDER_H__
#define __RENDER_H__


#include <GL/glew.h> 
#include <GLFW/glfw3.h>

GLFWwindow *window;


void render_init();

bool render_update();

#endif

