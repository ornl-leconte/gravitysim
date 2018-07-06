

#include "gravitysim.h"

#include "gs_math.h"
#include "render.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

float gravity_coef = 9.81;

// global particles array
int n_particles = 0;
total_particle_state_t * particles = NULL;


int main(int argc, char ** argv) {

    log_set_level(LOG_INFO);

    char c;

    shader_path = "../src";

    while ((c = getopt(argc, argv, "n:v:S:h")) != (char)(-1)) switch (c) {
        case 'h':
            printf("Usage: gravitysim [options]\n");
            printf("\n");
            printf("  -h                prints this help\n");
            printf("  -v [N]            sets verbosity (5=EVERYTHING, 1=ERRORS ONLY)\n");
            printf("  -S [dir]          shader directory\n");
            printf("\n");
            return 0;
            break;
        case 'v':
            log_set_level(atoi(optarg));
            break;
        case 'S':
            shader_path = optarg;
            break;
        case 'n':
            n_particles = atoi(optarg);
            break;
        case '?':
            printf("Unknown argument: -%c\n", optopt);
            return 1;
            break;
        default:
            printf("ERROR: unknown getopt return val\n");
            return 1;
            break;
    }

    srand((unsigned int)time(NULL));

    log_info("gravity sim v%d.%d", GRAVITYSIM_VERSION_MAJOR, GRAVITYSIM_VERSION_MINOR);
    
    log_info("number of particles: %d", n_particles);

    particles = (total_particle_state_t *)malloc(sizeof(total_particle_state_t) * n_particles);

    int i;

    for (i = 0; i < n_particles; ++i) {
        particles[i] = generate_state_default(8.0, 2.0, V3(0.0, 0.0, 0.0), V3(1.0, 1.0, 1.0), V3(0.0, 0.0, 0.0), V3(0.0, 0.0, 0.0));
    }

    render_init();


    while (render_update());

    return 0;
}


