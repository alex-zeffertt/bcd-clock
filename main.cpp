#include <pico/stdlib.h>
#include <cstdint>
#include <cmath>
#include "hardware/gpio.h"
#include "hardware/sync.h"
#include "hardware/structs/ioqspi.h"
#include "hardware/structs/sio.h"

// Must be power of two
#define TICK_HZ 8

// GPIO PIN numbers
enum
{
    GPIO_PIN_DIN = 16,
    GPIO_PIN_CLK = 17,
    GPIO_PIN_LOAD = 15,
};

// Addresses
enum
{
    NOOP = 0,
    DIGIT0 = 1,
    DIGIT1 = 2,
    DIGIT2 = 3,
    DIGIT3 = 4,
    DIGIT4 = 5,
    DIGIT5 = 6,
    DIGIT6 = 7,
    DIGIT7 = 8,
    DECODE_MODE = 9,
    INTENSITY = 10,
    SCAN_LIMIT = 11,
    SHUTDOWN = 12,
    DISPLAY_TEST = 15,
};

// States
enum
{
    UPDATE_TIME = 0,
    SELECT_HOURS,
    SET_HOURS,
    SELECT_MINUTES,
    SET_MINUTES,
    SELECT_SECONDS,
    SET_SECONDS,
    NUM_STATES,
};

bool __no_inline_not_in_flash_func(get_bootsel_button)()
{
    const uint CS_PIN_INDEX = 1;
    uint32_t flags = save_and_disable_interrupts();

    hw_write_masked(&ioqspi_hw->io[CS_PIN_INDEX].ctrl, GPIO_OVERRIDE_LOW << IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_LSB,
                    IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_BITS);

#define CS_BIT (1u << 1)
    bool button_state = !(sio_hw->gpio_hi_in & CS_BIT);

    hw_write_masked(&ioqspi_hw->io[CS_PIN_INDEX].ctrl, GPIO_OVERRIDE_NORMAL << IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_LSB,
                    IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_BITS);

    restore_interrupts(flags);

    return button_state;
}

void write(uint16_t val)
{
    gpio_put(GPIO_PIN_LOAD, 0);
    sleep_us(1);

    for (int bit = 15; bit >= 0; bit--)
    {
        gpio_put(GPIO_PIN_CLK, 0);
        gpio_put(GPIO_PIN_DIN, (val >> bit) & 1);
        sleep_us(1);
        gpio_put(GPIO_PIN_CLK, 1);
        sleep_us(1);
        gpio_put(GPIO_PIN_CLK, 0);
    }

    gpio_put(GPIO_PIN_LOAD, 1);
}

void write_reg(uint8_t addr, uint8_t val)
{
    uint16_t _val = val;
    _val += ((uint16_t)addr) << 8;
    write(_val);
}

void write_col(uint8_t col, uint8_t bits)
{
    // When wiring I had assumed SEGA-SEGG were 0-6 and SEGDP was 7
    // But it turns out SEGA-SEGG is 1-7 and SEGDP is 0
    write_reg(DIGIT0 + col, (bits >> 1) + ((bits & 1) << 7));
}

uint8_t digit2col(uint8_t decimal_digit)
{
    auto b0 = decimal_digit & 1;
    auto b1 = decimal_digit & 2;
    auto b2 = decimal_digit & 4;
    auto b3 = decimal_digit & 8;

#if 0
    // Use every other LED for readability
    return (b3 << 3) + (b2 << 2) + (b1 << 1) + (b0 << 0);
#else
    return decimal_digit;
#endif
}

void tick()
{
    static int state = UPDATE_TIME;
    static uint64_t bootsel_tickcount = 0;
    static int hours = 0;
    static int minutes = 0;
    static int seconds = 0;
    static int subtick = 0;
    static bool bootsel_prev = false;

    // Workout if bootsel button pressed and for how long
    bool bootsel = get_bootsel_button();
    gpio_put(PICO_DEFAULT_LED_PIN, bootsel);
    bootsel_tickcount = bootsel ? (bootsel_tickcount + 1) : 0;

    // State transitions
    state = (bootsel && !bootsel_prev) ? ((state + 1) % NUM_STATES) : state;
    bootsel_prev = bootsel;

    // Handle states
    switch (state)
    {
    case SET_HOURS:
        if ((bootsel_tickcount & 3) == 3)
            hours = (hours + 1) % 24;
        break;

    case SET_MINUTES:
        if ((bootsel_tickcount & 3) == 3)
            minutes = (minutes + 1) % 60;
        break;

    case SET_SECONDS:
        if ((bootsel_tickcount & 3) == 3)
            seconds = (seconds + 1) % 60;
        break;

    case UPDATE_TIME:
        subtick = (subtick + 1) % TICK_HZ;
        seconds = (subtick) ? seconds : ((seconds + 1) % 60);
        minutes = (seconds || subtick) ? minutes : ((minutes + 1) % 60);
        hours = (minutes || seconds || subtick) ? hours : ((hours + 1) % 24);
        break;

    default:
        break;
    }

    // Display
    bool setting_seconds = (state == SET_SECONDS || state == SELECT_SECONDS);
    bool setting_minutes = (state == SET_MINUTES || state == SELECT_MINUTES);
    bool setting_hours = (state == SET_HOURS || state == SELECT_HOURS);

    write_col(0, digit2col(seconds % 10) | (setting_seconds << 7));
    write_col(1, digit2col(seconds / 10) | (setting_seconds << 7));
    write_col(3, digit2col(minutes % 10) | (setting_minutes << 7));
    write_col(4, digit2col(minutes / 10) | (setting_minutes << 7));
    write_col(6, digit2col(hours % 10) | (setting_hours << 7));
    write_col(7, digit2col(hours / 10) | (setting_hours << 7));
}

int main()
{
    gpio_init(GPIO_PIN_DIN);
    gpio_init(GPIO_PIN_CLK);
    gpio_init(GPIO_PIN_LOAD);
    gpio_init(PICO_DEFAULT_LED_PIN);

    gpio_set_dir(GPIO_PIN_DIN, GPIO_OUT);
    gpio_set_dir(GPIO_PIN_CLK, GPIO_OUT);
    gpio_set_dir(GPIO_PIN_LOAD, GPIO_OUT);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    gpio_put(GPIO_PIN_DIN, 0);
    gpio_put(GPIO_PIN_CLK, 0);
    gpio_put(GPIO_PIN_LOAD, 1);
    gpio_put(PICO_DEFAULT_LED_PIN, 0);

    // Turn all LEDs on
    write_reg(DISPLAY_TEST, 1);
    sleep_ms(500);
    write_reg(DISPLAY_TEST, 0);
    sleep_ms(500);

    for (uint8_t col = 0; col < 8; col++)
        write_col(col, 0);

    // Normal operation
    write_reg(SHUTDOWN, 1);

    // All 8 rows (digits)
    write_reg(SCAN_LIMIT, 7);

    // Medium intensity
    write_reg(INTENSITY, 0xf);

    // No segment (col) decode for any rows (digits)
    write_reg(DECODE_MODE, 0);

    absolute_time_t now = get_absolute_time();

    while (true)
    {
        // Wait for next tick
        absolute_time_t next = delayed_by_ms(now, 1000 / TICK_HZ);

        sleep_until(next);

        tick();

        now = next;
    }
}
