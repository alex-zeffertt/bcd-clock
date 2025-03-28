#include <pico/stdlib.h>
#include <cstdint>

// GPIO PIN numbers
#define GPIO_PIN_DIN 0
#define GPIO_PIN_CLK 1
#define GPIO_PIN_LOAD 2

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
}

int main()
{
    gpio_init(GPIO_PIN_DIN);
    gpio_init(GPIO_PIN_CLK);
    gpio_init(GPIO_PIN_LOAD);

    gpio_set_dir(GPIO_PIN_DIN, GPIO_OUT);
    gpio_set_dir(GPIO_PIN_CLK, GPIO_OUT);
    gpio_set_dir(GPIO_PIN_LOAD, GPIO_OUT);

    gpio_put(GPIO_PIN_DIN, 0);
    gpio_put(GPIO_PIN_CLK, 0);
    gpio_put(GPIO_PIN_LOAD, 1);

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
