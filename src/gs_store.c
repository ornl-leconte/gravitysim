
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
    // write metadata
    fwrite(&GS.N, sizeof(GS.N), 1, fp_write);

    fwrite(GS.P, sizeof(vec4_t), GS.N, fp_write);

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


void gs_store_read_frame() {
    fread(GS.N, sizeof(GS.N), 1, fp_read);
    update_N(GS.N);
    fread(GS.P, sizeof(vec4_t), GS.N, fp_read);

    if (feof(fp_read)) log_info("done with animation");

    n_read++;
}


void gs_store_read_end() {
    if (fp_read != NULL) fclose(fp_read);
}
