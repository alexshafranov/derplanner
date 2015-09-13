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

    travel::Worldstate world_struct;
    memset(&world_struct, 0, sizeof(world_struct));

    world_struct.atoms[atom_start] = tuple_list::create<start_tuple>(1);
    world_struct.atoms[atom_finish] = tuple_list::create<finish_tuple>(1);
    world_struct.atoms[atom_short_distance] = tuple_list::create<short_distance_tuple>(tuple_list_page);
    world_struct.atoms[atom_long_distance] = tuple_list::create<long_distance_tuple>(tuple_list_page);
    world_struct.atoms[atom_airport] = tuple_list::create<airport_tuple>(tuple_list_page);

    plnnr::Worldstate world(&world_struct);

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

    World_Printf printer;
    plnnr::reflect(world_struct, printer);

    plnnr::Stack methods(32768);
    plnnr::Stack tasks(32768);
    plnnr::Stack jstack(32768);
    plnnr::Stack trace(32768);

    Planner_State pstate;
    pstate.top_method = 0;
    pstate.top_task = 0;
    pstate.methods = &methods;
    pstate.tasks = &tasks;
    pstate.journal = &jstack;
    pstate.trace = &trace;

    find_plan_init(pstate, travel::task_root, travel::root_branch_0_expand);

    Find_Plan_Status status = plan_in_progress;
    while (status == plan_in_progress)
    {
        status = find_plan_step(pstate, world.data());
    }

    if (status == plan_found)
    {
        printf("\nplan found:\n\n");
        Task_Instance* task = bottom<Task_Instance>(pstate.tasks);
        task_printf task_printer;
        plnnr::walk_stack_up<travel::Task_Type>(task, task_printer);
    }
    else
    {
        printf("plan not found.\n");
    }

    return 0;
}
