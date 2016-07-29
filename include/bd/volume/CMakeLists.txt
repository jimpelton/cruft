#
#  include/volume/CMakeLists.txt
#

set(volume_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/block.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/blockaveragefilter.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/blockcollection.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/fileblockcollection.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/volume.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/valuerangefilter.h"
    PARENT_SCOPE
    )
