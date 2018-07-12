
/*

ccgl_gl.h - OpenGL specific ccgl functionality

*/


#ifndef __CCGL_GL_H__
#define __CCGL_GL_H__

#include "ccgl.h"

#include <GL/glew.h> 
#include <GLFW/glfw3.h>
#include <string.h>

typedef struct _shader_t {

    GLuint f_shader, v_shader;

    GLuint program;

} shader_t;


typedef struct _model_t {

    GLuint vbo, vao, nbo;

    int vbo_num, nbo_num;

} model_t;


model_t load_obj(char * obj_path);


shader_t load_shader(char * v_name, char * f_name);

char * _read_file(char * file_name);

#endif
