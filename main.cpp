#include <cstdint>
#include "Max7219.h"
#include "Kwm30881.h"
#include "Bootsel.h"
#include "pico/bootrom.h"
#include <stdio.h>

// State machine tickrate: this is the rate at which we check for bootsel presses
#define TICK_HZ 8

// GPIO PIN numbers
#define GPIO_PIN_DIN 16
#define GPIO_PIN_CLK 17
#define GPIO_PIN_LOAD 15

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
    READY,
};

void tick(Kwm30881 &kwm30881, Bootsel &button)
{
    static int state = UPDATE_TIME;
    static uint64_t tick = 0;
    static uint64_t bootsel_posedge_tick = 0;
    static uint64_t bootsel_negedge_tick = 0;
    static int hours = 0;
    static int minutes = 0;
    static int seconds = 0;
    static int subtick = 0;
    static bool bootsel_prev = false;

    // Update time in ticks
    tick++;

    // Workout if bootsel button pressed
    bool bootsel = button.get();
    bool bootsel_posedge = bootsel && !bootsel_prev;
    bool bootsel_negedge = bootsel_prev && !bootsel;
    bootsel_prev = bootsel;
    if (bootsel_posedge)
        bootsel_posedge_tick = tick;
    if (bootsel_negedge)
        bootsel_negedge_tick = tick;

    // Handle states
    switch (state)
    {
    case UPDATE_TIME:
        subtick = (subtick + 1) % TICK_HZ;
        seconds = (subtick) ? seconds : ((seconds + 1) % 60);
        minutes = (seconds || subtick) ? minutes : ((minutes + 1) % 60);
        hours = (minutes || seconds || subtick) ? hours : ((hours + 1) % 24);

        if (bootsel && tick - bootsel_posedge_tick > TICK_HZ)
            reset_usb_boot(0, 0); // Upgrade
        if (bootsel_negedge)
            state = SELECT_HOURS;
        break;

    case SELECT_HOURS:
        if (bootsel_posedge)
            state = SET_HOURS;
        break;

    case SET_HOURS:
        if (bootsel && ((tick - bootsel_posedge_tick) & 3) == 3)
            hours = (hours + 1) % 24;
        if (bootsel_posedge)
            state = SELECT_MINUTES;
        break;

    case SELECT_MINUTES:
        if (bootsel_posedge)
            state = SET_MINUTES;
        break;

    case SET_MINUTES:
        if (bootsel && ((tick - bootsel_posedge_tick) & 3) == 3)
            minutes = (minutes + 1) % 60;
        if (bootsel_posedge)
            state = SELECT_SECONDS;
        break;

    case SELECT_SECONDS:
        if (bootsel_posedge)
            state = SET_SECONDS;
        break;

    case SET_SECONDS:
        if (bootsel && ((tick - bootsel_posedge_tick) & 3) == 3)
            seconds = (seconds + 1) % 60;
        if (bootsel_posedge)
            state = READY;
        break;

    case READY:
        if (bootsel_negedge)
            state = UPDATE_TIME;
        break;

    default:
        break;
    }

    // Display
    bool setting_seconds = (state == SET_SECONDS || state == SELECT_SECONDS);
    bool setting_minutes = (state == SET_MINUTES || state == SELECT_MINUTES);
    bool setting_hours = (state == SET_HOURS || state == SELECT_HOURS);

    kwm30881.write_col(0, (seconds % 10) | (setting_seconds << 7));
    kwm30881.write_col(1, (seconds / 10) | (setting_seconds << 7));
    kwm30881.write_col(3, (minutes % 10) | (setting_minutes << 7));
    kwm30881.write_col(4, (minutes / 10) | (setting_minutes << 7));
    kwm30881.write_col(6, (hours % 10) | (setting_hours << 7));
    kwm30881.write_col(7, (hours / 10) | (setting_hours << 7));
}

int main()
{
    Max7219 max7219(GPIO_PIN_DIN, GPIO_PIN_CLK, GPIO_PIN_LOAD);
    Kwm30881 kwm30881(max7219);
    Bootsel button;

    absolute_time_t now = get_absolute_time();

    stdio_usb_init();

    while (true)
    {
        // Wait for next tick
        absolute_time_t next = delayed_by_ms(now, 1000 / TICK_HZ);

        sleep_until(next);

        tick(kwm30881, button);

        now = next;
    }
}
