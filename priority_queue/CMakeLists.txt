add_library(priority_queue STATIC prior_queue.c)

target_link_libraries(priority_queue PRIVATE
    addr_heap
    hash_table    
)

target_include_directories(priority_queue
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(priority_queue_test main.c)

target_link_libraries(priority_queue_test PRIVATE priority_queue)