
set(extra_files "")
set(extra_libs "")

if (OpenCL_FOUND)
    set(extra_files ${extra_files} phys/naive_opencl.c)
    set(extra_libs ${extra_libs} ${OpenCL_LIBRARIES})
endif()

add_executable(gravitysim gravitysim.c log.c gs_math.c controls.c gs_physics.c render.c gs_store.c render_vals.c ccgl.c ccgl_gl.c ccgl.h ccgl_gl.h gravitysim.h log.h gs_math.h gs_physics.h gs_store.h render.h render_vals.h controls.h phys/naive.c phys/naive_parallel.c ${extra_files})


if (CUDA_FOUND)
    add_library(gravitysim_CUDA phys/naive_cuda.cu)
    set(extra_libs ${extra_libs} gravitysim_CUDA ${CUDA_LIBRARIES})
endif()

message(STATUS "cuda build: ${CUDA_LIBRARIES}")

target_link_libraries(gravitysim m ${CMAKE_DL_LIBS} ${GLEW_LIBRARIES} glfw pthread ${OPENGL_LIBRARIES} ${extra_libs})


install (FILES gravitysim.h DESTINATION include)
#install (FILES frag.shader DESTINATION share)
