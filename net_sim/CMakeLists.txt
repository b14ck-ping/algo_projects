add_executable(net_sim main.c)

target_sources(net_sim PRIVATE
    graph.c
)

target_include_directories(net_sim
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)
