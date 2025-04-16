#pragma once

#include <pico/stdlib.h>
#include <cstdint>
#include "hardware/gpio.h"

class Max7219
{
  public:
    /**
     * @brief Register Addresses
     */
    enum class Address
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

    /**
     * @brief ctor
     */
    Max7219(int gpio_pin_din, int gpio_pin_clk, int gpio_pin_load)
        : _gpio_pin_din(gpio_pin_din), _gpio_pin_clk(gpio_pin_clk), _gpio_pin_load(gpio_pin_load)
    {
        // Initialise SPI pins neeeded to talk to device
        gpio_init(_gpio_pin_din);
        gpio_init(_gpio_pin_clk);
        gpio_init(_gpio_pin_load);

        // All are outputs
        gpio_set_dir(_gpio_pin_din, GPIO_OUT);
        gpio_set_dir(_gpio_pin_clk, GPIO_OUT);
        gpio_set_dir(_gpio_pin_load, GPIO_OUT);

        // Set default values
        gpio_put(_gpio_pin_din, 0);
        gpio_put(_gpio_pin_clk, 0);
        gpio_put(_gpio_pin_load, 1);
    }

    /**
     * @brief Write a value into one of the 8-bit registers
     */
    void write_reg(Address addr, uint8_t val)
    {
        uint16_t _val = val;
        _val += static_cast<uint16_t>(addr) << 8;
        write(_val);
    }

  private:
    /**
     * @brief Write a 16 bit value onto the SPI bus
     */
    void write(uint16_t val)
    {
        gpio_put(_gpio_pin_load, 0);
        sleep_us(1);

        for (int bit = 15; bit >= 0; bit--)
        {
            gpio_put(_gpio_pin_clk, 0);
            gpio_put(_gpio_pin_din, (val >> bit) & 1);
            sleep_us(1);
            gpio_put(_gpio_pin_clk, 1);
            sleep_us(1);
            gpio_put(_gpio_pin_clk, 0);
        }

        gpio_put(_gpio_pin_load, 1);
    }

    const int _gpio_pin_din;
    const int _gpio_pin_clk;
    const int _gpio_pin_load;
};
