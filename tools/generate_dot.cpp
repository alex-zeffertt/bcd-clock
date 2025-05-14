#include "MainFsmTable.h"
#include <iostream>

int main()
{
    std::cout << "digraph D{\n";
    std::cout << "graph [ resolution=128, fontname=Arial, fontcolor=blue, fontsize=12 ];\n";
    std::cout << "node [ fontname=Arial, fontcolor=blue, fontsize=12];\n";
    std::cout << "edge [ fontname=Helvetica, fontcolor=blue, fontsize=6 ];\n";

    for (int old_state = 0; old_state < MainFsmTable::NUM_STATES; old_state++)
    {
        for (int event = 0; event < MainFsmTable::NUM_EVENTS; event++)
        {
            auto [action, new_state] = MainFsmTable::transitions[old_state][event];
            if (action != MainFsmTable::AC_IGNORE_EVENT)
            {
                std::cout << MainFsmTable::state_names[old_state]               //
                          << " -> " << MainFsmTable::state_names[new_state]     //
                          << " [label=<<u>" << MainFsmTable::event_names[event] //
                          << "</u><br/>" << MainFsmTable::action_names[action]  //
                          << ">];\n";
            }
        }
    }
    std::cout << "}\n";
    return 0;
}
