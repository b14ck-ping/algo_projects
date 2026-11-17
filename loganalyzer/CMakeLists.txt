cmake_minimum_required(VERSION 3.20)

project(log_analyzer LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)



add_executable(log_analyzer loganalyzer.c)

target_sources(log_analyzer PRIVATE
    file_reader/file_reader.c
)

target_include_directories(log_analyzer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/file_reader
)