
#include "gs_store.h"

#include "gravitysim.h"


FILE * fp_write = NULL, * fp_read = NULL;
int n_written, n_read;

void gs_store_write_init(char * file_path) {
    fp_write = fopen(file_path, "w");
    if (fp_write == NULL) {
        log_error("ERROR opening storage file: '%s'", file_path);
        exit(1);
    }
    n_written = 0;
}

void _gs_write_float(float x) {

    fwrite(&x, sizeof(x), 1, fp_write);
}

float _gs_read_float() {
    float res;
    fread(&res, sizeof(res), 1, fp_read);
    return res;
}

void gs_store_write_frame() {
    // write metadata
    fprintf(fp_write, "%d:", GS.N);

    //fwrite(GS.P, sizeof(vec4_t), GS.N, fp_write);

    int i;
    for (i = 0; i < GS.N; ++i) {
        _gs_write_float(GS.P[i].x);
        _gs_write_float(GS.P[i].y);
        _gs_write_float(GS.P[i].z);
        _gs_write_float(GS.P[i].w);

        _gs_write_float(GS.V[i].x);
        _gs_write_float(GS.V[i].y);
        _gs_write_float(GS.V[i].z);

        _gs_write_float(GS.C[i].x);
        _gs_write_float(GS.C[i].y);
        _gs_write_float(GS.C[i].z);
    }
    //fprintf(fp_write, "\n");

    n_written++;
}


void gs_store_write_end() {
    if (fp_write != NULL) fclose(fp_write);
}

void gs_store_read_init(char * file_path) {
    fp_read = fopen(file_path, "r");
    if (fp_read == NULL) {
        log_error("ERROR opening storage file: '%s'", file_path);
        exit(1);
    }

    n_read = 0;
}


bool gs_store_read_frame() {
    
    fscanf(fp_read, "%d:", &GS.N);
    //fread(GS.P, sizeof(vec4_t), GS.N, fp_read);
    update_N(GS.N);
    
    int i;
    for (i = 0; i < GS.N; ++i) {
        GS.P[i].x = _gs_read_float();
        GS.P[i].y = _gs_read_float();
        GS.P[i].z = _gs_read_float();
        GS.P[i].w = _gs_read_float();

        GS.V[i].x = _gs_read_float();
        GS.V[i].y = _gs_read_float();
        GS.V[i].z = _gs_read_float();

        GS.C[i].x = _gs_read_float();
        GS.C[i].y = _gs_read_float();
        GS.C[i].z = _gs_read_float();
    }

    //fscanf(fp_read, "\n");


    if (feof(fp_read)) {
        log_info("done with animation");
        return false;
    }

    n_read++;
    return true;
}


void gs_store_read_end() {
    if (fp_read != NULL) fclose(fp_read);
}
