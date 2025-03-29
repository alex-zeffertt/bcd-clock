#include <pico/stdlib.h>
#include <cstdint>
#include "hardware/gpio.h"
#include "hardware/sync.h"
#include "hardware/structs/ioqspi.h"
#include "hardware/structs/sio.h"

// GPIO PIN numbers
enum
{
    GPIO_PIN_DIN = 17,
    GPIO_PIN_CLK = 16,
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

bool __no_inline_not_in_flash_func(get_bootsel_button)()
{
    const uint CS_PIN_INDEX = 1;
    uint32_t flags = save_and_disable_interrupts();

    hw_write_masked(&ioqspi_hw->io[CS_PIN_INDEX].ctrl, GPIO_OVERRIDE_LOW << IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_LSB,
                    IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_BITS);

    // Note we can't call into any sleep functions in flash right now
    for (volatile int i = 0; i < 1000; ++i)
        ;

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

void tick()
{
    static uint64_t tickcount = -1;
    static int hours = 23;
    static int minutes = 59;
    static int seconds = 59;

    tickcount++;

    if ((tickcount % 10) == 0)
    {
        seconds++;
    }
    if (seconds == 60)
    {
        seconds = 0;
        minutes++;
    }
    if (minutes == 60)
    {
        minutes = 0;
        hours++;
    }
    if (hours == 24)
    {
        hours = 0;
    }

    write_col(0, seconds % 10);
    write_col(1, seconds / 10);

    write_col(3, minutes % 10);
    write_col(4, minutes / 10);

    write_col(6, hours % 10);
    write_col(7, hours / 10);

    gpio_put(PICO_DEFAULT_LED_PIN, get_bootsel_button());
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

    for (uint8_t i = 0; i < 15; i++)
    {
        write_reg(i, 0);
    }

    // Turn all LEDs on
    write_reg(DISPLAY_TEST, 1);
    sleep_ms(500);
    write_reg(DISPLAY_TEST, 0);
    sleep_ms(500);

    // Normal operation
    write_reg(SHUTDOWN, 1);

    // All 7 rows (digits)
    write_reg(SCAN_LIMIT, 7);

    // Medium intensity
    write_reg(INTENSITY, 0xf);

    // No segment (col) decode for any rows (digits)
    write_reg(DECODE_MODE, 0);

    absolute_time_t now = get_absolute_time();

    while (true)
    {
        // Wait 100ms every tick
        absolute_time_t next = delayed_by_ms(now, 100);

        sleep_until(next);

        tick();

        now = next;
    }
}
