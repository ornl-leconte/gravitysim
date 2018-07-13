
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

int win_width, win_height;

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


    // spherical-type particles
    struct {
        struct{
            // qualities 1-8 (8 highest)
            model_t Q_1, Q_2, Q_3, Q_4, Q_5, Q_6, Q_7;
        } ico;

        struct {
            model_t blocky, mememan;
        } special;
    } particles;

} prefabs;

// struct to sort and apply different qualities/models;
struct {

    float * qual;

    vec4_t * sorted_particles;

    GLuint vbo;

    int Q_1_len, Q_2_len, Q_3_len, Q_4_len, Q_5_len, Q_6_len, Q_7_len;
    vec4_t * Q_1, * Q_2, * Q_3, * Q_4, * Q_5, * Q_6, * Q_7;

} render_div;




// all shader programs
struct {

    GLuint basic;

    GLuint instanced;

    GLuint plane;

} shaders;


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

//GLuint vao;


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
    bool fullscreen = win_width == 0 || win_height == 0;
    if (fullscreen) {

        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

        window = glfwCreateWindow(mode->width, mode->height, "gravitysim", glfwGetPrimaryMonitor(), NULL);

    } else {
        // it's important to create a window BEFORE intiailizing glew or anything
        //glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
        window = glfwCreateWindow(640, 480, "gravitysim", NULL, NULL);
    }

    if (window == NULL) {
        printf("GLFW failed to create a window\n");
        exit(1);
    }

    glfwMakeContextCurrent(window);


    glfwSwapInterval(render_info.buffering);

    glfwGetWindowSize(window, &win_width, &win_height);
    glfwGetFramebufferSize(window, &vp_width, &vp_height);

    // for some reason we need this to be set
    glewExperimental = 1;   
    int glew_init_res = glewInit();                                            
    if (glew_init_res != GLEW_OK) {                                        
        printf("GLEW failed to init: %d\n", glew_init_res);   
        exit(1);                      
    }

    glViewport(0, 0, vp_width, vp_height);

    log_info("OpenGL version: %s", glGetString(GL_VERSION));

    log_info("OpenGL renderer: %s", glGetString(GL_RENDERER));
    
    

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    scene.cam_period = 0.0;
    scene.cam_pitch = M_PI / 4.0;
    scene.cam_dist = 250.0;
    scene.cam_fov = M_PI / 4.0;


    

    /* basic opengl settings */
    // Set the viewport to cover the new window
    glfwGetFramebufferSize(window, &vp_width, &vp_height);

	glDepthFunc(GL_LESS); 

    //wireframe mode
    //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    // might need to transpose
    //dump_mat4(view);

/*
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
*/
    /* model creation */

    glGenBuffers(1, &render_div.vbo);

    /* load objects */

    log_info("about to load models, this may take a while");


    float st = glfwGetTime(), et;


    prefabs.ico = load_obj("models/ico.obj");
    
    prefabs.cade = load_obj("models/cade.obj");
    prefabs.plane = load_obj("models/plane.obj");

    prefabs.particles.ico.Q_1 = load_obj("models/particle_ico_quality1.obj");
    prefabs.particles.ico.Q_2 = load_obj("models/particle_ico_quality2.obj");
    prefabs.particles.ico.Q_3 = load_obj("models/particle_ico_quality3.obj");
    prefabs.particles.ico.Q_4 = load_obj("models/particle_ico_quality4.obj");
    prefabs.particles.ico.Q_5 = load_obj("models/particle_ico_quality5.obj");
    prefabs.particles.ico.Q_6 = load_obj("models/particle_ico_quality6.obj");
    prefabs.particles.ico.Q_7 = load_obj("models/particle_ico_quality7.obj");


    prefabs.particles.special.blocky = load_obj("models/particle_blocky.obj");
    prefabs.particles.special.mememan = load_obj("models/particle_mememan.obj");

    et = glfwGetTime();

    log_info("done loading models, took %f ms", 1000.0 * (et - st));

    /* load shaders */


    shaders.basic = load_shader("basic.v.shader", "basic.f.shader").program;
    
    shaders.instanced = load_shader("instanced.v.shader", "instanced.f.shader").program;

    shaders.plane = load_shader("plane.v.shader", "plane.f.shader").program;


    /* for distance changing */

    render_div.qual = (float *)malloc(sizeof(float) * n_particles);
    render_div.sorted_particles = (vec4_t *)malloc(sizeof(vec4_t) * n_particles);
    render_div.Q_1 = (vec4_t *)malloc(sizeof(vec4_t) * n_particles);
    render_div.Q_2 = (vec4_t *)malloc(sizeof(vec4_t) * n_particles);
    render_div.Q_3 = (vec4_t *)malloc(sizeof(vec4_t) * n_particles);
    render_div.Q_4 = (vec4_t *)malloc(sizeof(vec4_t) * n_particles);
    render_div.Q_5 = (vec4_t *)malloc(sizeof(vec4_t) * n_particles);
    render_div.Q_6 = (vec4_t *)malloc(sizeof(vec4_t) * n_particles);
    render_div.Q_7 = (vec4_t *)malloc(sizeof(vec4_t) * n_particles);


    GLCHK

    log_info("render init done");
}


