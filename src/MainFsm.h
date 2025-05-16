#pragma once

#include "pico/bootrom.h" // reset_usb_boot
#include "Bootsel.h"
#include "Kwm30881.h"
#include "Fsm.h"
#include "MainFsmTable.h"
#include <cstdio>

class MainFsm : public MainFsmTable, public Fsm<MainFsmTable>
{
  public:
    MainFsm(Kwm30881 &kwm30881, Bootsel &button, uint64_t tick_hz)
        : _kwm30881(kwm30881), _button(button), _tick_hz(tick_hz)
    {
        set_initial(ST_UPDATE_TIME);
    }

  private:
    Event handle_action(Action action, void *context) override
    {
        auto func = _handlers[action];
        return func ? ((this)->*func)(context) : EV_NULL_EVENT;
    }

    // action handlers

    Event update_time_tick(void *context)
    {
        bool button = common_tick();
        if (button && tick - button_posedge_tick > _tick_hz)
            return EV_BUTTON_HELD_1SEC;

        // Update time
        subtick = (subtick + 1) % _tick_hz;
        seconds = (subtick) ? seconds : ((seconds + 1) % 60);
        minutes = (seconds || subtick) ? minutes : ((minutes + 1) % 60);
        hours = (minutes || seconds || subtick) ? hours : ((hours + 1) % 24);

        showtime();

        return EV_NULL_EVENT;
    }

    Event setting_hours_tick(void *context)
    {
        bool button = common_tick();
        if (button && (((tick - button_posedge_tick) % (_tick_hz / 2)) == 0))
            hours = (hours + 1) % 24;

        showtime(true, false, false);

        return EV_NULL_EVENT;
    }

    Event setting_minutes_tick(void *context)
    {
        bool button = common_tick();
        if (button && (((tick - button_posedge_tick) % (_tick_hz / 2)) == 0))
            minutes = (minutes + 1) % 60;

        showtime(false, true, false);

        return EV_NULL_EVENT;
    }

    Event setting_seconds_tick(void *context)
    {
        bool button = common_tick();
        if (button && (((tick - button_posedge_tick) % (_tick_hz / 2)) == 0))
            seconds = (seconds + 1) % 60;

        showtime(false, false, true);

        return EV_NULL_EVENT;
    }

    Event start_upgrade(void *context)
    {
        reset_usb_boot(0, 0);
        return EV_NULL_EVENT; // Never gets here
    }

    // Loggers

    void log_event(Event event) const override
    {
        if (event != EV_TICK)
            printf("Event: %s\n", event_names[event]);
    }

    void log_state(State oldstate, State newstate) const override
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
    void showtime(bool hours_selected = false, bool minutes_selected = false, bool seconds_selected = false)
    {
        _kwm30881.write_col(0, (seconds % 10) | (seconds_selected << 7));
        _kwm30881.write_col(1, (seconds / 10) | (seconds_selected << 7));
        _kwm30881.write_col(3, (minutes % 10) | (minutes_selected << 7));
        _kwm30881.write_col(4, (minutes / 10) | (minutes_selected << 7));
        _kwm30881.write_col(6, (hours % 10) | (hours_selected << 7));
        _kwm30881.write_col(7, (hours / 10) | (hours_selected << 7));
    }

    using Function = Event (MainFsm::*)(void *);
    static constexpr const Function _handlers[NUM_ACTIONS] = {
        [AC_IGNORE_EVENT] = nullptr,
        [AC_UPDATE_TIME_TICK] = &MainFsm::update_time_tick,
        [AC_SETTING_HOURS_TICK] = &MainFsm::setting_hours_tick,
        [AC_SETTING_MINUTES_TICK] = &MainFsm::setting_minutes_tick,
        [AC_SETTING_SECONDS_TICK] = &MainFsm::setting_seconds_tick,
        [AC_START_UPGRADE] = &MainFsm::start_upgrade,
        [AC_NO_ACTION] = nullptr,
    };

    Kwm30881 &_kwm30881;
    Bootsel &_button;
    const uint64_t _tick_hz;
    uint64_t tick = 0;
    uint64_t button_posedge_tick = 0;
    int hours = 0;
    int minutes = 0;
    int seconds = 0;
    int subtick = 0;
    bool button_prev = false;
};
