
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

    model_t ico, cade, plane;

} prefabs;



GLuint vao;


// all shader programs
struct {

    GLuint basic;

    GLuint instanced;

    GLuint plane;

} shaders;


// misc buffers
struct {

    GLuint inst_pos;
    GLuint inst_mass;

} mbufs;


// updated every loop to use as matrix operations to transform
struct {

    mat4_t model;

    mat4_t viewproj;

} transformations;


struct {

    vec3_t light_pos;

    mat4_t floor_model;

    float cam_dist, cam_period, cam_pitch, cam_fov;

} scene;


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
bool * is_pressed_state;

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

    scene.cam_period = 0.0;
    scene.cam_pitch = M_PI / 4.0;
    scene.cam_dist = 100.0;
    scene.cam_fov = M_PI / 4.0;
    

    

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


    glGenBuffers(1, &mbufs.inst_mass);
    glBindBuffer(GL_ARRAY_BUFFER, mbufs.inst_mass);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * MAX_NUM_PARTICLES, NULL, GL_STREAM_DRAW);



    /* load objects */

    prefabs.ico = load_obj("../models/ico.obj", 1.0);
    prefabs.cade = load_obj("../models/cade.obj", 1.0);
    prefabs.plane = load_obj("../models/plane.obj", 1.0);

    /* load shaders */


    add_shader_path("../src");

    shaders.basic = load_shader("basic.v.shader", "basic.f.shader").program;
    shaders.instanced = load_shader("instanced.v.shader", "instanced.f.shader").program;
    shaders.plane = load_shader("plane.v.shader", "plane.f.shader").program;

    is_pressed_state = malloc(sizeof(bool) *  (GLFW_KEY_LAST + 1));

    int i;
    for (i = 0; i < GLFW_KEY_LAST + 1; ++i) {
        is_pressed_state[i] = GLFW_RELEASE;
    }


    GLCHK


    log_info("render init done");
}


void update_transform_matrix(vec3_t camera_pos, vec3_t towards, vec3_t rot) {

    // identity
    //mat4_t model = MAT4_I;

    transformations.model = mat4_mul(MAT4_I, scaler(1.0, 1.0, 1.0));


    // apply transformations, rotations, etc on model vertexes here

    mat4_t proj = perspective(scene.cam_fov, (float)win_width / (float)win_height, 0.01f, 1000.0f);

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

void _floor_render() {

    glUseProgram(shaders.plane);

    int model_tr = glGetUniformLocation(shaders.plane, "uni_model_tr");
    int viewproj_tr = glGetUniformLocation(shaders.plane, "uni_viewproj_tr");

    if (!(model_tr >= 0 && viewproj_tr >= 0)) {
        printf("error finding transformation shader positions\n");
        exit(1);
    }

    glUniformMatrix4fv(model_tr, 1, GL_TRUE, &(scene.floor_model.v[0][0]));
    glUniformMatrix4fv(viewproj_tr, 1, GL_TRUE, &(transformations.viewproj.v[0][0]));

    render_model(prefabs.plane);

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
    glUniform3f(lightpos, scene.light_pos.x, scene.light_pos.y, scene.light_pos.z);


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
    int offset_mass = glGetUniformLocation(shaders.instanced, "uni_particle_mass");

    if (!(model_tr >= 0 && viewproj_tr >= 0 && lightpos >= 0 && offset_array >= 0 && offset_mass >= 0)) {
        printf("error finding transformation shader positions\n");
        exit(1);
    }


    glUniformMatrix4fv(model_tr, 1, GL_TRUE, &(transformations.model.v[0][0]));
    glUniformMatrix4fv(viewproj_tr, 1, GL_TRUE, &(transformations.viewproj.v[0][0]));
    glUniform3f(lightpos, scene.light_pos.x, scene.light_pos.y, scene.light_pos.z);

    vec3_t * enabled_poss = malloc(sizeof(vec3_t) * n_particles);
    float * enabled_mass = malloc(sizeof(float) * n_particles);

    int i;
    int c = 0;
    for (i = 0; i < n_particles; ++i) {
        if (particle_data.is_enabled[i]) {
            enabled_poss[c] = particle_data.positions[i];
            enabled_mass[c] = particle_data.masses[i];
            c++;
        }
    }
    

    glUniform3fv(offset_array, c, enabled_poss);
    glUniform1fv(offset_mass,c , enabled_mass);

    model_t model = prefabs.ico;

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, model.vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, model.nbo);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glDrawArraysInstanced(GL_TRIANGLES, 0, model.vbo_num, c);
    //log_trace("num instances: %d", c);

}


