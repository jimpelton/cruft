
#
# <root>/test/CMakeLists.txt
#

cmake_minimum_required(VERSION 2.8)
include_directories("../include" "./catch")
add_definitions(-DRES_FOLDER="${CRUFT_RESOURCE_DIR}")
add_subdirectory("test_parsedat")
add_subdirectory("test_util")

