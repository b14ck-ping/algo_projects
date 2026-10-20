cmake_minimum_required(VERSION 3.12)

project(algo_projects LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Each immediate subdirectory with a CMakeLists.txt defines its own targets.
file(GLOB project_entries CONFIGURE_DEPENDS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/*")

foreach(entry IN LISTS project_entries)
    if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${entry}"
       AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${entry}/CMakeLists.txt"
       AND NOT "${CMAKE_CURRENT_SOURCE_DIR}/${entry}" STREQUAL "${CMAKE_BINARY_DIR}")
        add_subdirectory("${entry}")
    endif()
endforeach()
