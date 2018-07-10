
/*

ccgl_gl.h - OpenGL specific ccgl functionality

*/


#ifndef __CCGL_GL_H__
#define __CCGL_GL_H__

#include "ccgl.h"

#include <GL/glew.h> 
#include <GLFW/glfw3.h>


typedef struct _shader_t {

    GLuint f_shader, v_shader;

    GLuint program;

} shader_t;


typedef struct _model_t {

    int vbo, nbo;

    int vbo_num, nbo_num;

} model_t;


model_t load_obj(char * obj_path, float scale);


static struct {
    int len;
    char ** vals;

    int _max_val_len;
} shader_search;


void add_shader_path(char * path);

shader_t load_shader(char * v_name, char * f_name);


#endif
