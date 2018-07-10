
add_executable(gravitysim gravitysim.c log.c gs_math.c render.c render_vals.c gravitysim.h log.h gs_math.h render.h render_vals.h)


add_library(ccgl ccgl.c ccgl_gl.c ccgl.h ccgl_gl.h)

target_link_libraries(gravitysim m ccgl ${GLEW_LIBRARIES} glfw ${OPENGL_LIBRARIES} ${CMAKE_DL_LIBS})

install (FILES gravitysim.h DESTINATION include)
#install (FILES frag.shader DESTINATION share)
