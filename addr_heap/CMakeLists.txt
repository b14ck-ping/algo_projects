add_library(addr_heap STATIC addr_heap.c)

target_include_directories(addr_heap
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(addr_heap_tests main.c)

target_link_libraries(addr_heap_tests
    PRIVATE addr_heap
)