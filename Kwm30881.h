#pragma once

#include <pico/stdlib.h>
#include <cstdint>
#include "Max7219.h"

class Kwm30881
{
  public:
    // Number of columns of LEDs
    static constexpr int NUM_COLS = 8;

    /**
     * @brief ctor
     *
     * @param max7219 device driving the rows (as segments) and cols (as digits)
     */
    Kwm30881(Max7219 &max7219) : _max7219(max7219)
    {
        // Flash all LEDs on and off as a visual self test of wiring
        _max7219.write_reg(Max7219::Address::DISPLAY_TEST, 1);
        sleep_ms(500);
        _max7219.write_reg(Max7219::Address::DISPLAY_TEST, 0);
        sleep_ms(500);

        // Clear all the digits
        for (uint8_t col = 0; col < 8; col++)
            write_col(col, 0);

        // Normal operation
        _max7219.write_reg(Max7219::Address::SHUTDOWN, 1);

        // All 8 rows (digits)
        _max7219.write_reg(Max7219::Address::SCAN_LIMIT, 7);

        // Medium intensity
        _max7219.write_reg(Max7219::Address::INTENSITY, 0xf);

        // No segment (col) decode for any rows (digits)
        _max7219.write_reg(Max7219::Address::DECODE_MODE, 0);
    }

    /**
     * @brief Specify which LEDs on and which are off in a particular column
     *
     * @param col column to modify from 0 to 7 (incl.)
     * @param bits mask identifing which LEDs to enable
     */
    void write_col(uint8_t col, uint8_t bits)
    {
        // When wiring I had assumed SEGA-SEGG were 0-6 and SEGDP was 7
        // But it turns out SEGA-SEGG is 1-7 and SEGDP is 0
        uint8_t val = (bits >> 1) + ((bits & 1) << 7);

        // The DIGITX addresses are consecutive
        uint8_t base = static_cast<uint8_t>(Max7219::Address::DIGIT0);
        auto addr = static_cast<Max7219::Address>(base + col);

        _max7219.write_reg(addr, val);
    }

    /**
     * @brief Set all of the LED states in one go
     */
    void write_cols(const uint8_t bits[NUM_COLS])
    {
        for (int col = 0; col < NUM_COLS; col++)
        {
            write_col(col, bits[col]);
        }
    }

  private:
    Max7219 &_max7219;
};
