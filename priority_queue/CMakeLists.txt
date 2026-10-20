cmake_minimum_required(VERSION 3.10)

project(priority_queue LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCE prior_queue.c addr_heap.c heap.c hash_table.c)
set(HEADER prior_queue.h addr_heap.h heap.h hash_table.h)

add_executable(priority_queue main.c ${SOURCE} ${HEADER})

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(priority_queue PRIVATE
        $<$<CONFIG:Debug>:-O0>
        $<$<NOT:$<CONFIG:Debug>>:-O3>
        -march=native -Wall -Wextra -Wpedantic)
    if(UNIX)
        target_compile_options(priority_queue PRIVATE -fsanitize=address,undefined)
        target_link_libraries(priority_queue PRIVATE -fsanitize=address,undefined)
    endif()
elseif(MSVC)
    target_compile_options(priority_queue PRIVATE /W4
        $<$<CONFIG:Debug>:/Od>
        $<$<NOT:$<CONFIG:Debug>>:/O2>)
endif()
