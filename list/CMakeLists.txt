add_library(list STATIC list.c)

target_include_directories(list
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)