add_library(graphs STATIC graph.c
    graph_algorithms.c)

target_include_directories(graphs
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)


add_executable(net_sim main.c)

target_link_libraries(net_sim
    PRIVATE graphs
)


