

add_subdirectory(context)
add_subdirectory(file)
add_subdirectory(geo)
add_subdirectory(graphics)
add_subdirectory(log)
add_subdirectory(scene)
add_subdirectory(util)




set(bd_HEADERS
    "${context_HEADERS}"
    "${file_HEADERS}"
    "${geo_HEADERS}"
    "${graphics_HEADERS}"
    "${log_HEADERS}"
    "${scene_HEADERS}"
    "${util_HEADERS}"
    PARENT_SCOPE
    )