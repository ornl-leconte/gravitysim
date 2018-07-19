
/*

code for reading and writing simulation data to file.

So this can be used to recompute stuff

*/


#ifndef __GS_STORE_H__
#define __GS_STORE_H__

#include <stdbool.h>


void gs_store_write_init(char * file_path);

void gs_store_write_frame();

void gs_store_write_end();



void gs_store_read_init(char * file_path);

bool gs_store_read_frame();

void gs_store_read_end();


#endif
