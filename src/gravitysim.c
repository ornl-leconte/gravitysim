

#include "gravitysim.h"

#include "gs_math.h"
#include "gs_physics.h"
#include "render.h"
#include "ccgl_gl.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <getopt.h>

#include "gs_store.h"

vec4_t universal_gravity = V4(0.0, 0.0, 0.0, 0.0);

float gravity_coef = 0.1;

float GS_looptime = 0.0;

char * shared_data_dir = NULL;

// global particles array
int n_particles;

int i_p = 0;

int n_frames;


void spawn_particle(vec4_t part) {
    particle_data.P[i_p] = part;
    particle_data.forces[i_p] = V4(0.0, 0.0, 0.0, 0.0);
    particle_data.velocities[i_p] = V4(0.0, 0.0, 0.0, 0.0);
    particle_data.is_enabled[i_p] = true;
    i_p++;
}

void spawn_cluster(vec4_t center, float rad, float mass, int n) {

    int i;
    for (i = 0; i < n && i_p < n_particles; ++i) {

        float y_rot = 2 * M_PI * random_float();
        float pitch = M_PI * (random_float() - 0.5);
        float trad = rad * random_float();

        vec4_t part;
        part.x = center.x + trad * cosf(pitch) * cosf(y_rot);
        part.y = center.y + trad * sinf(pitch);
        part.z = center.z + trad * cosf(pitch) * sinf(y_rot);
        part.w = mass;
        spawn_particle(part);
    }
}


