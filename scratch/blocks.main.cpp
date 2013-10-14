#include <stdio.h>
#include <string.h>
#include <derplanner/runtime/runtime.h>
#include <derplanner/runtime/interface.h>
#include "blocks.h"

using namespace plnnr;

int main()
{
    const size_t tuple_list_page = 1024;

    ::worldstate world_struct;
    memset(&world_struct, 0, sizeof(world_struct));

    world_struct.block = tuple_list::create<block_tuple>(tuple_list_page);
    world_struct.on_table = tuple_list::create<on_table_tuple>(tuple_list_page);
    world_struct.on = tuple_list::create<on_tuple>(tuple_list_page);
    world_struct.clear = tuple_list::create<clear_tuple>(tuple_list_page);
    world_struct.goal_on_table = tuple_list::create<goal_on_table_tuple>(tuple_list_page);
    world_struct.goal_on = tuple_list::create<goal_on_tuple>(tuple_list_page);
    world_struct.goal_clear = tuple_list::create<goal_clear_tuple>(tuple_list_page);
    world_struct.holding = tuple_list::create<holding_tuple>(tuple_list_page);
    world_struct.dont_move = tuple_list::create<dont_move_tuple>(tuple_list_page);
    world_struct.need_to_move = tuple_list::create<need_to_move_tuple>(tuple_list_page);
    world_struct.put_on_table = tuple_list::create<put_on_table_tuple>(tuple_list_page);
    world_struct.stack_on_block = tuple_list::create<stack_on_block_tuple>(tuple_list_page);

    plnnr::worldstate world(&world_struct);

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

    plnnr::stack mstack(32768);
    plnnr::stack tstack(32768);
    plnnr::stack jstack(32768);

    planner_state pstate;
    pstate.top_method = 0;
    pstate.top_task = 0;
    pstate.mstack = &mstack;
    pstate.tstack = &tstack;
    pstate.journal = &jstack;

    bool result = find_plan(pstate, solve_branch_0_expand, world.data());

    if (result)
    {
        printf("plan found:\n");

        task_instance* task = reverse_task_list(pstate.top_task);

        for (task_instance* t = task; t != 0; t = t->link)
        {
            printf("task_type=%s", task_name((task_type)t->type));

            switch (t->type)
            {
            case task_putdown:
                {
                    putdown_args* args = static_cast<putdown_args*>(t->args);
                    printf("\t(%d)\n", args->_0);
                }
                break;
            case task_unstack:
                {
                    unstack_args* args = static_cast<unstack_args*>(t->args);
                    printf("\t(%d, %d)\n", args->_0, args->_1);
                }
                break;
            case task_pickup:
                {
                    pickup_args* args = static_cast<pickup_args*>(t->args);
                    printf("\t(%d)\n", args->_0);
                }
                break;
            case task_stack:
                {
                    stack_args* args = static_cast<stack_args*>(t->args);
                    printf("\t(%d, %d)\n", args->_0, args->_1);
                }
                break;
            }
        }
    }
    else
    {
        printf("plan not found.\n");
    }

    return 0;
}
