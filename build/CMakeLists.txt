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
target_link_libraries(main evdev udev m)
target_include_directories(main PUBLIC ../include)
set_target_properties(main
   PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_executable(can_vesc_test ../examples/can_vesc_test.c)
target_sources(can_vesc_test PUBLIC ${sources})
target_link_libraries(can_vesc_test evdev udev m)
target_include_directories(can_vesc_test PUBLIC ../include)
set_target_properties(can_vesc_test
   PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_executable(tank_track_movement ../examples/tank_track_movement.c)
target_sources(tank_track_movement PUBLIC ${sources})
target_link_libraries(tank_track_movement evdev udev m)
target_include_directories(tank_track_movement PUBLIC ../include)
set_target_properties(tank_track_movement
   PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_executable(pwm_test ../examples/pwm_test.c)
target_sources(pwm_test PUBLIC ${sources})
target_link_libraries(pwm_test evdev udev m)
target_include_directories(pwm_test PUBLIC ../include)
set_target_properties(pwm_test
   PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_executable(gpio_test ../examples/gpio_test.c)
target_sources(gpio_test PUBLIC ${sources})
target_link_libraries(gpio_test evdev udev m)
target_include_directories(gpio_test PUBLIC ../include)
set_target_properties(gpio_test
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
