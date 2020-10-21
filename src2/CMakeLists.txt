set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
#    ${CMAKE_CURRENT_BINARY_DIR}
)

#add_subdirectory (dbg)
add_subdirectory (logging)
add_subdirectory (util)
add_subdirectory (common)
add_subdirectory (network)
#add_subdirectory (msg)
add_subdirectory (driver)
add_subdirectory (holder)
add_subdirectory (application)
add_subdirectory (factory)
add_subdirectory (type_traits)
add_subdirectory (serial)
add_subdirectory (example)
add_subdirectory (runner)
add_subdirectory (main)
