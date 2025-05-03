// A generic FSM engine that real FSMs inherit from.
#pragma once

#include <map>
#include <utility>

template <typename Derived, typename State, typename Event>

class Fsm
{
  public:
    using Base = Fsm;
    using Action = Event (Derived::*)(void *);
    using Key = std::pair<Event, State>;
    using Value = std::pair<Action, State>;

    void inject_event(Event event, void *context = nullptr)
    {
        // loop until no more actions or NULL_EVENT
        while (event != Event::NULL_EVENT)
        {
            auto it = Derived::transitions().find(Key{event, _state});
            if (it == Derived::transitions().end())
                break;

            auto [action, next_state] = it->second;
            if (action == nullptr)
                event = Event::NULL_EVENT;
            else
                event = (static_cast<Derived *>(this)->*action)(context);
            _state = next_state;
        }
    }

    State get_state() const
    {
        return _state;
    }

  protected:
    void set_initial(State s)
    {
        _state = s;
    }

    State _state{};
};
