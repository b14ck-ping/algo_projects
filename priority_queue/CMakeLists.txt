cmake_minimum_required(VERSION 3.10)

project(priority_queue LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -march=native -Wall -Wextra -Wpedantic")
#-fsanitize=address,undefined
set(SOURCE prior_queue.c heap.c hash_table.c)
set(HEADER prior_queue.h heap.h hash_table.h)

add_executable(main main.c ${SOURCE} ${HEADER})