void update_transform_matrix(vec4_t camera_pos, vec4_t towards, vec4_t rot) {

    // identity
    //mat4_t model = MAT4_I;

    transformations.model = mat4_mul(MAT4_I, scaler(1.0, 1.0, 1.0));


    // apply transformations, rotations, etc on model vertexes here

    mat4_t proj = perspective(scene.cam_fov, (float)win_width / (float)win_height, 0.01f, 1000.0f);

    mat4_t view = look_at(camera_pos, towards, V4(0.0, 1.0, 0.0, 0.0));

    //mvp = mat4_mul(proj, view);
   // mvp = mat4_mul(mvp, model);
   transformations.viewproj = mat4_mul(proj, view);


    //dump_mat4(mvp);

   // mvp.v[3][3] = 1.0;

}

void render_model(model_t model) {
    glBindVertexArray(model.vao);

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
    //render_model(prefabs.ico);
}

// method to just assign all to Q4
void sort_particles_allQ2() {
    render_div.Q_1_len = 0;
    render_div.Q_2_len = n_particles;
    render_div.Q_3_len = 0;
    render_div.Q_4_len = 0;
    render_div.Q_5_len = 0;
    render_div.Q_6_len = 0;
    render_div.Q_7_len = 0;
    memcpy(render_div.Q_2, particle_data.P, sizeof(vec4_t) * n_particles);
}

int particle_comp(void * _a, void * _b) {
    vec4_t a = *(vec4_t *)_a, b = *(vec4_t *)_b;
    float dx = scene.cam_pos.x - a.x;
    float dy = scene.cam_pos.y - a.y;
    float dz = scene.cam_pos.z - a.z;

    float qual_a = (dx * dx + dy * dy + dz * dz) / a.w;

    dx = scene.cam_pos.x - b.x;
    dy = scene.cam_pos.y - b.y;
    dz = scene.cam_pos.z - b.z;

    float qual_b = (dx * dx + dy * dy + dz * dz) / b.w;
    
    if (qual_a > qual_b) {
        return 1;
    } else if (qual_a < qual_b) {
        return -1;
    } else {
        return 0;
    }
}

void sort_particles() {
    render_div.Q_1_len = 0;
    render_div.Q_2_len = 0;
    render_div.Q_3_len = 0;
    render_div.Q_4_len = 0;
    render_div.Q_5_len = 0;
    render_div.Q_6_len = 0;
    render_div.Q_7_len = 0;


    memcpy(render_div.sorted_particles, particle_data.P, n_particles * sizeof(vec4_t));

    qsort(render_div.sorted_particles, n_particles, sizeof(vec4_t), particle_comp);


    int i;
    for (i = 0; i < n_particles; ++i) {
        // use a really high quality sphere if it's so close
        // but only render 4 of these max
        vec4_t ca = render_div.sorted_particles[i];
        if (render_div.Q_7_len < 4 && n_particles <= 1024) {
            render_div.Q_7[render_div.Q_7_len++] = ca;
        } else if (render_div.Q_6_len < 10 && n_particles <= 2048) {
            render_div.Q_6[render_div.Q_6_len++] = ca;
        } else if (render_div.Q_5_len < 0.04 * n_particles && n_particles <= 4096) {
            render_div.Q_5[render_div.Q_5_len++] = ca;
        } else if (render_div.Q_4_len < 0.10 * n_particles && n_particles <= 8196) {
            render_div.Q_4[render_div.Q_4_len++] = ca;
        } else if (render_div.Q_3_len < 0.25 * n_particles && n_particles <= 16384) {
            render_div.Q_3[render_div.Q_3_len++] = ca;
        } else if (render_div.Q_3_len < 0.35 * n_particles) {
            render_div.Q_2[render_div.Q_2_len++] = ca;
        } else {
            render_div.Q_1[render_div.Q_1_len++] = ca;
        }
    }
    //printf("Q2:%lf,Q3:%lf,Q4:%lf,Q5:%lf,Q6:%lf,Q7:%lf\n", (float)render_div.Q_2_len/n_particles, (float)render_div.Q_3_len/n_particles, (float)render_div.Q_4_len/n_particles, (float)render_div.Q_5_len/n_particles, (float)render_div.Q_6_len/n_particles, (float)render_div.Q_7_len/n_particles);
}

