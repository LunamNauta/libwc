cmake_minimum_required(VERSION 3.5)
project(libwc)

#set(CMAKE_BUILD_TYPE Debug)
add_compile_definitions(POSIX_C_SOURCE=200809L)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_COMPILER gcc)
set(CMAKE_C_FLAGS_RELEASE "-O3")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE sources ../src/*.c)

add_executable(main ../examples/main.c)
target_sources(main PUBLIC ${sources})
target_link_libraries(main evdev udev ncurses m)
target_include_directories(main PUBLIC ../include)
set_target_properties(main
   PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_library(wc ${sources})
target_include_directories(wc PUBLIC ../include)
set_target_properties(wc
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
