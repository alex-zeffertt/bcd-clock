#include "ExampleFsm.h"

int main()
{
    ExampleFsm fsm{};

    // Change from open -> closed
    fsm.inject_event(ExampleFsm::EV_BUTTON_PRESS);

    // Change from closed -> open
    fsm.inject_event(ExampleFsm::EV_BUTTON_PRESS);

    // Change from open -> closed
    fsm.inject_event(ExampleFsm::EV_BUTTON_PRESS);

    // Change from closed -> open
    fsm.inject_event(ExampleFsm::EV_TIMEOUT);

    return 0;
}
