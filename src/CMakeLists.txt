
set(extra_files "")
set(extra_libs "")

if (OpenCL_FOUND)
    set(extra_files ${extra_files} phys/naive_opencl.c phys/subsec_opencl.c)
    set(extra_libs ${extra_libs} ${OpenCL_LIBRARIES})
endif()



if (CUDA_FOUND)
#    add_library(gravitysim_CUDA phys/naive_cuda.cu)

    add_custom_command(OUTPUT ${CMAKE_SOURCE_DIR}/src/phys/naive_cuda.cu.o
COMMAND ${CUDA_COMPILER_WRAPPER} -I${CMAKE_SOURCE_DIR}/src -I${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/src/phys/naive_cuda.cu -c -o ${CMAKE_SOURCE_DIR}/src/phys/naive_cuda.cu.o
MAIN_DEPENDENCY ${CMAKE_SOURCE_DIR}/src/phys/naive_cuda.cu)
    set(extra_files ${extra_files} ${CMAKE_SOURCE_DIR}/src/phys/naive_cuda.cu.o)
    set(extra_libs ${extra_libs} ${CUDA_LIBRARIES} stdc++) 
endif()

add_executable(gravitysim gravitysim.c log.c gs_math.c controls.c gs_physics.c render.c part.c gs_store.c render_vals.c ccgl.c ccgl_gl.c ccgl.h ccgl_gl.h gravitysim.h log.h gs_math.h part.h gs_physics.h gs_store.h render.h render_vals.h controls.h phys/naive.c phys/naive_parallel.c phys/subsec.c ${extra_files})

message(STATUS "cuda build: ${CUDA_LIBRARIES}")

target_link_libraries(gravitysim m ${CMAKE_DL_LIBS} ${GLEW_LIBRARIES} glfw pthread ${OPENGL_LIBRARIES} glew ${extra_libs})


install (FILES gravitysim.h DESTINATION include)
#install (FILES frag.shader DESTINATION share)
