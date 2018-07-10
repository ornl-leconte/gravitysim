
#include "gravitysim.h"


#include "ccgl.h"
// use opengl specific stuff
#include "ccgl_gl.h"


#include "render.h"
#include "gs_math.h"


#include "render_vals.h"


#include <stdlib.h>
#include <string.h>
#include <math.h>

int win_width = 640, win_height = 480;

int vp_width, vp_height;    


GLFWwindow *window = NULL;

int n_frames = 0;


// stores texture IDs
struct {

    GLuint tiled_floor;

} textures;


// to store our different instanced stuff
struct {

    model_t ico, cade;

} prefabs;



GLuint vao;


// all shader programs
struct {

    GLuint basic;

    GLuint instanced;

} shaders;


// misc buffers
struct {

    GLuint inst_pos;

} mbufs;


// updated every loop to use as matrix operations to transform
struct {

    mat4_t model;

    mat4_t viewproj;

} transformations;


// error handling
GLenum cerr;

#define GLCHK while ((cerr = glGetError()) != GL_NO_ERROR) { \
  error_callback(cerr, gluErrorString(cerr)); \
} 

void error_callback(int error, const char* description) {
    if (error == 1280 || error == 1281 || error == 1282) {
        // safe to ignore, generally
    } else {
        log_error("Error[%d]: %s", error, description);
        exit(error);
    }
}

// initialization

void render_init() {

    // initializing glfw
    if (glfwInit() != 1) {                                                  
        printf("Glfw failed to init\n");     
        exit(1);                           
    }

    glfwSetErrorCallback(error_callback);
    
    /* POSSIBLY VOLATILE, only for macbooks */
#ifdef __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

#endif

    // it's important to create a window BEFORE intiailizing glew or anything
    window = glfwCreateWindow(win_width, win_height, "gravitysim", NULL, NULL);

    glfwMakeContextCurrent(window);
    


    if (window == NULL) {
        printf("GLFW failed to create a window\n");
        exit(1);
    }


    // for some reason we need this to be set
    glewExperimental = 1;                                               
    if (glewInit() != GLEW_OK) {                                        
        printf("GLEW failed to init\n");   
        exit(1);                      
    }


    log_info("OpenGL version: %s", glGetString(GL_VERSION));

    log_info("OpenGL renderer: %s", glGetString(GL_RENDERER));
    
    

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    

    

    /* basic opengl settings */
    // Set the viewport to cover the new window
    glfwGetFramebufferSize(window, &vp_width, &vp_height);

	glDepthFunc(GL_LESS); 

    //wireframe mode
    //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    // might need to transpose
    //dump_mat4(view);



    /* model creation */

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &mbufs.inst_pos);
    glBindBuffer(GL_ARRAY_BUFFER, mbufs.inst_pos);
    glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float) * MAX_NUM_PARTICLES, NULL, GL_STREAM_DRAW);


    /* load objects */

    prefabs.ico = load_obj("../models/ico.obj", 0.1);
    prefabs.cade = load_obj("../models/cade.obj", 1.0);


    /* load shaders */


    add_shader_path("../src");

    shaders.basic = load_shader("basic.v.shader", "basic.f.shader").program;
    shaders.instanced = load_shader("instanced.v.shader", "instanced.f.shader").program;


    GLCHK


    log_info("render init done");
}


void update_transform_matrix(vec3_t camera_pos, vec3_t towards, vec3_t rot) {

    // identity
    //mat4_t model = MAT4_I;

    transformations.model = mat4_mul(MAT4_I, scaler(0.1, 0.1, 0.1));


    // apply transformations, rotations, etc on model vertexes here

    mat4_t proj = perspective(M_PI / 4.0, (float)win_width / (float)win_height, 0.1f, 100.0f);

    mat4_t view = look_at(camera_pos, towards, V3(0.0, 1.0, 0.0));

    //mvp = mat4_mul(proj, view);
   // mvp = mat4_mul(mvp, model);
   transformations.viewproj = mat4_mul(proj, view);


    //dump_mat4(mvp);

   // mvp.v[3][3] = 1.0;

}

