#pragma once

#include "Fsm.h"
#include "ExampleFsmTable.h"
#include <iostream>

class ExampleFsm : public ExampleFsmTable, public Fsm<ExampleFsmTable>
{
  public:
    ExampleFsm()
    {
        set_initial(ST_CIRCUIT_OPEN);
    }

  private:
    Event handle_action(Action action, void *context) override
    {
        auto func = _handlers[action];
        return func ? ((this)->*func)(context) : EV_NULL_EVENT;
    }

    Event close_circuit_and_start_timer(void *context)
    {
        std::cout << "closing circuit and starting timer\n";
        return EV_NULL_EVENT;
    }

    Event open_circuit(void *context)
    {
        std::cout << "opening circuit\n";
        return EV_NULL_EVENT;
    }

    Event emit_button_press(void *context)
    {
        std::cout << "emitting button press on timeout\n";
        return EV_BUTTON_PRESS;
    }

    using Function = Event (ExampleFsm::*)(void *);
    static constexpr const Function _handlers[NUM_ACTIONS] = {
        [AC_IGNORE_EVENT] = nullptr,
        [AC_CLOSE_CIRCUIT_AND_START_TIMER] = &ExampleFsm::close_circuit_and_start_timer,
        [AC_OPEN_CIRCUIT] = &ExampleFsm::open_circuit,
        [AC_EMIT_BUTTON_PRESS] = &ExampleFsm::emit_button_press,
    };
};
