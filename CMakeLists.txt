cmake_minimum_required(VERSION 3.12)

# Various project options which can be modified using ccmake
option(ROTATE "Rotate the display 90 degrees" OFF)
option(SEGDP_WIRED_TO_8TH_ROW "Set if breadboard miswired as in docs/Schematic.png" ON)
set(GPIO_PIN_DIN 16 CACHE STRING "GPIO pin wired to DIN on Max7219")
set(GPIO_PIN_CLK 17 CACHE STRING "GPIO pin wired to CLK on Max7219")
set(GPIO_PIN_LOAD 15 CACHE STRING "GPIO pin wired to LOAD on Max7219")

include(pico-sdk/pico_sdk_init.cmake)

project(bcd_clock C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_subdirectory(src)
