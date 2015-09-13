#include <stdio.h>
#include <string.h>
#include <derplanner/runtime/runtime.h>
#include <derplanner/runtime/interface.h>
#include <derplanner/runtime/world_printf.h>
#include "blocks.h"

using namespace plnnr;
using namespace blocks;

int main()
{
    const size_t tuple_list_page = 1024;

    blocks::Worldstate world_struct;
    memset(&world_struct, 0, sizeof(world_struct));

    world_struct.atoms[atom_block] = tuple_list::create<block_tuple>(tuple_list_page);
    world_struct.atoms[atom_on_table] = tuple_list::create<on_table_tuple>(tuple_list_page);
    world_struct.atoms[atom_on] = tuple_list::create<on_tuple>(tuple_list_page);
    world_struct.atoms[atom_clear] = tuple_list::create<clear_tuple>(tuple_list_page);
    world_struct.atoms[atom_goal_on_table] = tuple_list::create<goal_on_table_tuple>(tuple_list_page);
    world_struct.atoms[atom_goal_on] = tuple_list::create<goal_on_tuple>(tuple_list_page);
    world_struct.atoms[atom_goal_clear] = tuple_list::create<goal_clear_tuple>(tuple_list_page);
    world_struct.atoms[atom_holding] = tuple_list::create<holding_tuple>(tuple_list_page);
    world_struct.atoms[atom_dont_move] = tuple_list::create<dont_move_tuple>(tuple_list_page);
    world_struct.atoms[atom_need_to_move] = tuple_list::create<need_to_move_tuple>(tuple_list_page);
    world_struct.atoms[atom_put_on_table] = tuple_list::create<put_on_table_tuple>(tuple_list_page);
    world_struct.atoms[atom_stack_on_block] = tuple_list::create<stack_on_block_tuple>(tuple_list_page);

    plnnr::Worldstate world(&world_struct);

    world.append(atom<block_tuple>(1));
    world.append(atom<block_tuple>(2));
    world.append(atom<block_tuple>(3));
    world.append(atom<block_tuple>(4));

    world.append(atom<on_table_tuple>(1));
    world.append(atom<on_table_tuple>(3));

    world.append(atom<on_tuple>(2, 1));
    world.append(atom<on_tuple>(4, 3));

    world.append(atom<clear_tuple>(2));
    world.append(atom<clear_tuple>(4));

    // goal state:
    world.append(atom<goal_on_table_tuple>(1));
    world.append(atom<goal_on_table_tuple>(3));

    world.append(atom<goal_on_tuple>(4, 1));
    world.append(atom<goal_on_tuple>(2, 3));

    world.append(atom<goal_clear_tuple>(4));
    world.append(atom<goal_clear_tuple>(2));

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

    find_plan_init(pstate, blocks::task_solve, blocks::solve_branch_0_expand);

    Find_Plan_Status status =  plan_in_progress;
    while (status == plan_in_progress)
    {
        status = find_plan_step(pstate, world.data());
    }

    if (status == plan_found)
    {
        printf("\nplan found:\n\n");
        Task_Instance* task = bottom<Task_Instance>(pstate.tasks);
        task_printf task_printer;
        plnnr::walk_stack_up<blocks::Task_Type>(task, task_printer);
    }
    else
    {
        printf("plan not found.\n");
    }

    return 0;
}
