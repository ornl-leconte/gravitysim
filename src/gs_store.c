
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

void gs_store_write_frame() {
    if (n_written == 0) {
        // write first metadata
        fwrite(&n_particles, sizeof(n_particles), 1, fp_write);
    }

    fwrite(particle_data.P, sizeof(vec4_t), n_particles, fp_write);

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
    
    // read header library
    fread(&n_particles, sizeof(n_particles), 1, fp_read);
    
    n_read = 0;
}


void gs_store_read_frame() {
    fread(particle_data.P, sizeof(vec4_t), n_particles, fp_read);

    if (feof(fp_read)) log_info("done with animation");

    n_read++;
}


void gs_store_read_end() {
    if (fp_read != NULL) fclose(fp_read);
}