int main(int argc, char ** argv) {

    log_set_level(LOG_INFO);

    win_width = 640;
    win_height = 480;

    render_info.buffering = 1;


    bool do_show = true;

    float L_time = -1.0f;

    char c;

    shared_data_dir = ".";

    char * sim_write = NULL, * sim_read = NULL;

    while ((c = getopt(argc, argv, "n:v:S:w:G:F:O:I:L:B:Xh")) != (char)(-1)) switch (c) {
        case 'h':
            printf("Usage: gravitysim [options]\n");
            printf("\n");
            printf("  -h                prints this help\n");
            printf("  -v [N]            sets verbosity (5=EVERYTHING, 1=ERRORS ONLY)\n");
            printf("  -S [dir]          sets the shared directory (${SHARED_DIR}/src should be where most stuff is stored)\n");
            printf("  -w [WxH]          width and height of window (0 for either for fullscreen)\n");
            printf("  -G [f]            gravitation constant\n");
            printf("  -F [f,f,f]        additional gravity vector\n");
            printf("  -X                this flag does no GUI (for baking computations)\n");
            printf("  -L [f]            fixed loop time (for baking computations)\n");
            printf("  -B [n]            buffer swap (2+=buffering, 1=default, 0=no vsync)\n");
            printf("  -O [file]         store output into file\n");
            printf("  -I [file]         replay baked computation output from -O\n");
            printf("\n");
            return 0;
            break;
        case 'v':
            log_set_level(atoi(optarg));
            break;
        case 'S':
            //add_shader_path(optarg);
            shared_data_dir = strdup(optarg);
            break;
        case 'w':
            sscanf(optarg, "%dx%d", &win_width, &win_height);
            break;
        case 'n':
            n_particles = atoi(optarg);
            break;
        case 'G':
            sscanf(optarg, "%f", &gravity_coef);
            break;
        case 'F':
            sscanf(optarg, "%f,%f,%f", &universal_gravity.x, &universal_gravity.y, &universal_gravity.z);
            break;
        case 'L':
            sscanf(optarg, "%f", &L_time);
            break;
        case 'B':
            sscanf(optarg, "%d", &render_info.buffering);
            break;
        case 'X':
            do_show = false;
            break;
        case 'O':
            sim_write = strdup(optarg);
            break;
        case 'I':
            sim_read = strdup(optarg);
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
    
#ifdef HAVE_OPENCL
    log_info("Running with OpenCL");
#endif

    if (!do_show) log_warn("no GUI will be displayed, we are using -X");

    if (sim_write != NULL && sim_read != NULL) {
        log_error("Can't specify -I and -O at the same time");
        exit(1);
    }

    if (sim_write != NULL) {
        log_info("writing output to '%s'", sim_write);
        gs_store_write_init(sim_write);
    }

    if (sim_read != NULL) {
        log_info("replaying simulation '%s'", sim_read);
        gs_store_read_init(sim_read);
    }

    log_info("number of particles: %d", n_particles);


    particle_data.P = (vec4_t *)malloc(sizeof(vec4_t) * n_particles);
    particle_data.velocities = (vec4_t *)malloc(sizeof(vec4_t) * n_particles);
    particle_data.forces = (vec4_t *)malloc(sizeof(vec4_t) * n_particles);
    particle_data.is_enabled = (bool *)malloc(sizeof(bool) * n_particles);


    int i;
    for (i = 0; i < n_particles; ++i) {
        particle_data.is_enabled[i] = false;
    }


    //spawn_cluster(V4(50.0, 50.0, 0.0, 0.0), 50.0, 1000.0, 1);
    //spawn_cluster(V4(-50.0, 50.0, 0.0, 0.0), 1.0, 100.0, 1);
    //spawn_cluster(V4(50.0, -50.0, 0.0, 0.0), 20.0, 500.0, 1);
    /*
    spawn_cluster(V4(-10.0, -10.0, -10.0, 0.0), 1.0, 1.0, n_particles/4);
    spawn_cluster(V4(-10.0, -10.0, 10.0, 0.0), 1.0, 1.0, n_particles/4);
    spawn_cluster(V4(-10.0, 10.0, -10.0, 0.0), 1.0, 1.0, n_particles/4);
    spawn_cluster(V4(-10.0, 10.0, 10.0, 0.0), 1.0, 1.0, n_particles/4);
    */
/*
   int N = (int)cbrtf((float)n_particles);

   for (i = 0; i < n_particles; ++i) {
       float cx = 2.0f * fmod((float)i, (float)N) / N - 1.0f;
       float cy = 2.0f * fmod((float)i / (float)N, (float)N) / N - 1.0f;
       float cz = 2.0f * fmod((float)i / (float)(N*N), (float)N) / N - 1.0f;
       spawn_particle(V4(100.0 * cx, 100.0 * cy, 100.0 * cz, 1.0));
   }*/
   /*
    spawn_cluster(V4(10.0, 70.0, 50.0, 0.0), 10.0, 1.0, n_particles/4);
    spawn_cluster(V4(-20.0, -50.0, 20.0, 0.0), 10.0, 1.0, n_particles/4);
    spawn_cluster(V4(50.0, 30.0, -80.0, 0.0), 10.0, 1.0, n_particles/4);
    spawn_cluster(V4(-90.0, -80.0, 0.0, 0.0), 10.0, 1.0, n_particles/4);
*/

    sim_data.is_paused = false;//true;

    particle_data._num_enabled = 0;
    for (i = 0; i < n_particles; ++i) {
        if (particle_data.is_enabled[i]) particle_data._num_enabled++;
    }

    n_frames = 0;

    physics_init();

    if (do_show != 0) {
        control_init();

        render_init();
    } else {
        // we still use the timer
        glfwInit();
    }

    log_info("all systems initialized");

    bool keep_going = true;

    while (keep_going) {

        if (sim_write != NULL) gs_store_write_frame();
        
        float total_st = glfwGetTime(), total_et;

        if (do_show != 0) {
            control_update();
        }

        float ph_st = glfwGetTime(), ph_et;

        physics_exts.need_recalc_position = true;
        physics_exts.need_add_gravity = true;
        physics_exts.need_collision_handle = true;
        physics_exts.need_clamp = true;

        if (!sim_data.is_paused) {
            if (sim_read != NULL) {
                gs_store_read_frame();
            } else {
                //physics_loop_naive_cuda();
                //physics_loop_naive_opencl();            
                physics_loop_naive();
                //physics_loop_naive_parallel();

                // universal gravity constant
                if (physics_exts.need_add_gravity) physics_add_gravity();

                if (physics_exts.need_collision_handle) physics_collision_handle();

                // some methods may update position implicitly
                if (physics_exts.need_recalc_position) physics_update_positions();

                // keep them in a box
                if (physics_exts.need_clamp) physics_clamp_positions();
            }
        }

        ph_et = glfwGetTime();


        if (do_show != 0) {
            keep_going = sim_write != NULL || render_update();
        }

        total_et = glfwGetTime();

        if (L_time < 0.0f) {
            GS_looptime = total_et - total_st;
        } else {
            GS_looptime = L_time;
        }

        if (log_get_level() >= LOG_DEBUG) {
            printf("%d: loop time: %3.2f ms, phys time %3.2f ms", n_frames, 1000.0 * (total_et - total_st), 1000.0 * (ph_et - ph_st));
        }

        if (log_get_level() >= LOG_TRACE) {
            printf(", instances: %d", particle_data._num_enabled);
            //printf(", avg_pos: %f,%f,%f", physics_data.avg_pos.x, physics_data.avg_pos.y, physics_data.avg_pos.z);
        }

        if (log_get_level() >= LOG_DEBUG) {
            printf("\n");
        }
        //log_trace("total time: %f ms (%f fps)", (et - st) / 1000.0, 1.0 / (et - st));
        n_frames++;
    }


    if (sim_write != NULL) gs_store_write_end();
    if (sim_read != NULL) gs_store_read_end();

    return 0;
}