void render_model(model_t model) {
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, model.vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, model.nbo);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // Draw the triangle !
    glDrawArrays(GL_TRIANGLES, 0, model.vbo_num); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

void _basic_render() {

    glUseProgram(shaders.basic);

    int model_tr = glGetUniformLocation(shaders.basic, "uni_model_tr");
    int viewproj_tr = glGetUniformLocation(shaders.basic, "uni_viewproj_tr");

    int lightpos = glGetUniformLocation(shaders.basic, "uni_lightpos");

    if (!(model_tr >= 0 && viewproj_tr >= 0 && lightpos >= 0)) {
        printf("error finding transformation shader positions\n");
        exit(1);
    }

    glUniformMatrix4fv(model_tr, 1, GL_TRUE, &(transformations.model.v[0][0]));
    glUniformMatrix4fv(viewproj_tr, 1, GL_TRUE, &(transformations.viewproj.v[0][0]));
    glUniform3f(lightpos, sinf(glfwGetTime() * M_PI) * 3, 2.0, 2.0);


    /* render models */
    render_model(prefabs.cade);
    render_model(prefabs.ico);
}


void _inst_render() {

    glUseProgram(shaders.instanced);

    int model_tr = glGetUniformLocation(shaders.instanced, "uni_model_tr");
    int viewproj_tr = glGetUniformLocation(shaders.instanced, "uni_viewproj_tr");

    int lightpos = glGetUniformLocation(shaders.instanced, "uni_lightpos");
    int offset_array = glGetUniformLocation(shaders.instanced, "uni_particle_offests");

    if (!(model_tr >= 0 && viewproj_tr >= 0 && lightpos >= 0 && offset_array >= 0)) {
        printf("error finding transformation shader positions\n");
        exit(1);
    }

    glUniformMatrix4fv(model_tr, 1, GL_TRUE, &(transformations.model.v[0][0]));
    glUniformMatrix4fv(viewproj_tr, 1, GL_TRUE, &(transformations.viewproj.v[0][0]));
    glUniform3f(lightpos, sinf(glfwGetTime() * M_PI) * 3, 2.0, 2.0);

    glUniform3fv(offset_array, n_particles, particle_data.positions);

    model_t model = prefabs.ico;

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, model.vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, model.nbo);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glDrawArraysInstanced(GL_TRIANGLES, 0, model.vbo_num, n_particles);


}



bool render_update() {

    double st = glfwGetTime(), et;

    if (glfwWindowShouldClose(window)) {
        printf("Window closing...\n");
        return false;
    }

    /* frame init */

    // account for changes
    glfwGetWindowSize(window, &win_width, &win_height);
    glfwGetFramebufferSize(window, &vp_width, &vp_height);
    glViewport(0, 0, vp_width, vp_height);


    // period
    float cper = M_PI * glfwGetTime() / 4.0;
    float cf = .25;

    vec3_t center_pos = V3(-1.0, 1.0, 1.0);

    vec3_t camera_pos = vec3_add(center_pos, V3(sinf(cper) * cf, 0.0, cosf(cper) * cf));


    //sinf(cper) * cf, 0.25, cosf(cper) * cf
    update_transform_matrix(camera_pos, V3(0.0, 0.0, 0.0), V3(0.0, 0.0, 0.0));

    glClearColor(0.2f, 0.0f, 0.8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();


    /* draw the stuff */

    //_basic_render();
    _inst_render();

    
    /* boiler plate stuff */

    glfwSwapBuffers(window);
    glfwPollEvents();

    et = glfwGetTime();

   GLCHK


    log_trace("frame %d done, time %f ms", n_frames++, (et-st)/1000.0);

    return true;
}


