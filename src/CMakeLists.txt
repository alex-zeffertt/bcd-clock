if(ROTATE)
    add_compile_definitions(ROTATE)
endif()

if(SEGDP_WIRED_TO_8TH_ROW)
    add_compile_definitions(SEGDP_WIRED_TO_8TH_ROW)
endif()
add_compile_definitions(GPIO_PIN_DIN=${GPIO_PIN_DIN})
add_compile_definitions(GPIO_PIN_CLK=${GPIO_PIN_CLK})
add_compile_definitions(GPIO_PIN_LOAD=${GPIO_PIN_LOAD})

add_executable(bcd_clock main.cpp Bootsel.cpp)

pico_add_extra_outputs(bcd_clock)

target_link_libraries(bcd_clock pico_stdlib)
target_include_directories(bcd_clock PRIVATE ../lib/fsm)

pico_enable_stdio_usb(bcd_clock 1)
pico_enable_stdio_uart(bcd_clock 0)
