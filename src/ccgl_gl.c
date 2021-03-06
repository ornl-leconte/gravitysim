#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "gs_math.h"

#include "ccgl_gl.h"

#include "log.h"



char * _read_file(char * file_name) {

    FILE * fp = fopen(file_name, "r");
    if (fp == NULL) {
        return NULL;
    }
    fseek(fp, 0, SEEK_END);

    int fp_l = ftell(fp);
    //rewind(fp);

    fseek(fp, 0, SEEK_SET);

    char * from_file = (char *)malloc(fp_l + 1);
    fread(from_file, fp_l, 1, fp);
    
    fclose(fp);
    
    from_file[fp_l] = 0;
    return from_file;

}

shader_t load_shader(char * v_name, char * f_name) {
    shader_t res;

    char * cv_name = malloc(strlen(v_name) + strlen(shared_data_dir) + 256), 
         * cf_name = malloc(strlen(f_name) + strlen(shared_data_dir) + 256);
    
    bool v_found = false, f_found = false;


    char *cv_source = NULL, *cf_source = NULL;
    sprintf(cv_name, "%s/src/shaders/%s", shared_data_dir, v_name);
    log_debug("trying shader %s", cv_name);
    if ((cv_source = _read_file(cv_name)) != NULL) {

        GLuint vr = glCreateShader(GL_VERTEX_SHADER);

        log_trace("shader '%s' source:\n%s", cv_name, cv_source);


        glShaderSource(vr, 1, (const char *const *)&cv_source, NULL);
        glCompileShader(vr);

        GLint result_check, log_length;
        glGetShaderiv(vr, GL_COMPILE_STATUS, &result_check);
        glGetShaderiv(vr, GL_INFO_LOG_LENGTH, &log_length);

        if (result_check == GL_FALSE) {
            char * error_log = malloc(log_length + 1);

            glGetShaderInfoLog(vr, log_length, NULL, error_log);

            log_error("while compiling shader '%s': \n%s\n", cv_name, error_log);
        
            exit(1);
        }
        v_found = true;
        res.v_shader = vr;
        free(cv_source);
    }
    sprintf(cf_name, "%s/src/shaders/%s", shared_data_dir, f_name);
    log_debug("trying shader %s", cf_name);
    if ((cf_source = _read_file(cf_name)) != NULL) {

        GLuint fr = glCreateShader(GL_FRAGMENT_SHADER);

        log_trace("shader '%s' source:\n%s", cf_name, cf_source);

        glShaderSource(fr, 1, (const char *const *)&cf_source, NULL);
        glCompileShader(fr);

        GLint result_check, log_length;
        glGetShaderiv(fr, GL_COMPILE_STATUS, &result_check);
        glGetShaderiv(fr, GL_INFO_LOG_LENGTH, &log_length);

        if (result_check == GL_FALSE) {
            char * error_log = malloc(log_length + 1);

            glGetShaderInfoLog(fr, log_length, NULL, error_log);

            log_error("while compiling shader '%s': \n%s\n", cf_name, error_log);
        
            exit(1);
        }
        f_found = true;
        res.f_shader = fr;
        free(cf_source);
    }

    if (v_found && f_found) {
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
        //glUseProgram(res.program);

    } else {
        log_error("couldn't find vertex and fragment shader");
        exit(1);
    }


    free(cv_name);
    free(cf_name);
    return res;
}



// triplet of integers
typedef struct _i3_t {
    int x, y, z;

} i3_t;

typedef struct vec3_t {
    float x, y, z;
} vec3_t;

model_t load_obj(char * _obj_path) {
    char * obj_path = malloc(strlen(shared_data_dir) + strlen(_obj_path) + 256);
    sprintf(obj_path, "%s/%s", shared_data_dir, _obj_path);


    model_t res;


    int read_verts_num = 0;
    vec3_t * read_verts = NULL;

    int read_normals_num = 0;
    vec3_t * read_normals = NULL;


    int f_num = 0;
    i3_t * f_verts = NULL;
    i3_t * f_normals = NULL;
    i3_t * f_uvs = NULL;

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
            fscanf(fp, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
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
            i3_t fv, fuv, fn;
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

            f_verts = (i3_t *)realloc(f_verts, sizeof(i3_t) * f_num);
            f_normals = (i3_t *)realloc(f_normals, sizeof(i3_t) * f_num);
            f_uvs = (i3_t *)realloc(f_uvs, sizeof(i3_t) * f_num);

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
        out_verts[3 * i + 0] = read_verts[f_verts[i].x-1];
        out_verts[3 * i + 1] = read_verts[f_verts[i].y-1];
        out_verts[3 * i + 2] = read_verts[f_verts[i].z-1];

        out_normals[3 * i + 0] = read_normals[f_normals[i].x-1];
        out_normals[3 * i + 1] = read_normals[f_normals[i].y-1];
        out_normals[3 * i + 2] = read_normals[f_normals[i].z-1];
    }

    res.vbo_num = f_num * 3;
    res.nbo_num = f_num * 3;

    //res.vbo_num = read_verts_num * 3;
    //res.nbo_num = read_normals_num * 3;

    glGenVertexArrays(1, &res.vao);
    glBindVertexArray(res.vao);

    glGenBuffers(1, &res.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, res.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec3_t) * res.vbo_num, out_verts, GL_STATIC_DRAW);

    glGenBuffers(1, &res.nbo);
    glBindBuffer(GL_ARRAY_BUFFER, res.nbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec3_t) * res.nbo_num, out_normals, GL_STATIC_DRAW);

    if (res.nbo_num != res.vbo_num) {
        printf("while loading model '%s', vbo_num and nbo_num are different!\n", obj_path);
        exit(1);
    }

    free(read_verts);
    free(read_normals);
    free(f_verts);
    free(f_uvs);
    free(f_normals);

    free(out_verts);
    free(out_normals);

    free(obj_path);

    return res;

}
