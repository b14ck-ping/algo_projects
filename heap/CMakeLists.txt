add_library(heap STATIC heap.c)

target_include_directories(heap
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)
