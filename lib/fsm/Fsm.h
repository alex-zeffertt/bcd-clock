// A generic FSM engine that real FSMs inherit from.
#pragma once

#include <map>
#include <utility>

template <typename Table>

class Fsm
{
  public:
    using Event = int;
    using State = int;
    using Action = int;

    void inject_event(Event event, void *context = nullptr)
    {
        // loop until no more actions or NULL_EVENT
        while (event != Table::EV_NULL_EVENT)
        {
            auto [action, next_state] = Table::transitions[_state][event];

            if (action == Table::AC_IGNORE_EVENT)
            {
                // Do not change state
                event = Table::EV_NULL_EVENT;
            }
            else
            {
                // Change state
                event = handle_action(action, context);
                _state = next_state;
            }
        }
    }

    State get_state() const
    {
        return _state;
    }

  protected:
    virtual Event handle_action(Action action, void *context) = 0;

    void set_initial(State s)
    {
        _state = s;
    }

  private:
    State _state;
};
