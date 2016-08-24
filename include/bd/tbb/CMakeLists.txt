#
#  include/tbb/CMakeLists.txt
#

set(tbb_HEADERS
        "${CMAKE_CURRENT_SOURCE_DIR}/parallelblockstats.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/parallelblockminmax.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/parallelminmax.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/parallelhistogram.h"
    PARENT_SCOPE
    )
