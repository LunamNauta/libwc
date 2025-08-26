cmake_minimum_required(VERSION 3.5)
project(libwc)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_COMPILER gcc)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE sources ../src/*.c)

# Change to add_library
add_executable(main ../src/main.c)
target_sources(main PUBLIC ${sources})
target_include_directories(main PUBLIC ../include)

set_target_properties(main
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
