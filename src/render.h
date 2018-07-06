/*

this handles rendering the scene

*/


#ifndef __RENDER_H__
#define __RENDER_H__


#include <GL/glew.h> 
#include <GLFW/glfw3.h>


typedef struct program_t {

    GLuint f_shader, v_shader;

    GLuint program;

} program_t;

GLFWwindow *window;

char * shader_path;

void render_init();

bool render_update();

#endif

