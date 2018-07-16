

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

char * shared_data_dir = NULL;


// helper method to realloc stuff
void update_N(int N) {
    GS.N = N;
    GS.F = (vec4_t *)realloc(GS.F, sizeof(vec4_t) * N);
    GS.V = (vec4_t *)realloc(GS.V, sizeof(vec4_t) * N);
    GS.P = (vec4_t *)realloc(GS.P, sizeof(vec4_t) * N);
    GS.C = (vec4_t *)realloc(GS.C, sizeof(vec4_t) * N);
}

void spawn_particle(vec4_t part, vec4_t vel) {
    update_N(GS.N+1);
    GS.F[GS.N-1] = V4(0.0, 0.0, 0.0, 0.0);
    GS.V[GS.N-1] = vel;
    GS.P[GS.N-1] = part;
}

void spawn_cluster(vec4_t base, float size, int N) {
    int i;
    for (i = 0; i < N; ++i) {
        vec4_t part = base;
        part.x += (random_float() - 0.5) * size;
        part.y += (random_float() - 0.5) * size;
        part.z += (random_float() - 0.5) * size;
        spawn_particle(part, V4(0.0, 0.0, 0.0, 0.0));
    }
}
void spawn_uvsphere(vec4_t base, float mass, float _rad, int rings, int sectors, int layers) {
    int i, j;
    float R = 1.0 / (rings);
    float S = 1.0 / (sectors);
    float rad;
    int l;
    for (l = 0; l < layers; ++l) {
        float rad = _rad * (l + 1) / layers;
        for (i = 0; i < rings; ++i) {
            for (j = 0; j < sectors; ++j) {
                vec4_t cp = base;
                cp.x = rad * cosf(2 * M_PI * j * S) * sinf(M_PI * i * R);
                cp.y = rad * sinf(-M_PI_2 + M_PI * i * R);
                cp.z = rad * sinf(2 * M_PI * j * S) * sinf(M_PI * i * R);
                cp.w = mass;
                spawn_particle(cp, V4(0.0, 0.0, 0.0, 0.0));
            }
        }
    }
}




void run_generator(char * file_path) {

    FILE * fp = fopen(file_path, "r");
    if (fp == NULL) {
        printf("ERROR opening generator file '%s'\n", file_path);
        exit(1);
    }

    char cur_func[256];
    char cur_args[4096];

    while (!feof(fp)) {
        int r = fscanf(fp, "%s ", cur_func);
        if (r != 1) {
            continue;
        }

        fgets(cur_args, 4096, fp);

        if (strcmp(cur_func, "p") == 0) {
            vec4_t part, vel;
            if (7 == sscanf(cur_args, "%f,%f,%f %f %f,%f,%f\n", &part.x, &part.y, &part.z, &part.w, &vel.x, &vel.y, &vel.z)) {
                spawn_particle(part, vel);
            } else if (4 == sscanf(cur_args, "%f,%f,%f %f\n", &part.x, &part.y, &part.z, &part.w)) {
                spawn_particle(part, V4(0.0, 0.0, 0.0, 0.0));
            } else if (3 == sscanf(cur_args, "%f,%f,%f\n", &part.x, &part.y, &part.z)) {
                part.w = 1.0;
                spawn_particle(part, V4(0.0, 0.0, 0.0, 0.0));
            } else {
                printf("ERROR: func 'p': couldn't find any valid argument combinations\n");
                exit(1);
            }
        } else if (strcmp(cur_func, "uvs") == 0) {
            vec4_t center;
            int rings, sectors, layers;
            float mass, rad;
            if (8 == sscanf(cur_args, "%f,%f,%f %d,%d,%d %f %f\n", &center.x, &center.y, &center.z, &rings, &sectors, &layers, &rad, &mass)) {
                spawn_uvsphere(center, mass, rad, rings, sectors, layers);
            } else {
                printf("ERROR: func 'uvs': couldn't find any valid argument combinations\n");
                exit(1);
            }
        } else if (strcmp(cur_func, "g") == 0) {
            vec4_t start, end, num;
            float mass;
            if (10 == sscanf(cur_args, "%f,%f,%f %f,%f,%f %f,%f,%f %f\n", &start.x, &start.y, &start.z, &end.x, &end.y, &end.z, &num.x, &num.y, &num.z, &mass)) {
            } else if (9 == sscanf(cur_args, "%f,%f,%f %f,%f,%f %f,%f,%f\n", &start.x, &start.y, &start.z, &end.x, &end.y, &end.z, &num.x, &num.y, &num.z)) {
                mass = 1.0f;
            } else {
                printf("ERROR: func 'g': couldn't find any valid argument combinations\n");
            }

            int nX = (int)floor(num.x+0.5), nY = (int)floor(num.y+0.5), nZ = (int)floor(num.x+0.5);
            float fx, fy, fz;
            for (fx = start.x; fx < end.x; fx += (end.x - start.x) / nX)
            for (fy = start.y; fy < end.y; fy += (end.y - start.y) / nY)
            for (fz = start.z; fz < end.z; fz += (end.z - start.z) / nZ) {
                spawn_particle(V4(fx, fy, fz, mass), V4(0.0, 0.0, 0.0, 0.0));
            }

        } else if (strlen(cur_func) >= 1 && cur_func[0] == '#') {
            // comment
            continue;
        } else {
            printf("ERROR: unknown function: '%s'\n", cur_func);
        }
    }

}


