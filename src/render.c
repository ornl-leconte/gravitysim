
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

int n_frames = 0;


// stores texture IDs
struct {

    GLuint tiled_floor;

} textures;

typedef struct _model_t {

    int vbo, nbo;

    int vbo_num, nbo_num;

} model_t;


struct {

    model_t ico, cade;

} prefabs;


struct {
    GLuint tri;

    int tri_num;

} vbo;

struct {
    GLuint tri;
} vao;

// normals
struct {
    GLuint tri;

    int tri_num;
    
} nbo;


struct {

    program_t basic;

} programs;


struct {

    mat4_t model;

    mat4_t viewproj;

} transformations;


char * shader_path = NULL;

GLenum cerr;

#define GLCHK while ((cerr = glGetError()) != GL_NO_ERROR) { \
  error_callback(cerr, gluErrorString(cerr)); \
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

        log_error("while compiling shader '%s': \n%s\n", name, error_log);
    
        exit(1);

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
        log_error("while linking '%s'&'%s':\n%s", v_name, f_name, info_res);

        exit(1);
        free(info_res);
    }
    glUseProgram(res.program);
    
    return res;
}


model_t load_obj(char * obj_path) {

    model_t res;


    int read_verts_num = 0;
    vec3_t * read_verts = NULL;

    int read_normals_num = 0;
    vec3_t * read_normals = NULL;


    int f_num = 0;
    vec3i_t * f_verts = NULL;
    vec3i_t * f_normals = NULL;
    vec3i_t * f_uvs = NULL;

    FILE * fp = fopen(obj_path, "r");

    if (fp == NULL) {
        printf("failed to open file: '%s'\n", obj_path);
        exit(1);
    }

    while (true) {
        char lineHeader[256];
        // read the first word of the line
        int r = fscanf(fp, "%s", lineHeader);
        if (r == EOF) break;
                
        if (strcmp(lineHeader, "v") == 0) {
            vec3_t vertex;
            fscanf(fp, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );
            read_verts_num++;
            read_verts = (vec3_t *)realloc(read_verts, sizeof(vec3_t) * read_verts_num);
            read_verts[read_verts_num - 1] = vertex;
        } else if ( strcmp( lineHeader, "vn" ) == 0 ){
            vec3_t normal;
            fscanf(fp, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
            read_normals_num++;
            read_normals = (vec3_t *)realloc(read_normals, sizeof(vec3_t) * read_normals_num);
            read_normals[read_normals_num - 1] = normal;
        } else if (strcmp(lineHeader, "f") == 0) {
            vec3i_t fv, fuv, fn;
            char v_s[256], uv_s[256], n_s[256];
            int matches = fscanf(fp, "%s %s %s\n", v_s, uv_s, n_s);
            if (matches != 3) {
                printf("LOADING MODEL '%s' FAILED IN PARSER\n", obj_path);
                exit(1);
            }

            char combined[4*256];
            sprintf(combined, "%s %s %s", v_s, uv_s, n_s);
            
            matches = sscanf(combined, "%d/%d/%d %d/%d/%d %d/%d/%d\n", 
                &fv.x, &fuv.x, &fn.x,
                &fv.y, &fuv.y, &fn.y,
                &fv.z, &fuv.z, &fn.z
            );

            if (matches != 9) {
                matches = sscanf(combined, "%d//%d %d//%d %d//%d\n", 
                    &fv.x, &fn.x,
                    &fv.y, &fn.y,
                    &fv.z, &fn.z
                );
                fuv.x = 0;
                fuv.y = 0;
                fuv.z = 0;
                if (matches != 6) {
                    printf("ERROR PARSING FORMAT\n");
                    exit(1);
                }
            }

            f_num++;

            f_verts = (vec3i_t *)realloc(f_verts, sizeof(vec3i_t) * f_num);
            f_normals = (vec3i_t *)realloc(f_normals, sizeof(vec3i_t) * f_num);
            f_uvs = (vec3i_t *)realloc(f_uvs, sizeof(vec3i_t) * f_num);

            f_verts[f_num - 1] = fv;
            f_uvs[f_num - 1] = fuv;
            f_normals[f_num - 1] = fn;
        }

    }

    fclose(fp);

    vec3_t * out_verts = (vec3_t *)malloc(sizeof(vec3_t) * 3 * f_num);
    vec3_t * out_normals = (vec3_t *)malloc(sizeof(vec3_t) * 3 * f_num);

    int i;
    for (i = 0; i < f_num; i++) {
        vec3_t cv_0 = read_verts[f_verts[i].x-1];
        vec3_t cv_1 = read_verts[f_verts[i].y-1];
        vec3_t cv_2 = read_verts[f_verts[i].z-1];

        out_verts[3 * i + 0] = cv_0;
        out_verts[3 * i + 1] = cv_1;
        out_verts[3 * i + 2] = cv_2;

        vec3_t cn_0 = read_normals[f_normals[i].x-1];
        vec3_t cn_1 = read_normals[f_normals[i].y-1];
        vec3_t cn_2 = read_normals[f_normals[i].z-1];

        out_normals[3 * i + 0] = cn_0;
        out_normals[3 * i + 1] = cn_1;
        out_normals[3 * i + 2] = cn_2;
    }

    res.vbo_num = f_num * 9;
    res.nbo_num = f_num * 9;

    //res.vbo_num = read_verts_num * 3;
    //res.nbo_num = read_normals_num * 3;

    glGenBuffers(1, &res.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, res.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * res.vbo_num, out_verts, GL_STATIC_DRAW);

    glGenBuffers(1, &res.nbo);
    glBindBuffer(GL_ARRAY_BUFFER, res.nbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * res.nbo_num, out_normals, GL_STATIC_DRAW);


    free(read_verts);
    free(read_normals);
    free(f_verts);
    free(f_uvs);
    free(f_normals);

    free(out_verts);
    free(out_normals);

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

    //wireframe mode
    //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    // might need to transpose
    //dump_mat4(view);

    /* model creation */

    glGenVertexArrays(1, &vao.tri);
    glBindVertexArray(vao.tri);

    /* load objects */

    prefabs.ico = load_obj("../models/ico.obj");
    prefabs.cade = load_obj("../models/cade.obj");


    /* load shaders */



    programs.basic = create_program("basic.v.shader", "basic.f.shader");

    GLCHK

    log_info("render init done");
}


void update_transform_matrix(vec3_t camera_pos, vec3_t towards, vec3_t rot) {

    // identity
    //mat4_t model = MAT4_I;

    transformations.model = MAT4_I;

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

bool render_update() {
    if (glfwWindowShouldClose(window)) {
        printf("Window closing...\n");
        return false;
    }


    /* frame init */

    glfwGetWindowSize(window, &win_width, &win_height);
    glfwGetFramebufferSize(window, &vp_width, &vp_height);


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


    glUseProgram(programs.basic.program);

    int model_tr = glGetUniformLocation(programs.basic.program, "uni_model_tr");
    int viewproj_tr = glGetUniformLocation(programs.basic.program, "uni_viewproj_tr");

    int lightpos = glGetUniformLocation(programs.basic.program, "uni_lightpos");

    if (!(model_tr >= 0 && viewproj_tr >= 0 && lightpos >= 0)) {
        printf("error finding transformation shader positions\n");
        exit(1);
    }

    glUniformMatrix4fv(model_tr, 1, GL_TRUE, &(transformations.model.v[0][0]));
    glUniformMatrix4fv(viewproj_tr, 1, GL_TRUE, &(transformations.viewproj.v[0][0]));
    glUniform3f(lightpos, -2.0, 2.0, 2.0);


    /* render models */
    //render_model(prefabs.cade);
    render_model(prefabs.ico);

    
    /* boiler plate stuff */

    glfwSwapBuffers(window);
    glfwPollEvents();


    log_trace("frame %d done", n_frames++);

    return true;
}


