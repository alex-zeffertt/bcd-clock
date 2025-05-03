#pragma once

#include <span>
#include "State.h"

/**
 * @brief base class for state machines
 */
class StateMachine
{
  public:
    /**
     * @brief ctor
     *
     * @param states lookup table mapping integer ids to states - not owned
     */
    StateMachine(State *inital_state) : _state(initial_state)
    {
    }

    /**
     * @brief Event handler called once per tick in this state
     */
    void tick()
    {
        State *new_state = _state->tick();
        if (new_state != _state)
        {
            change_state(new_state);
        }
    }

  private:
    void change_state(State &new_state)
    {
        _state->exit();
        new_state = state;
        _state->enter();
    }

    State *_state;
};
