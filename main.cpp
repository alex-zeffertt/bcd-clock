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
    // Fixup row screwup
    write_reg(DIGIT0 + col, (bits >> 1) + ((bits & 1) << 7));
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

    for (int i = 0;; i++)
    {
        //        for (int j = 0; j < 8; j++)
        //        {
        //            write_reg(DIGIT0 + ((i + j) & 3), (1 << j));
        //        }
        for (int col = 0; col < 8; col++)
        {
            int row = col;
            write_col(col, row);
        }

        sleep_ms(500);
    }
}
