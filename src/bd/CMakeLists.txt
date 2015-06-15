#
# shared/src/bd
#

add_subdirectory(context)
add_subdirectory(file)
add_subdirectory(geo)
add_subdirectory(graphics)
add_subdirectory(log)
add_subdirectory(scene)
add_subdirectory(util)
add_subdirectory(volume)

#set(bd_HEADERS
#    "${file_HEADERS}"
#    "${graphics_HEADERS}"
#    "${log_HEADERS}"
#    "${util_HEADERS}"
#    PARENT_SCOPE
#    )

set(bd_SOURCES
	"${context_SOURCES}"
    "${file_SOURCES}"
    "${geo_SOURCES}"
    "${graphics_SOURCES}"
    "${log_SOURCES}"
    "${scene_SOURCES}"
    "${util_SOURCES}"
    "${volume_SOURCES}"
    PARENT_SCOPE
    )



