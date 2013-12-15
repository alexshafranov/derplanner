#include <stdio.h>
#include <string.h>
#include <derplanner/runtime/runtime.h>
#include <derplanner/runtime/interface.h>
#include <derplanner/runtime/world_printf.h>
#include "travel.h"

using namespace plnnr;
using namespace travel;

int main()
{
    const size_t tuple_list_page = 1024;

    travel::worldstate world_struct;
    memset(&world_struct, 0, sizeof(world_struct));

    world_struct.atoms[atom_start] = tuple_list::create<start_tuple>(tuple_list_page);
    world_struct.atoms[atom_finish] = tuple_list::create<finish_tuple>(tuple_list_page);
    world_struct.atoms[atom_short_distance] = tuple_list::create<short_distance_tuple>(tuple_list_page);
    world_struct.atoms[atom_long_distance] = tuple_list::create<long_distance_tuple>(tuple_list_page);
    world_struct.atoms[atom_airport] = tuple_list::create<airport_tuple>(tuple_list_page);

    plnnr::worldstate world(&world_struct);

    const int spb = 0;
    const int led = 1;
    const int svo = 2;
    const int msc = 3;

    world.append(atom<start_tuple>(spb));
    world.append(atom<finish_tuple>(msc));

    world.append(atom<short_distance_tuple>(spb, led));
    world.append(atom<short_distance_tuple>(led, spb));
    world.append(atom<short_distance_tuple>(msc, svo));
    world.append(atom<short_distance_tuple>(svo, msc));

    world.append(atom<long_distance_tuple>(spb, msc));
    world.append(atom<long_distance_tuple>(msc, spb));
    world.append(atom<long_distance_tuple>(led, svo));
    world.append(atom<long_distance_tuple>(svo, led));
    world.append(atom<long_distance_tuple>(spb, svo));
    world.append(atom<long_distance_tuple>(svo, spb));
    world.append(atom<long_distance_tuple>(msc, led));
    world.append(atom<long_distance_tuple>(led, msc));

    world.append(atom<airport_tuple>(spb, led));
    world.append(atom<airport_tuple>(msc, svo));

    world_printf printer;
    plnnr::reflect(world_struct, printer);

    plnnr::stack methods(32768);
    plnnr::stack tasks(32768);
    plnnr::stack jstack(32768);
    plnnr::stack trace(32768);

    planner_state pstate;
    pstate.top_method = 0;
    pstate.top_task = 0;
    pstate.methods = &methods;
    pstate.tasks = &tasks;
    pstate.journal = &jstack;
    pstate.trace = &trace;

    find_plan_init(pstate, travel::task_root, travel::root_branch_0_expand);

    find_plan_status status;

    while (true)
    {
        status = find_plan_step(pstate, world.data());

        // task_printf task_printer;

        // printf("\n");
        // printf("method stack:\n");
        // plnnr::walk_stack_down<travel::task_type>(pstate.top_method, task_printer);
        // printf("===\n");

        // printf("task stack:\n");
        // plnnr::walk_stack_down<travel::task_type>(pstate.top_task, task_printer);
        // printf("===\n");
        // printf("\n");

        if (status != plan_in_progress)
        {
            break;
        }
    }

    if (status == plan_found)
    {
        printf("\nplan found:\n\n");
        task_instance* task = bottom<task_instance>(pstate.tasks);
        task_printf task_printer;
        plnnr::walk_stack_up<travel::task_type>(task, task_printer);
    }
    else
    {
        printf("plan not found.\n");
    }

    for (plnnr::method_trace* trace = plnnr::bottom<plnnr::method_trace>(pstate.trace); trace != plnnr::top<plnnr::method_trace>(pstate.trace)+1; ++trace)
    {
        printf("%s, %d\n", travel::task_name(static_cast<travel::task_type>(trace->type)), trace->branch_index);
    }

    return 0;
}
