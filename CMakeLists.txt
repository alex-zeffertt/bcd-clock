#Building:
#
#  sudo apt install cmake gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib
#
#  gh repo clone raspberrypi/pico-sdk
#  gh repo clone raspberrypi/picotool
#
#  cd picotool
#  git submodule update --init --recursive
#  PICO_SDK_PATH=$(pwd)/../pico-sdk cmake -S . -B build
#  (cd build; make)
#  cd -
#
#  cd pico-sdk
#  git submodule update --init --recursive
#  PICOTOOL_FETCH_FROM_GIT_PATH=$(pwd)/../picotool cmake -S. -B build
#  (cd build; make)
#  cd -
#
#  PICOTOOL_FETCH_FROM_GIT_PATH=$(pwd)/picotool cmake -S . -B build
#  (cd build /; make)
#
#Installation:
#
#  Hold down bootsel
#  Connect USB
#  Release bootesl
#  Copy build / bcd_clock.uf2 to new storage device
#  Eject new storage
#  power - cycle RP2040

cmake_minimum_required(VERSION 3.12)

include(pico-sdk/pico_sdk_init.cmake)

project(bcd_clock C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(bcd_clock main.cpp)

pico_add_extra_outputs(bcd_clock)

target_link_libraries(bcd_clock pico_stdlib)
