cmake_minimum_required(VERSION 3.5)
project(libwc)

#set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_COMPILER gcc)
set(CMAKE_C_FLAGS_RELEASE "-O3")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE sources ../src/*.c)

add_executable(main ../examples/main.c)
target_sources(main PUBLIC ${sources})
target_link_libraries(main evdev udev)
target_include_directories(main PUBLIC ../include)
set_target_properties(main
   PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_executable(set_motor_vel ../examples/set_motor_vel.c)
target_sources(set_motor_vel PUBLIC ${sources})
target_link_libraries(set_motor_vel evdev udev)
target_include_directories(set_motor_vel PUBLIC ../include)
set_target_properties(set_motor_vel
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
