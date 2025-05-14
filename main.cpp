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

int main()
{
    Max7219 max7219(GPIO_PIN_DIN, GPIO_PIN_CLK, GPIO_PIN_LOAD);
    Kwm30881 kwm30881(max7219);
    Bootsel button;
    MainFsm fsm(kwm30881, button, TICK_HZ);
    absolute_time_t now = get_absolute_time();
    MainFsm::State old_state = fsm.get_state();

    stdio_usb_init();

    while (true)
    {
        // Wait for next tick
        absolute_time_t next = delayed_by_ms(now, 1000 / TICK_HZ);

        sleep_until(next);

        fsm.inject_event(MainFsm::EV_TICK);

        MainFsm::State new_state = fsm.get_state();
        if (new_state != old_state)
        {
            printf("Changing state %s -> %s\n", MainFsm::state_names[old_state], MainFsm::state_names[new_state]);
            new_state = old_state;
        }

        now = next;
    }
}
