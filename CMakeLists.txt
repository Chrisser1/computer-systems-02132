cmake_minimum_required(VERSION 3.16)
project(os_challenge C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -g -O3 -march=native)

find_package(Threads REQUIRED)

include_directories(src)

add_executable(cell-detection
        src/main.c
        src/image_processing.c
        src/image_processing.h
        src/cbmp.c
        src/cbmp.h
)

target_include_directories(cell-detection PRIVATE
    src
)

target_link_libraries(cell-detection PRIVATE
        Threads::Threads
)

# Place the final "server" executable in the project's root directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
