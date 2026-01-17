#pragma once

#include <array>

class MainFsmTable
{
  public:
    // Must be 0 based
    enum
    {
        ST_TICKING = 0,
        ST_SETTING_HOURS,
        ST_WAIT_CONFIRM1,
        ST_SETTING_MINUTES,
        ST_WAIT_CONFIRM2,
        ST_UPGRADING,
        NUM_STATES,
    };

    // Must be 0 based, EV_NULL_EVENT=0 is mandatory
    enum
    {
        EV_NULL_EVENT = 0,
        EV_TICK,
        EV_BUTTON_RELEASED,
        EV_BUTTON_HELD_1SEC,
        NUM_EVENTS,
    };

    // Must be 0 based, AC_IGNORE_EVENT=0 is mandatory
    enum
    {
        AC_IGNORE_EVENT = 0,
        AC_TICKING_TICK,
        AC_SETTING_HOURS_TICK,
        AC_SETTING_MINUTES_TICK,
        AC_START_UPGRADE,
        AC_NO_ACTION,
        NUM_ACTIONS,
    };

    struct TransitionResult
    {
        int action;
        int state;
    };

    static constexpr auto transitions = [] {
        std::array<std::array<TransitionResult, NUM_EVENTS>, NUM_STATES> table{};
        table[ST_TICKING][EV_TICK] = {AC_TICKING_TICK, ST_TICKING};
        table[ST_TICKING][EV_BUTTON_RELEASED] = {AC_NO_ACTION, ST_SETTING_HOURS};
        table[ST_TICKING][EV_BUTTON_HELD_1SEC] = {AC_START_UPGRADE, ST_UPGRADING};
        table[ST_SETTING_HOURS][EV_TICK] = {AC_SETTING_HOURS_TICK, ST_SETTING_HOURS};
        table[ST_SETTING_HOURS][EV_BUTTON_RELEASED] = {AC_NO_ACTION, ST_WAIT_CONFIRM1};
        table[ST_WAIT_CONFIRM1][EV_BUTTON_RELEASED] = {AC_NO_ACTION, ST_SETTING_MINUTES};
        table[ST_SETTING_MINUTES][EV_TICK] = {AC_SETTING_MINUTES_TICK, ST_SETTING_MINUTES};
        table[ST_SETTING_MINUTES][EV_BUTTON_RELEASED] = {AC_NO_ACTION, ST_WAIT_CONFIRM2};
        table[ST_WAIT_CONFIRM2][EV_BUTTON_RELEASED] = {AC_NO_ACTION, ST_TICKING};
        return table;
    }();

    static constexpr const char *state_names[] = {
        [ST_TICKING] = "TICKING",
        [ST_SETTING_HOURS] = "SETTING_HOURS",
        [ST_WAIT_CONFIRM1] = "WAIT_CONFIRM1",
        [ST_SETTING_MINUTES] = "SETTING_MINUTES",
        [ST_WAIT_CONFIRM2] = "WAIT_CONFIRM2",
        [ST_UPGRADING] = "UPGRADING",
    };

    static constexpr const char *event_names[] = {
        [EV_NULL_EVENT] = "",
        [EV_TICK] = "TICK",
        [EV_BUTTON_RELEASED] = "BUTTON_RELEASED",
        [EV_BUTTON_HELD_1SEC] = "BUTTON_HELD_1SEC",
    };

    static constexpr const char *action_names[] = {
        [AC_IGNORE_EVENT] = "",
        [AC_TICKING_TICK] = "TICKING_TICK",
        [AC_SETTING_HOURS_TICK] = "SETTING_HOURS_TICK",
        [AC_SETTING_MINUTES_TICK] = "SETTING_MINUTES_TICK",
        [AC_START_UPGRADE] = "START_UPGRADE",
        [AC_NO_ACTION] = "",
    };
};
