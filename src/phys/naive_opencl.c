

#ifndef HAVE_OPENCL
#error OpenCL is required to compile this file
#endif

#include "gs_physics.h"

#include "ccgl_gl.h"

#ifdef MAC
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif
 


// checks a statement for errors
#define CLCHK(st) cl_env.err = st; if (cl_env.err != CL_SUCCESS) { _cl_error_handle(__FILE__, __LINE__); }
#define CLCHK_NOSET(st) st; if (cl_env.err != CL_SUCCESS) { _cl_error_handle(__FILE__, __LINE__); }

// OpenCL info about the environment
struct {

    cl_platform_id platform;
    cl_device_id device;
    cl_context context;
    cl_command_queue queue;
    cl_program program;
    cl_kernel kernel;

    cl_int err;

} cl_env;


struct {

    // 4 * sizeof(float) * n_particles packed of (x, y, z, mass)
    cl_mem in_p;

    // 4 * sizeof(float) * n_particles packed of (x, y, z, _) force vectors for the corresponding particle (fourth is left just for packing reasons)
    cl_mem out_force;

} cl_data;


struct {

} unpack_bufs;


void _cl_error_handle(char * file, int line) {
    printf("OpenCL error at %sL%d: %d\n", cl_env.err);
}

bool _plno_hasinit = false;

void physics_loop_naive_opencl() {

    if (!_plno_hasinit) {
        // initialization code goes here

        CLCHK(clGetPlatformIDs(1, &cl_env.platform, NULL));
        CLCHK(clGetDeviceIDs(cl_env.platform, CL_DEVICE_TYPE_GPU, 1, &cl_env.device, NULL));

        int d_name_l, d_driver_l, d_version_l;

        clGetDeviceInfo(cl_env.device, CL_DEVICE_NAME, 0, NULL, &d_name_l);
        clGetDeviceInfo(cl_env.device, CL_DRIVER_VERSION, 0, NULL, &d_driver_l);
        clGetDeviceInfo(cl_env.device, CL_DEVICE_OPENCL_C_VERSION, 0, NULL, &d_version_l);

        char * d_name = (char*) malloc(d_name_l);
        char * d_driver = (char*) malloc(d_driver_l);
        char * d_version = (char*) malloc(d_version_l);

        clGetDeviceInfo(cl_env.device, CL_DEVICE_NAME, d_name_l, d_name, NULL);
        clGetDeviceInfo(cl_env.device, CL_DRIVER_VERSION, d_driver_l, d_driver, NULL);
        clGetDeviceInfo(cl_env.device, CL_DEVICE_OPENCL_C_VERSION, d_version_l, d_version, NULL);

        log_info("OPENCL, device: '%s', driver: '%s', compute: '%s'", d_name, d_driver, d_version);

        free(d_name); free(d_driver); free(d_version);


        CLCHK_NOSET(cl_env.context = clCreateContext(0, 1, &cl_env.device, NULL, NULL, &cl_env.err));
        CLCHK_NOSET(cl_env.queue = clCreateCommandQueue(cl_env.context, cl_env.device, 0, &cl_env.err));


        char * program_path = malloc(strlen(shared_data_dir) + 4096 + 256);
        sprintf(program_path, "%s/src/kernels/%s", shared_data_dir, "naive_opencl.cl");
        char * program_source = _read_file(program_path);
        free(program_path);

        CLCHK_NOSET(cl_env.program = clCreateProgramWithSource(cl_env.context, 1, (const char **)&program_source, NULL, &cl_env.err));


        CLCHK(clBuildProgram(cl_env.program, 0, NULL, NULL, NULL, NULL));

        printf("HERE\n");

        CLCHK_NOSET(cl_env.kernel = clCreateKernel(cl_env.program, "compute_system", &cl_env.err));


        CLCHK_NOSET(cl_data.in_p = clCreateBuffer(cl_env.context, CL_MEM_READ_ONLY, sizeof(vec4_t) * n_particles, NULL, &cl_env.err));
        //CLCHK_NOSET(cl_data.g_vel = clCreateBuffer(cl_env.context, CL_MEM_READ_WRITE, sizeof(vec4_t) * n_particles, NULL, &cl_env.err));
        CLCHK_NOSET(cl_data.out_force = clCreateBuffer(cl_env.context, CL_MEM_WRITE_ONLY, sizeof(vec4_t) * n_particles, NULL, &cl_env.err));

        _plno_hasinit = true;
    }

    // this means we have to
    //physics_exts.need_recalc_position = false;

    CLCHK(clEnqueueWriteBuffer(cl_env.queue, cl_data.in_p, CL_TRUE, 0, sizeof(vec4_t) * n_particles, particle_data.P, 0, NULL, NULL));

    cl_int cl_n_particles = n_particles;
    cl_float cl_gravity_coef = gravity_coef;

    CLCHK(clSetKernelArg(cl_env.kernel, 0, sizeof(cl_mem), &cl_data.in_p));
    CLCHK(clSetKernelArg(cl_env.kernel, 1, sizeof(cl_mem), &cl_data.out_force));
    CLCHK(clSetKernelArg(cl_env.kernel, 2, sizeof(cl_int), &cl_n_particles));
    CLCHK(clSetKernelArg(cl_env.kernel, 3, sizeof(cl_float), &cl_gravity_coef));

    int global_size = n_particles;
    int local_size = 1;

    CLCHK_NOSET(clEnqueueNDRangeKernel(cl_env.queue, cl_env.kernel, 1, NULL, &global_size, &local_size, 0, NULL, NULL));

    CLCHK(clFinish(cl_env.queue));


    CLCHK(clEnqueueReadBuffer(cl_env.queue, cl_data.out_force, CL_TRUE, 0, sizeof(vec4_t) * n_particles, particle_data.forces, 0, NULL, NULL));

}

