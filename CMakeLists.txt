cmake_minimum_required(VERSION 3.20)

project(algo_projects LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
option(ENABLE_SANITIZERS
    "Enable AddressSanitizer and UBSan"
    OFF
)

option(ENABLE_WARNINGS
    "Enable compiler warnings"
    ON
)

option(WARNINGS_AS_ERRORS
    "Treat warnings as errors"
    OFF
)

if(ENABLE_WARNINGS)
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(
            -Wall
            -Wextra
            -Wpedantic
            -Wconversion
            -Wsign-conversion
            -Wshadow
            -Wformat=2
            -Wstrict-prototypes
            -Werror=return-type
        )

        if(WARNINGS_AS_ERRORS)
            add_compile_options(-Werror)
        endif()

    elseif(MSVC)
        add_compile_options(/W4)

        if(WARNINGS_AS_ERRORS)
            add_compile_options(/WX)
        endif()
    endif()
endif()


if (ENABLE_SANITIZERS)
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux"
        AND CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")

        add_compile_options(
            -fsanitize=address,undefined
            -fno-omit-frame-pointer
        )

        add_link_options(
            -fsanitize=address,undefined
        )

    else()
        message(FATAL_ERROR
            "Sanitizers are supported only on Linux with GCC/Clang"
        )
    endif()
endif()

file(GLOB project_entries CONFIGURE_DEPENDS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/*")

foreach(entry IN LISTS project_entries)
    if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${entry}"
       AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${entry}/CMakeLists.txt"
       AND NOT "${CMAKE_CURRENT_SOURCE_DIR}/${entry}" STREQUAL "${CMAKE_BINARY_DIR}")
        add_subdirectory("${entry}" EXCLUDE_FROM_ALL)
    endif()
endforeach()