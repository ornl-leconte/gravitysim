

#include "gravitysim.h"

#include "gs_math.h"
#include "render.h"
#include "ccgl_gl.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <getopt.h>

float gravity_coef = 9.81;

// global particles array
int n_particles = 10;


int main(int argc, char ** argv) {

    log_set_level(LOG_INFO);

    char c;

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
            add_shader_path(optarg);
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


    particle_data.positions = (vec3_t *)malloc(sizeof(vec3_t) * n_particles);
    particle_data.velocities = (vec3_t *)malloc(sizeof(vec3_t) * n_particles);
    particle_data.masses = (float *)malloc(sizeof(float) * n_particles);

    int i;

    for (i = 0; i < n_particles; ++i) {
        particle_data.positions[i] = vec3_gen_default(0.0, 10.0, 0.0, 10.0, 0.0, 10.0);
        particle_data.velocities[i] = V3(0.0, 0.0, 0.0);
        particle_data.masses[i] = float_gen_default(1.0, 0.5);
    }

    physics_init();

    render_init();

    bool keep_going = true;

    float st = glfwGetTime(), et = glfwGetTime();

    while (keep_going) {
        st = et;
        physics_loop_basic();
        keep_going = render_update();
        et = glfwGetTime();
        log_trace("total time: %f ms (%f fps)", (et - st) / 1000.0, 1.0 / (et - st));
    }

    return 0;
}


