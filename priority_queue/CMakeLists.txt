cmake_minimum_required(VERSION 3.10) 
set(CMAKE_C_STANDARD 11)

project(priority_queue)

set(SOURCE prior_queue.c heap.c hash_table.c)#
set(HEADER prior_queue.h heap.h hash_table.h)#

add_executable(main main.c)	