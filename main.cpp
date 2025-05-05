#include <cstdio>
#include <cstdint>
#include "Max7219.h"
#include "Kwm30881.h"
#include "Bootsel.h"
#include "MainFsm.h"

// State machine tickrate: this is the rate at which we check for bootsel presses
#define TICK_HZ 8

// GPIO PIN numbers
#define GPIO_PIN_DIN 16
#define GPIO_PIN_CLK 17
#define GPIO_PIN_LOAD 15

// For debug
const char *state_str[] = {
    [static_cast<int>(State::UPDATE_TIME)] = "UPDATE_TIME",
    [static_cast<int>(State::SETTING_HOURS)] = "SETTING_HOURS",
    [static_cast<int>(State::SETTING_MINUTES)] = "SETTING_MINUTES",
    [static_cast<int>(State::SETTING_SECONDS)] = "SETTING_SECONDS",
    [static_cast<int>(State::UPGRADING)] = "UPGRADING",
};

int main()
{
    Max7219 max7219(GPIO_PIN_DIN, GPIO_PIN_CLK, GPIO_PIN_LOAD);
    Kwm30881 kwm30881(max7219);
    Bootsel button;
    MainFsm fsm(kwm30881, button, TICK_HZ);
    absolute_time_t now = get_absolute_time();
    int old_state = static_cast<int>(fsm.get_state());

    stdio_usb_init();

    while (true)
    {
        // Wait for next tick
        absolute_time_t next = delayed_by_ms(now, 1000 / TICK_HZ);

        sleep_until(next);

        fsm.inject_event(Event::TICK);

        int new_state = static_cast<int>(fsm.get_state());
        if (new_state != old_state)
        {
            printf("Changing state %s -> %s\n", state_str[old_state], state_str[new_state]);
            new_state = old_state;
        }

        now = next;
    }
}
