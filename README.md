# Setup

```sh
sudo apt install \
    cmake \
    gcc-arm-none-eabi \
    libnewlib-arm-none-eabi \
    libstdc++-arm-none-eabi-newlib

git submodule update --init --recursive
```

# Build SDK, picotool, and bcd-clock
```sh
./build.sh
```

# Quick rebuild of just bcd-clock
```sh
PICOTOOL_FETCH_FROM_GIT_PATH=$(pwd)/picotool cmake -S . -B build
make -C build
```

# Installation

*  Hold down bootsel
*  Connect USB
*  Release bootesl
*  Copy `build/bcd_clock.uf2` to new storage device
*  Eject new storage
*  power - cycle RP2040

