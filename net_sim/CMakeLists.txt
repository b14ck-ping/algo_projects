add_library(net_sim STATIC prior_queue.c)

# target_link_libraries(priority_queue PRIVATE
#     addr_heap
#     hash_table    
# )

target_include_directories(priority_queue
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(net_sim main.c)

target_link_libraries(net_sim_test PRIVATE net_sim)