bool pressed_once(int k) {
    bool res = (glfwGetKey(window, k) == GLFW_PRESS) && (is_pressed_state[k] != GLFW_PRESS);
    is_pressed_state[k] = glfwGetKey(window, k);
    return res;
}

bool is_pressed(int k) {
    is_pressed_state[k] = glfwGetKey(window, k);
    return glfwGetKey(window, k) == GLFW_PRESS;
}


void control_update() {
    // will be 1.0 at perfect 60.0 fps
    float cm = GS_looptime * 60.0;
    glfwSetInputMode(window, GLFW_STICKY_KEYS, 1);

    // do update for all sorts of controllers 
    if (is_pressed(GLFW_KEY_D)) scene.cam_period += (cm / 60.0) * (0.666 * M_PI);
    if (is_pressed(GLFW_KEY_A)) scene.cam_period -= (cm / 60.0) * (0.666 * M_PI);
    if (is_pressed(GLFW_KEY_W)) scene.cam_pitch += (cm / 60.0) * (0.666 * M_PI);
    if (is_pressed(GLFW_KEY_S)) scene.cam_pitch -= (cm / 60.0) * (0.666 * M_PI);
    if (is_pressed(GLFW_KEY_SPACE)) scene.cam_dist /= expf(0.5 * (cm / 60.0));
    if (is_pressed(GLFW_KEY_LEFT_SHIFT) || is_pressed(GLFW_KEY_RIGHT_SHIFT)) scene.cam_dist *= expf(0.5 * (cm / 60.0));

    if (is_pressed(GLFW_KEY_PERIOD)) scene.cam_fov += M_PI * 0.15 * (cm / 60.0);
    if (is_pressed(GLFW_KEY_COMMA)) scene.cam_fov -= M_PI * 0.15 * (cm / 60.0);

    if (pressed_once(GLFW_KEY_P)) sim_data.is_paused = !sim_data.is_paused;


    if (scene.cam_pitch > M_PI / 2) scene.cam_pitch = M_PI / 2 - 0.01;
    if (scene.cam_pitch < - M_PI / 2) scene.cam_pitch = - M_PI / 2 + 0.01;

    if (scene.cam_fov > M_PI) scene.cam_fov = M_PI - 0.01;
    if (scene.cam_fov < 0.0) scene.cam_fov = 0.01;


}


bool render_update() {

    double st = glfwGetTime();

    if (glfwWindowShouldClose(window)) {
        printf("Window closing...\n");
        return false;
    }

    /* frame init */

    // account for changes
    glfwGetWindowSize(window, &win_width, &win_height);
    glfwGetFramebufferSize(window, &vp_width, &vp_height);
    glViewport(0, 0, vp_width, vp_height);

    // will probably need to smooth this
    //float dist = 50.0 + 120.0 * powf((physics_data.std_pos.x + physics_data.std_pos.y + physics_data.std_pos.z) / 3.0, 0.4);


    scene.floor_model = mat4_mul(translator(0.0, -100.0, 0.0), scaler(100.0, 100.0, 100.0));
    vec3_t center_pos = V3(0.0, -10, 0.0);

    vec3_t camera_pos = camera_orbit(center_pos, scene.cam_dist, scene.cam_period, scene.cam_pitch);
    update_transform_matrix(camera_pos, center_pos, V3(0.0, 0.0, 0.0));

    scene.light_pos = camera_pos;


    glClearColor(0.2f, 0.0f, 0.8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();


    /* draw the stuff */

    _floor_render();
    //_basic_render();
    _inst_render();

    
    /* boiler plate stuff */

    glfwSwapBuffers(window);
    glfwPollEvents();

    GLCHK

    return true;
}


