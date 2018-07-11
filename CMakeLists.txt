cmake_minimum_required (VERSION 2.6)

cmake_policy(SET CMP0017 OLD) 


project (gravitysim C)
set (GRAVITYSIM_VERSION_MAJOR 0)
set (GRAVITYSIM_VERSION_MINOR 1)


set (CMAKE_POSITION_INDEPENDENT_CODE ON)



find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)


if(APPLE)
    set(OpenMP_C "${CMAKE_C_COMPILER}")
    set(OpenMP_C_FLAGS "-fopenmp=libomp -Wno-unused-command-line-argument")
    set(OpenMP_C_LIB_NAMES "libomp" "libgomp" "libiomp5")
    set(OpenMP_libomp_LIBRARY ${OpenMP_C_LIB_NAMES})
    set(OpenMP_libgomp_LIBRARY ${OpenMP_C_LIB_NAMES})
    set(OpenMP_libiomp5_LIBRARY ${OpenMP_C_LIB_NAMES})
endif()


find_package(OpenMP REQUIRED)


#message(STATUS "Hello There")


set (CMAKE_C_FLAGS "-O3 -ffast-math -g -Wall -pedantic -std=c99 ${CMAKE_C_FLAGS}")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__FILENAME__='\"$(subst ${CMAKE_SOURCE_DIR}/,,$(abspath $<))\"'")



set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}")


configure_file (
  "${PROJECT_SOURCE_DIR}/gravitysimconfig.h.in"
  "${PROJECT_BINARY_DIR}/gravitysimconfig.h"
)

if (CMAKE_BUILD_TYPE MATCHES RELEASE)
    message("release mode")
else()
    message("dev mode")
    add_definitions(-DGRAVITYSIM_DEV)
endif()

include_directories("${PROJECT_BINARY_DIR}")
include_directories("${CMAKE_SOURCE_DIR}/src")

add_subdirectory (src) 


#add_subdirectory (examples) 


install (FILES "${PROJECT_BINARY_DIR}/gravitysimconfig.h" DESTINATION include)