void _render_particles_pass(model_t cur_Q, vec4_t * _parts, int _parts_len) {
    if (_parts_len < 1) return;

    glBindVertexArray(cur_Q.vao);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, cur_Q.vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, cur_Q.nbo);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, render_div.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec4_t) * _parts_len, _parts, GL_STREAM_DRAW);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glVertexAttribDivisor(2, 1);


    /* instancing */

    glDrawArraysInstanced(GL_TRIANGLES, 0, cur_Q.nbo_num, _parts_len);
}

void render_particles() {
    
    glUseProgram(shaders.instanced);

    int model_tr = glGetUniformLocation(shaders.instanced, "u_model_tr");
    int viewproj_tr = glGetUniformLocation(shaders.instanced, "u_viewproj_tr");

    int lightpos = glGetUniformLocation(shaders.instanced, "u_lightpos");

    if (!(model_tr >= 0 && viewproj_tr >= 0 && lightpos >= 0)) {
        printf("error finding transformation shader positions (%d,%d,%d)\n", model_tr, viewproj_tr, lightpos);
        exit(1);
    }

    glUniformMatrix4fv(model_tr, 1, GL_TRUE, &(transformations.model.v[0][0]));
    glUniformMatrix4fv(viewproj_tr, 1, GL_TRUE, &(transformations.viewproj.v[0][0]));
    glUniform3f(lightpos, scene.light_pos.x, scene.light_pos.y, scene.light_pos.z);


    //sort_particles_allQ2();

    // do render passes
    //_render_particles_pass(prefabs.particles.ico.Q_1, render_div.Q_1, render_div.Q_1_len);

    // optimized for close particles beinghigher quality
    if (true) {
        // multipass render
        sort_particles();
        _render_particles_pass(prefabs.particles.ico.Q_1, render_div.Q_1, render_div.Q_1_len);
        _render_particles_pass(prefabs.particles.ico.Q_2, render_div.Q_2, render_div.Q_2_len);
        _render_particles_pass(prefabs.particles.ico.Q_3, render_div.Q_3, render_div.Q_3_len);
        _render_particles_pass(prefabs.particles.ico.Q_4, render_div.Q_4, render_div.Q_4_len);
        _render_particles_pass(prefabs.particles.ico.Q_5, render_div.Q_5, render_div.Q_5_len);
        _render_particles_pass(prefabs.particles.ico.Q_6, render_div.Q_6, render_div.Q_6_len);
        _render_particles_pass(prefabs.particles.ico.Q_7, render_div.Q_7, render_div.Q_7_len);
    } else {
        _render_particles_pass(prefabs.particles.ico.Q_1, particle_data.P, n_particles);
        //_render_particles_pass(prefabs.particles.special.blocky, particle_data.P, n_particles);
        //_render_particles_pass(prefabs.particles.special.mememan, particle_data.P, n_particles);
    }

}

bool render_update() {
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
    vec4_t center_pos = V4(0.0, 0.0, 0.0, 0.0);
    //vec4_t center_pos = particle_data.P[0];
    
    center_pos.w = 0.0;

    scene.cam_pos = camera_orbit(center_pos, scene.cam_dist, scene.cam_period, scene.cam_pitch);
    update_transform_matrix(scene.cam_pos, center_pos, V4(0.0, 0.0, 0.0, 0.0));

    

    scene.light_pos = scene.cam_pos;

   // scene.light_pos = V3(0.0, 0.0, 0.0);


    glClearColor(0.2f, 0.0f, 0.8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();


    /* draw the stuff */

    

    _floor_render();
    //_basic_render();
    //_inst_render();
    
    render_particles();

    
    /* boiler plate stuff */

    glfwSwapBuffers(window);
    glfwPollEvents();

    GLCHK

    return true;
}


