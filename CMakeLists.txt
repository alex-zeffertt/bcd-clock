cmake_minimum_required(VERSION 3.12)

include(pico-sdk/pico_sdk_init.cmake)

project(bcd_clock C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(bcd_clock main.cpp Bootsel.cpp)

pico_add_extra_outputs(bcd_clock)

target_link_libraries(bcd_clock pico_stdlib)

pico_enable_stdio_usb(bcd_clock 1)
pico_enable_stdio_uart(bcd_clock 0)

