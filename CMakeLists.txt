#
# <root>/shared/
#

cmake_minimum_required(VERSION 2.8)
#### T h i r d   P a r t y  ################################################

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake
    ${CMAKE_MODULE_PATH}
)

find_package(OpenGL REQUIRED)
find_package(GLFW REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLM REQUIRED)

# Windows requires a few extra definitions.
if (WIN32)
    add_definitions(-DGLEW_STATIC)
    add_definitions(-DNOMINMAX)
endif (WIN32)

if (UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -fdiagnostics-color=auto")
endif (UNIX)

###########################################################################

if (UNIX)
    include_directories("${OPENGL_INCLUDE_DIR}")
endif(UNIX)

include_directories("${GLEW_INCLUDE_DIR}")
include_directories("${GLM_INCLUDE_DIR}")
include_directories("${GLFW_INCLUDE_DIRS}")

#### P r o j e c t   D e f i n i t i o n  ##################################
project(crufterly CXX)

# Enables gl_check() macros.
add_definitions(-DBD_DEBUG)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

add_subdirectory("include/bd")
add_subdirectory("src/bd")

# bd_SOURCES and bd_HEADERS are created in subdirectories and passed up
# to this cmake scope--kinda dirty i think.
add_library(crufterly STATIC "${bd_SOURCES}" "${bd_HEADERS}" )


