#
#  include/volume/CMakeLists.txt
#

set(volume_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/block.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/blockaverage.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/blockaveragefilter.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/blockcollection.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/blockcollection2.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/fileblock.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/minmax.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/volume.h"
    PARENT_SCOPE
    )
