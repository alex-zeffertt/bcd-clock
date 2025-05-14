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


# External project to build generate_dot a host-side tool
include(ExternalProject)
ExternalProject_Add(generate_dot
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/tools
    BINARY_DIR ${CMAKE_BINARY_DIR}/tools/build
    BUILD_BYPRODUCTS ${CMAKE_BINARY_DIR}/tools/build/generate_dot
    INSTALL_COMMAND ""
)

# Ensure the generated file is ready
add_custom_target(MainFsm.dot
  DEPENDS generate_dot
  COMMAND ${CMAKE_BINARY_DIR}/tools/build/generate_dot > MainFsm.dot
)

# Ensure the generated file is ready
add_custom_target(MainFsm.png
  DEPENDS MainFsm.dot
  COMMAND cat MainFsm.dot | dot -Tpng -o MainFsm.png
)
