
#include "gravitysim.h"

#include "render.h"
#include "math.h"
#include "gs_math.h"

#include "render_vals.h"


#include <stdlib.h>
#include <string.h>

int win_width = 640, win_height = 480;

int vp_width, vp_height;    


GLFWwindow *window = NULL;


// stores texture IDs
struct {

    GLuint tiled_floor;

} textures;

struct {
    GLuint tri;
} vao;

struct {
    GLuint tri;
} vbo;


struct {

    program_t basic;

} programs;


mat4_t mvp;

char * shader_path = NULL;

GLenum cerr;

#define GLCHK while ((cerr = glGetError()) != GL_NO_ERROR) { \
  printf("ERROR: %d, %s\n", cerr, gluErrorString(cerr)); \
} 


void error_callback(int error, const char* description) {
    log_error("Error[%d]: %s\n", error, description);
    exit(error);
}


GLuint load_shader(char * name, GLuint shader_type) {
    char * full_name = malloc(strlen(shader_path) + strlen(name) + 12);
    sprintf(full_name, "%s/%s", shader_path, name);

    log_debug("loading shader '%s'", full_name);

    GLuint res = glCreateShader(shader_type);

    FILE * fp = fopen(full_name, "r");
    if (fp == NULL) {
        log_error("Could not open file '%s'", full_name);
        exit(1);
    }
    fseek(fp, 0, SEEK_END);

    int fp_l = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char * from_file = malloc(fp_l + 1);
    fread(from_file, fp_l, 1, fp);
    from_file[fp_l] = 0;

    // done file io
    fclose(fp);

    glShaderSource(res, 1, (const char *const *)&from_file, NULL);
    glCompileShader(res);

    GLint result_check, log_length;
    glGetShaderiv(res, GL_COMPILE_STATUS, &result_check);
    glGetShaderiv(res, GL_INFO_LOG_LENGTH, &log_length);

    if (result_check == GL_FALSE) {
        char * error_log = malloc(log_length + 1);

        glGetShaderInfoLog(res, log_length, NULL, error_log);

        log_error("while compiling shader '%s': %s\n", name, error_log);
    
        free(error_log);
    }

    // clean up variables
    free(from_file);
    free(full_name);
    return res;
}

program_t create_program(char * v_name, char * f_name) {
    program_t res;
    res.v_shader = load_shader(v_name, GL_VERTEX_SHADER);
    res.f_shader = load_shader(f_name, GL_FRAGMENT_SHADER);
    res.program = glCreateProgram();
    
    glAttachShader(res.program, res.v_shader);
    glAttachShader(res.program, res.f_shader);
    glLinkProgram(res.program);
    GLint link_res;
    glGetProgramiv(res.program, GL_LINK_STATUS, &link_res);
    if (link_res == GL_FALSE) {
        GLint info_len;
        glGetProgramiv(res.program, GL_INFO_LOG_LENGTH, &info_len);
        char * info_res = malloc(info_len + 1);
        glGetProgramInfoLog(res.program, info_len, &info_len, info_res);
        log_error("while linking '%s'&'%s':\n'%s'", v_name, f_name, info_res);
        free(info_res);
    }
    glUseProgram(res.program);
    
    return res;
}

void render_init() {

    // initializing glfw
    if (glfwInit() != 1) {                                                  
        printf("Glfw failed to init\n");     
        exit(1);                           
    }

    glfwSetErrorCallback(error_callback);
    
    /* POSSIBLY VOLATILE */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

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


    mat4_t proj = perspective(45.0, (float)win_width / (float)win_height, 0.1f, 100.0f);

    mat4_t view = look_at(V3(0.0, 0.0, 5.0), V3(0.0, 0.0, 0.0), V3(0.0, 1.0, 0.0));

    mat4_t model = mat4_create(
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0
    );

    //model = mat4_scale(model, .1);

    view.v[3][2] = 0.0;

    dump_mat4(view);
    
    mvp = mat4_mul(mat4_mul(proj, view), model);

   // mvp = view;//mat4_mul(model, view);

    dump_mat4(mvp);

    // might need to transpose
    //dump_mat4(view);

    /* model creation */

    glGenVertexArrays(1, &vao.tri);
    glBindVertexArray(vao.tri);

    static const GLfloat g_vertex_buffer_data[] = {
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        0.0f,  1.0f, 0.0f,
    };

    glGenBuffers(1, &vbo.tri);
    glBindBuffer(GL_ARRAY_BUFFER, vbo.tri);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);


    /* load shaders */


    programs.basic = create_program("basic.v.shader", "basic.f.shader");

    GLCHK

    log_info("render init done");
}

bool render_update() {
    if (glfwWindowShouldClose(window)) {
        printf("Window closing...\n");
        return false;
    }

    /* frame init */

    glClearColor(0.2f, 0.0f, 0.8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(programs.basic.program);


    GLuint matrixID = glGetUniformLocation(programs.basic.program, "MVP");

    glUniformMatrix4fv(matrixID, 1, GL_TRUE, &(mvp.v[0][0]));

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo.tri);
    glVertexAttribPointer(
    0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
    3,                  // size
    GL_FLOAT,           // type
    GL_FALSE,           // normalized?
    0,                  // stride
    (void*)0            // array buffer offset
    );
    // Draw the triangle !
    glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
    glDisableVertexAttribArray(0);


    /* boiler plate stuff */

    glfwSwapBuffers(window);
    glfwPollEvents();

    GLCHK

    return true;
}


