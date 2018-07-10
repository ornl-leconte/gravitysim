

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

double GS_looptime = 0.0;


int i_p = 0;

void spawn_cluster(vec3_t center, float rad, float mass, int n) {

    int i;
    for (i = 0; i < n && i_p < n_particles; ++i) {

        float y_rot = 2 * M_PI * random_float();
        float pitch = M_PI * (random_float() - 0.5);
        float trad = rad * random_float();

        vec3_t pos;
        pos.x = trad * cosf(pitch) * cosf(y_rot);
        pos.y = trad * sinf(pitch);
        pos.z = trad * cosf(pitch) * sinf(y_rot);
        particle_data.positions[i_p] = pos;
        particle_data.masses[i_p] = mass;
        particle_data.forces[i_p] = V3(0.0, 0.0, 0.0);
        particle_data.velocities[i_p] = V3(0.0, 0.0, 0.0);
        particle_data.is_enabled[i_p] = true;
        i_p++;
    }
}


int main(int argc, char ** argv) {

    log_set_level(LOG_INFO);

    char c;

    while ((c = getopt(argc, argv, "n:v:S:G:h")) != (char)(-1)) switch (c) {
        case 'h':
            printf("Usage: gravitysim [options]\n");
            printf("\n");
            printf("  -h                prints this help\n");
            printf("  -v [N]            sets verbosity (5=EVERYTHING, 1=ERRORS ONLY)\n");
            printf("  -S [dir]          shader directory\n");
            printf("  -G [f]            gravity constant\n");
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
        case 'G':
            sscanf(optarg, "%f", &gravity_coef);
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
    particle_data.forces = (vec3_t *)malloc(sizeof(vec3_t) * n_particles);
    particle_data.masses = (float *)malloc(sizeof(float) * n_particles);
    particle_data.is_enabled = (bool *)malloc(sizeof(bool) * n_particles);

    spawn_cluster(V3(0.0, 0.0, 0.0), 100, 1.0, n_particles);
/*
    int i;

    for (i = 0; i < n_particles; ++i) {
        particle_data.positions[i] = vec3_gen_default(0.0, 1.0, 0.0, 1.0, 0.0, 1.0);
        particle_data.velocities[i] = V3(0.0, 0.0, 0.0);
        particle_data.forces[i] = V3(0.0, 0.0, 0.0);
        particle_data.masses[i] = expf(2 * float_gen_default(0.5, 1.0));
    }

*/

    sim_data.is_paused = false;

    particle_data._num_enabled = 0;
    int i;
    for (i = 0; i < n_particles; ++i) {
        if (particle_data.is_enabled[i]) particle_data._num_enabled++;
    }

    physics_init();

    render_init();

    bool keep_going = true;

    while (keep_going) {

        control_update();

        float ph_st = glfwGetTime(), ph_et;

        if (!sim_data.is_paused) {
            physics_loop_basic();
            int i;
            for (i = 0; i < n_particles; ++i) {
                if (particle_data.is_enabled[i]) {
                    particle_data.positions[i] = vec3_sub(particle_data.positions[i], physics_data.avg_pos);
                    if (particle_data.positions[i].x > 100.0) particle_data.positions[i].x = 100.0;
                    if (particle_data.positions[i].y > 100.0) particle_data.positions[i].y = 100.0;
                    if (particle_data.positions[i].z > 100.0) particle_data.positions[i].z = 100.0;
                    if (particle_data.positions[i].x < -100.0) particle_data.positions[i].x = -100.0;
                    if (particle_data.positions[i].y < -100.0) particle_data.positions[i].y = -100.0;
                    if (particle_data.positions[i].z < -100.0) particle_data.positions[i].z = -100.0;
                }
            }
        }

        ph_et = glfwGetTime();

        particle_data._num_enabled = 0;
        for (i = 0; i < n_particles; ++i) {
            if (particle_data.is_enabled[i]) particle_data._num_enabled++;
        }

        float rn_st = glfwGetTime(), rn_et;
        keep_going = render_update();
        rn_et = glfwGetTime();

        GS_looptime = rn_et + ph_et - rn_st - ph_st;

        if (log_get_level() >= LOG_DEBUG) {
            printf("loop time: %3.2f ms", 1000.0 * (rn_et + ph_et - rn_st - ph_st));
        }

        if (log_get_level() >= LOG_TRACE) {
            printf(", instances: %d", particle_data._num_enabled);
            if (physics_data.splits > 0) {
                printf(", splits: %d", physics_data.splits);
            }
            if (physics_data.joins > 0) {
                printf(", joins: %d", physics_data.joins);
            }
            printf(", avg_pos: %f,%f,%f", physics_data.avg_pos.x, physics_data.avg_pos.y, physics_data.avg_pos.z);
        }

        if (log_get_level() >= LOG_DEBUG) {
            printf("\n");
        }
        //log_trace("total time: %f ms (%f fps)", (et - st) / 1000.0, 1.0 / (et - st));
    }

    return 0;
}


