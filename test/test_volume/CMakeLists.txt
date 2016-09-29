#
# <root>/test/test_volume/CMakeLists.txt
#


#project(test_util)
add_executable(test_volume test_volume_main.cpp
        test_VoxelOpacityFilter.cpp
        test_OpacityTransferFunction.cpp)


target_link_libraries(test_volume crufterly)
