add_library(hash_table STATIC hash_table.c)

target_include_directories(hash_table
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(hash_table_test hash_table_test.c)

target_link_libraries(hash_table_test PRIVATE hash_table)