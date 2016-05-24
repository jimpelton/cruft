#
# <root>/CMakeLists.txt
#

cmake_minimum_required(VERSION 2.8)

#### P r o j e c t   D e f i n i t i o n  ##################################
project(crufterly CXX)

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
    add_definitions(-DGLEW_STATIC)  #Link statically to Glew
    add_definitions(-DNOMINMAX)     #Disable the overrides of std::min/max in Windows.h
endif (WIN32)

if (UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
              -std=c++14 \
              -Wall \
              -Wno-missing-braces \
              -pthread \
              -fdiagnostics-color=auto")
endif (UNIX)

###########################################################################

if (UNIX)
    include_directories("${OPENGL_INCLUDE_DIR}")
endif(UNIX)

# Resource directory is for test code only.
set(CRUFT_RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/res")
#message("Cruft resource directory: ${CRUFT_RESOURCE_DIR}")

include_directories(
        "${GLEW_INCLUDE_DIR}"
        "${GLM_INCLUDE_DIR}"
        "${GLFW_INCLUDE_DIRS}"

    )



# Enables gl_check() macros.
add_definitions(-DBD_DEBUG)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

add_subdirectory("include/bd")
add_subdirectory("src/bd")

# bd_SOURCES and bd_HEADERS are created in subdirectories and passed up
# to this cmake scope--kinda dirty i think.
add_library(crufterly STATIC "${bd_SOURCES}" "${bd_HEADERS}" )
target_link_libraries(crufterly
    "${GLEW_LIBRARIES}"
    "${OPENGL_LIBRARIES}"
    "${GLFW_LIBRARIES}"
    "/usr/local/lib/libtbb.dylib"
    )
add_subdirectory("test")

install(TARGETS crufterly ARCHIVE
        DESTINATION "lib/crufterly/static/${CMAKE_BUILD_TYPE}/")

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/bd/"
        DESTINATION "include/crufterly/bd" FILES_MATCHING PATTERN "*.h")

add_custom_target(install_${PROJECT_NAME}
                  make install
                  DEPENDS ${PROJECT_NAME}
                  COMMENT "Installing ${PROJECT_NAME}")