int main(int argc, char ** argv) {

    log_set_level(LOG_INFO);

    render.win_width = 640;
    render.win_height = 480;

    render.buffering = 1;
    render.show = true;

    GS.is_paused = false;
    GS.n_frames = 0;
    GS.G = 0.01;
    GS.coll_B = 12.0f;
    GS.dt = 1.0f / 60.0f;
    GS.tt = 0.0f;
    GS.N = 0;

    shared_data_dir = ".";

    char * sim_write = NULL, * sim_read = NULL;

    char c;
    while ((c = getopt(argc, argv, "N:f:v:S:w:G:B:O:I:L:b:XPh")) != (char)(-1)) switch (c) {
        case 'h':
            printf("Usage: gravitysim [options]\n");
            printf("\n");
            printf("  -h                prints this help\n");
            printf("  -v [N]            sets verbosity (5=EVERYTHING, 1=ERRORS ONLY)\n");
            printf("  -S [dir]          sets the shared directory (${SHARED_DIR}/src should be where most stuff is stored)\n");
            printf("  -w [WxH]          width and height of window (0 for either for fullscreen)\n");
            printf("  -f [file]         run a generator file\n");
            printf("  -N [N]            set number of particles\n");
            printf("  -G [f]            gravitation constant\n");
            printf("  -B [f]            beta collision factor\n");
            printf("  -X                this flag does no GUI (for baking computations)\n");
            printf("  -P                begin the simulation paused\n");
            printf("  -L [f]            fixed loop time (for baking computations)\n");
            printf("  -b [n]            buffer swap (2+=buffering, 1=default, 0=no vsync)\n");
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
            sscanf(optarg, "%dx%d", &render.win_width, &render.win_height);
            break;
        case 'N':
            sscanf(optarg, "%d", &GS.N);
            break;
        case 'f':
            run_generator(optarg);
            break;            
        case 'G':
            sscanf(optarg, "%f", &GS.G);
            break;
        case 'B':
            sscanf(optarg, "%f", &GS.coll_B);
            break;
        case 'L':
            sscanf(optarg, "%f", &GS.dt);
            break;
        case 'b':
            sscanf(optarg, "%d", &render.buffering);
            break;
        case 'X':
            render.show = false;
            break;
        case 'P':
            GS.is_paused = true;
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

    //srand((unsigned int)time(NULL));
    srand(42);

    log_info("gravity sim v%d.%d", GRAVITYSIM_VERSION_MAJOR, GRAVITYSIM_VERSION_MINOR);
    
#ifdef HAVE_OPENCL
    log_info("Running with OpenCL");
#endif

    if (!render.show) log_warn("no GUI will be displayed, we are using -X");

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


    //spawn_cluster(V4(0.0, 0.0, 0.0, 1.0), 100.0, 1024);

    log_info("number of particles: %d", GS.N);

    physics_init();

    if (render.show) {
        control_init();

        render_init();
    } else {
        // we still use the timer
        glfwInit();
    }

    int i;
    for (i = 0; i < GS.N; ++i) {
        GS.C[i] = V4(0.0, 0.0, 1.0, 1.0);
    }

    log_info("all systems initialized");

    bool keep_going = true;

    while (keep_going) {

        if (sim_write != NULL) gs_store_write_frame();
        
        float total_st = glfwGetTime(), total_et;

        if (render.show != 0) {
            control_update();
        }

        float ph_st = glfwGetTime(), ph_et;

        //for (i = 0; i < GS.N; ++i) {
        //    GS.C[i] = V4(0.0, (float)fmod(glfwGetTime(), 1.0), 1.0, 1.0);
        //}
        // these can be hinted at by other implementations that do them more quickly or in one step rather than relying on the gs_physics.c methods to do integrals, etc
        physics_exts.need_recalc_position = true;
        physics_exts.need_collision_handle = true;
        physics_exts.need_clamp = true;

        if (!GS.is_paused) {
            if (sim_read != NULL) {
                gs_store_read_frame();
            } else {
          
                //physics_loop_naive();
                //physics_loop_naive_parallel();
                physics_loop_subsec();

                //physics_loop_naive_cuda();

                //physics_loop_naive_opencl();  

                //if (physics_exts.need_collision_handle) physics_collision_handle();

                // some methods may update position implicitly
                if (physics_exts.need_recalc_position) physics_update_positions();

                // keep them in a box
                if (physics_exts.need_clamp) physics_clamp_positions();
            }
        }

        ph_et = glfwGetTime();

        if (render.show != 0) {
            keep_going = sim_write != NULL || render_update();
        }

        total_et = glfwGetTime();

        if (sim_write == NULL) {
            GS.dt = total_et - total_st;
        }

        if (log_get_level() >= LOG_DEBUG) {
            printf("%d: loop time: %3.2f ms, phys time %3.2f ms", GS.n_frames, 1000.0 * (total_et - total_st), 1000.0 * (ph_et - ph_st));
        }

        if (log_get_level() >= LOG_TRACE) {
            printf(", instances: %d", GS.N);
            //printf(", avg_pos: %f,%f,%f", physics_data.avg_pos.x, physics_data.avg_pos.y, physics_data.avg_pos.z);
        }

        if (log_get_level() >= LOG_DEBUG) {
            printf("\n");
        }
        //log_trace("total time: %f ms (%f fps)", (et - st) / 1000.0, 1.0 / (et - st));
        GS.n_frames++;
    }


    if (sim_write != NULL) gs_store_write_end();
    if (sim_read != NULL) gs_store_read_end();

    return 0;
}


