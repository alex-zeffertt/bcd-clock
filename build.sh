#!/bin/bash
rm -rf picotool/build
rm -rf pico-sdk/build
rm -rf build

(cd picotool ;
 PICO_SDK_PATH=$(pwd)/../pico-sdk cmake -S . -B build)
make -C picotool/build

(cd pico-sdk ;
 PICOTOOL_FETCH_FROM_GIT_PATH=$(pwd)/../picotool cmake -S. -B build)
make -C pico-sdk/build

PICOTOOL_FETCH_FROM_GIT_PATH=$(pwd)/picotool cmake -S . -B build
make -C build
