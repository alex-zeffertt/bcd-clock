#pragma once

#include "pico/bootrom.h" // reset_usb_boot
#include "Bootsel.h"
#include "Kwm30881.h"
#include "Fsm.h"
#include "MainFsmTable.h"
#include <cstdio>

class MainFsm : public MainFsmTable, public Fsm<MainFsm>
{
  public:
    MainFsm(Kwm30881 &kwm30881, Bootsel &button, uint64_t tick_hz)
        : _kwm30881(kwm30881), _button(button), _1s(tick_hz), _0_5s(_1s / 2)
    {
        set_initial(ST_TICKING);
    }

  private:
    Event handle_action(Action action, void *context)
    {
        auto func = _handlers[action];
        return func ? ((this)->*func)(context) : EV_NULL_EVENT;
    }

    // action handlers

    Event ticking_tick(void *context)
    {
        bool button = common_tick();
        if (button && tick - button_posedge_tick > _1s)
            return EV_BUTTON_HELD_1SEC;

        // Update time
        subtick = (subtick + 1) % _1s;
        seconds = (subtick) ? seconds : ((seconds + 1) % 60);
        minutes = (seconds || subtick) ? minutes : ((minutes + 1) % 60);
        hours = (minutes || seconds || subtick) ? hours : ((hours + 1) % 24);

        showtime();

        return EV_NULL_EVENT;
    }

    Event setting_hours_tick(void *context)
    {
        bool button = common_tick();
        if (button && (((tick - button_posedge_tick) % _0_5s) == (_0_5s - 1)))
            hours = (hours + 1) % 24;

        showtime(true, false);

        return EV_NULL_EVENT;
    }

    Event setting_minutes_tick(void *context)
    {
        bool button = common_tick();
        if (button && (((tick - button_posedge_tick) % _0_5s) == (_0_5s - 1)))
        {
            seconds += 150;
            while (seconds >= 60)
            {
                seconds -= 60;
                minutes = (minutes + 1) % 60;
            }
        }

        showtime(false, true);

        return EV_NULL_EVENT;
    }

    Event start_upgrade(void *context)
    {
        reset_usb_boot(0, 0);
        return EV_NULL_EVENT; // Never gets here
    }

    // Loggers

    void log_event(Event event) const
    {
        if (event != EV_TICK)
            printf("Event: %s\n", event_names[event]);
    }

    void log_state(State oldstate, State newstate) const
    {
        printf("State change: %s -> %s\n", state_names[oldstate], state_names[newstate]);
    }

    // Utiliies

    // Returns state of button
    bool common_tick()
    {
        tick++;
        bool button = _button.get();
        bool button_posedge = button && !button_prev;
        button_prev = button;
        if (button_posedge)
            button_posedge_tick = tick;
        return button;
    }

    // Updates display
    void showtime(bool hours_selected = false, bool minutes_selected = false)
    {
        uint8_t cols[Kwm30881::NUM_COLS] = {};
        memset(cols, 0, sizeof(cols));

        constexpr int hour_hand_cols[12] = {3, 2, 2, 2, 2, 3, 4, 5, 5, 5, 5, 4};
        constexpr int hour_hand_rows[12] = {5, 5, 4, 3, 2, 2, 2, 2, 3, 4, 5, 5};
        constexpr int minute_hand_cols[24] = {3, 2, 1, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 7, 7, 7, 7, 7, 6, 5, 4};
        constexpr int minute_hand_rows[24] = {7, 7, 7, 6, 5, 4, 3, 2, 1, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 7, 7};

        int n_hours_leds = hours % 12;
        int n_minutes_leds = ((seconds + 60 * minutes) * 24) / 3600;

        for (size_t i = 0; i < n_hours_leds; i++)
        {
            cols[hour_hand_cols[i]] |= 1 << hour_hand_rows[i];
        }

        for (size_t i = 0; i < n_minutes_leds; i++)
        {
            cols[minute_hand_cols[i]] |= 1 << minute_hand_rows[i];
        }

        if (hours_selected)
        {
            for (size_t i = 0; i < 12; i++)
            {
                cols[hour_hand_cols[i]] ^= (1 << hour_hand_rows[i]);
            }
        }

        if (minutes_selected)
        {
            for (size_t i = 0; i < 24; i++)
            {
                cols[minute_hand_cols[i]] ^= (1 << minute_hand_rows[i]);
            }
        }

#ifdef ROTATE
        _kwm30881.write_cols(cols, true);
#else
        _kwm30881.write_cols(cols, false);
#endif
    }

    using Function = Event (MainFsm::*)(void *);
    static constexpr const Function _handlers[NUM_ACTIONS] = {
        [AC_IGNORE_EVENT] = nullptr,
        [AC_TICKING_TICK] = &MainFsm::ticking_tick,
        [AC_SETTING_HOURS_TICK] = &MainFsm::setting_hours_tick,
        [AC_SETTING_MINUTES_TICK] = &MainFsm::setting_minutes_tick,
        [AC_START_UPGRADE] = &MainFsm::start_upgrade,
        [AC_NO_ACTION] = nullptr,
    };

    Kwm30881 &_kwm30881;
    Bootsel &_button;
    const uint64_t _1s;
    const uint64_t _0_5s;
    uint64_t tick = 0;
    uint64_t button_posedge_tick = 0;
    int hours = 0;
    int minutes = 0;
    int seconds = 0;
    int subtick = 0;
    bool button_prev = false;

    // Allow Fsm to call private functions handle_action, log_event, and log_state
    // This is more efficient than virtual functions and avoids implicit heap allocation.
    friend Fsm;
};
