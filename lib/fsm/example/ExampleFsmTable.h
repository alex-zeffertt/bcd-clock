#pragma once

#include <array>

class ExampleFsmTable
{
  public:
    // Must be 0 based
    enum
    {
        ST_CIRCUIT_OPEN = 0,
        ST_CIRCUIT_CLOSED,
        NUM_STATES,
    };

    // Must be 0 based, EV_NULL_EVENT=0 is mandatory
    enum
    {
        EV_NULL_EVENT = 0,
        EV_BUTTON_PRESS,
        EV_TIMEOUT,
        NUM_EVENTS,
    };

    // Must be 0 based, AC_IGNORE_EVENT=0 is mandatory
    enum
    {
        AC_IGNORE_EVENT = 0,
        AC_CLOSE_CIRCUIT_AND_START_TIMER,
        AC_OPEN_CIRCUIT,
        AC_EMIT_BUTTON_PRESS,
        NUM_ACTIONS,
    };

    struct TransitionResult
    {
        int action;
        int state;
    };

    static constexpr auto transitions = [] {
        std::array<std::array<TransitionResult, NUM_EVENTS>, NUM_STATES> table{};
        table[ST_CIRCUIT_OPEN][EV_BUTTON_PRESS] = {AC_CLOSE_CIRCUIT_AND_START_TIMER, ST_CIRCUIT_CLOSED};
        table[ST_CIRCUIT_CLOSED][EV_BUTTON_PRESS] = {AC_OPEN_CIRCUIT, ST_CIRCUIT_OPEN};
        table[ST_CIRCUIT_CLOSED][EV_TIMEOUT] = {AC_EMIT_BUTTON_PRESS, ST_CIRCUIT_CLOSED};
        return table;
    }();

    static constexpr const char *state_names[] = {
        [ST_CIRCUIT_OPEN] = "CIRCUIT_OPEN",
        [ST_CIRCUIT_CLOSED] = "CIRCUIT_CLOSED",
    };

    static constexpr const char *event_names[] = {
        [EV_NULL_EVENT] = "",
        [EV_BUTTON_PRESS] = "BUTTON_PRESS",
        [EV_TIMEOUT] = "TIMEOUT",
    };

    static constexpr const char *action_names[] = {
        [AC_IGNORE_EVENT] = "",
        [AC_CLOSE_CIRCUIT_AND_START_TIMER] = "CLOSE_CIRCUIT_AND_START_TIMER",
        [AC_OPEN_CIRCUIT] = "OPEN_CIRCUIT",
        [AC_EMIT_BUTTON_PRESS] = "EMIT_BUTTON_PRESS",
    };
};
