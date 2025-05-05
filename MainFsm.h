#pragma once

#include "pico/bootrom.h" // reset_usb_boot
#include "Bootsel.h"
#include "Kwm30881.h"
#include "Fsm.h"

enum class State
{
    UPDATE_TIME,
    SETTING_HOURS,
    SETTING_MINUTES,
    SETTING_SECONDS,
    UPGRADING,
};

enum class Event
{
    TICK,
    BUTTON_RELEASED,
    BUTTON_HELD_1SEC,
    NULL_EVENT
};

class MainFsm : public Fsm<MainFsm, State, Event>
{
  public:
    MainFsm(Kwm30881 &kwm30881, Bootsel &button, uint64_t tick_hz)
        : _kwm30881(kwm30881), _button(button), _tick_hz(tick_hz)
    {
        set_initial(State::UPDATE_TIME);
    }

    // provide the transition table for this->inject_transition(event, context) to use
    static const std::map<Base::Key, Base::Value> &transitions()
    {
        static const std::map<Base::Key, Base::Value> tbl = {
            {{Event::TICK, State::UPDATE_TIME}, {&MainFsm::update_time_tick, State::UPDATE_TIME}},
            {{Event::TICK, State::SETTING_HOURS}, {&MainFsm::setting_hours_tick, State::SETTING_HOURS}},
            {{Event::TICK, State::SETTING_MINUTES}, {&MainFsm::setting_minutes_tick, State::SETTING_MINUTES}},
            {{Event::TICK, State::SETTING_SECONDS}, {&MainFsm::setting_seconds_tick, State::SETTING_SECONDS}},
            {{Event::BUTTON_RELEASED, State::UPDATE_TIME}, {nullptr, State::SETTING_HOURS}},
            {{Event::BUTTON_RELEASED, State::SETTING_HOURS}, {nullptr, State::SETTING_MINUTES}},
            {{Event::BUTTON_RELEASED, State::SETTING_MINUTES}, {nullptr, State::SETTING_SECONDS}},
            {{Event::BUTTON_RELEASED, State::SETTING_SECONDS}, {nullptr, State::UPDATE_TIME}},
            {{Event::BUTTON_HELD_1SEC, State::UPDATE_TIME}, {&MainFsm::start_upgrade, State::UPGRADING}},
        };
        return tbl;
    }

  private:
    void common_tick(bool &bootsel, bool &bootsel_negedge)
    {
        tick++;
        bootsel = _button.get();
        bool bootsel_posedge = bootsel && !bootsel_prev;
        bootsel_negedge = bootsel_prev && !bootsel;
        bootsel_prev = bootsel;
        if (bootsel_posedge)
            bootsel_posedge_tick = tick;
    }

    Event update_time_tick(void *context)
    {
        bool bootsel, bootsel_negedge;
        common_tick(bootsel, bootsel_negedge);

        if (bootsel && tick - bootsel_posedge_tick > _tick_hz)
            return Event::BUTTON_HELD_1SEC;

        if (bootsel_negedge)
            return Event::BUTTON_RELEASED;

        // Update time
        subtick = (subtick + 1) % _tick_hz;
        seconds = (subtick) ? seconds : ((seconds + 1) % 60);
        minutes = (seconds || subtick) ? minutes : ((minutes + 1) % 60);
        hours = (minutes || seconds || subtick) ? hours : ((hours + 1) % 24);

        showtime(hours, minutes, seconds);

        return Event::NULL_EVENT;
    }

    Event setting_hours_tick(void *context)
    {
        bool bootsel, bootsel_negedge;
        common_tick(bootsel, bootsel_negedge);

        if (bootsel_negedge)
            return Event::BUTTON_RELEASED;

        if (bootsel && (((tick - bootsel_posedge_tick) & 3) == 3))
            hours = (hours + 1) % 24;

        showtime(hours, minutes, seconds, true, false, false);

        return Event::NULL_EVENT;
    }

    Event setting_minutes_tick(void *context)
    {
        bool bootsel, bootsel_negedge;
        common_tick(bootsel, bootsel_negedge);

        if (bootsel_negedge)
            return Event::BUTTON_RELEASED;

        if (bootsel && (((tick - bootsel_posedge_tick) & 3) == 3))
            minutes = (minutes + 1) % 60;

        showtime(hours, minutes, seconds, false, true, false);

        return Event::NULL_EVENT;
    }

    Event setting_seconds_tick(void *context)
    {
        bool bootsel, bootsel_negedge;
        common_tick(bootsel, bootsel_negedge);

        if (bootsel && (((tick - bootsel_posedge_tick) & 3) == 3))
            seconds = (seconds + 1) % 60;

        if (bootsel_negedge)
            return Event::BUTTON_RELEASED;

        showtime(hours, minutes, seconds, false, false, true);

        return Event::NULL_EVENT;
    }

    Event start_upgrade(void *context)
    {
        reset_usb_boot(0, 0);
        return Event::NULL_EVENT; // Never gets here
    }

    void showtime(uint64_t hours, uint64_t minutes, uint64_t seconds, bool hours_selected = false,
                  bool minutes_selected = false, bool seconds_selected = false)
    {
        _kwm30881.write_col(0, (seconds % 10) | (seconds_selected << 7));
        _kwm30881.write_col(1, (seconds / 10) | (seconds_selected << 7));
        _kwm30881.write_col(3, (minutes % 10) | (minutes_selected << 7));
        _kwm30881.write_col(4, (minutes / 10) | (minutes_selected << 7));
        _kwm30881.write_col(6, (hours % 10) | (hours_selected << 7));
        _kwm30881.write_col(7, (hours / 10) | (hours_selected << 7));
    }

    Kwm30881 &_kwm30881;
    Bootsel &_button;
    const uint64_t _tick_hz;
    uint64_t tick = 0;
    uint64_t bootsel_posedge_tick = 0;
    int hours = 0;
    int minutes = 0;
    int seconds = 0;
    int subtick = 0;
    bool bootsel_prev = false;
};
