add_library(addr_heap STATIC addr_heap.c)

target_include_directories(addr_heap
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)