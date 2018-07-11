
add_executable(gravitysim gravitysim.c log.c gs_math.c controls.c gs_physics.c render.c render_vals.c gravitysim.h log.h gs_math.h gs_physics.h render.h render_vals.h controls.h phys/naive.c phys/naive_parallel.c)


add_library(ccgl ccgl.c ccgl_gl.c ccgl.h ccgl_gl.h)

target_link_libraries(gravitysim m ccgl ${GLEW_LIBRARIES} glfw ${OPENGL_LIBRARIES} ${CMAKE_DL_LIBS})

install (FILES gravitysim.h DESTINATION include)
#install (FILES frag.shader DESTINATION share)
