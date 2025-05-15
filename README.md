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

First time:

* Disconnect USB
* Hold down bootsel
* Connect USB
* Release bootesl
* Copy `build/src/bcd_clock.uf2` to new USB storage device

Later times:

* Make sure it is in the UPDATE_TIME state (i.e. time is changing every second)
* Hold down bootsel for > 1 second
* Copy `build/src/bcd_clock.uf2` to new USB storage device

# Debug

Start picocom

```sh
picocom -b 115200 /dev/ttyACM0
```

Try pressing a few buttons to get output

```
Changing state UPDATE_TIME -> SET_HOURS
Changing state SET_HOURS -> SET_MINUTES
Changing state SET_MINUTES -> SET_SECONDS
Changing state SET_SECONDS -> UPDATE_TIME
```

Exit with `C-a C-x`
