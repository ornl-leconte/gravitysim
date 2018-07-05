
add_executable(gravitysim gravitysim.c log.c gs_math.c render.c   gravitysim.h log.h gs_math.h render.h)

target_link_libraries(gravitysim m glew glfw ${OPENGL_LIBRARIES} ${CMAKE_DL_LIBS})

install (FILES gravitysim.h DESTINATION include)
