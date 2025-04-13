# Setup

```sh
sudo apt install \
    cmake \
    gcc-arm-none-eabi \
    libnewlib-arm-none-eabi \
    libstdc++-arm-none-eabi-newlib

git submodule update --init --recursive
```

# Build
```sh
./build.sh
```

# Quick rebuild of just local project
```sh
make -C build
```

# Installation

*  Hold down bootsel
*  Connect USB
*  Release bootesl
*  Copy `build/bcd_clock.uf2` to new storage device
*  Eject new storage
*  power - cycle RP2040

