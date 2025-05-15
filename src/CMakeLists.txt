add_executable(bcd_clock main.cpp Bootsel.cpp)

pico_add_extra_outputs(bcd_clock)

target_link_libraries(bcd_clock pico_stdlib)
target_include_directories(bcd_clock PRIVATE ../lib/fsm)

pico_enable_stdio_usb(bcd_clock 1)
pico_enable_stdio_uart(bcd_clock 0)
