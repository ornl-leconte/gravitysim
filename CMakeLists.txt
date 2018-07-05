cmake_minimum_required (VERSION 2.6)

project (gravitysim C)
set (GRAVITYSIM_VERSION_MAJOR 0)
set (GRAVITYSIM_VERSION_MINOR 1)

set (CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(OpenGL REQUIRED)
find_package(glew REQUIRED)
find_package(glfw3 REQUIRED)



#message(STATUS "Hello There")


set (CMAKE_C_FLAGS "-O2 -g -Wall -pedantic -std=c99 ${CMAKE_C_FLAGS}")
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


