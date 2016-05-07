

add_subdirectory(context)
add_subdirectory(io)
add_subdirectory(geo)
add_subdirectory(graphics)
add_subdirectory(log)
add_subdirectory(scene)
add_subdirectory(util)
add_subdirectory(volume)




set(bd_HEADERS
    "${context_HEADERS}"
    "${io_HEADERS}"
    "${geo_HEADERS}"
    "${graphics_HEADERS}"
    "${log_HEADERS}"
    "${scene_HEADERS}"
    "${util_HEADERS}"
    "${volume_HEADERS}"
    PARENT_SCOPE
    )