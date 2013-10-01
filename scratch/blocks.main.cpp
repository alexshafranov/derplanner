#include <stdio.h>
#include <string.h>
#include "blocks.cpp"

int main()
{
    const size_t tuple_list_page = 1024;

    worldstate world;
    memset(&world, 0, sizeof(world));

    world.block = tuple_list::create<block_tuple>(tuple_list_page);
    world.on_table = tuple_list::create<on_table_tuple>(tuple_list_page);
    world.on = tuple_list::create<on_tuple>(tuple_list_page);
    world.clear = tuple_list::create<clear_tuple>(tuple_list_page);
    world.goal_on_table = tuple_list::create<goal_on_table_tuple>(tuple_list_page);
    world.goal_on = tuple_list::create<goal_on_tuple>(tuple_list_page);
    world.goal_clear = tuple_list::create<goal_clear_tuple>(tuple_list_page);
    world.holding = tuple_list::create<holding_tuple>(tuple_list_page);
    world.dont_move = tuple_list::create<dont_move_tuple>(tuple_list_page);
    world.need_to_move = tuple_list::create<need_to_move_tuple>(tuple_list_page);
    world.put_on_table = tuple_list::create<put_on_table_tuple>(tuple_list_page);
    world.stack_on_block = tuple_list::create<stack_on_block_tuple>(tuple_list_page);

    // Initial state:
    {
        block_tuple* tuple;

        tuple = tuple_list::append<block_tuple>(world.block);
        tuple->_0 = 1;

        tuple = tuple_list::append<block_tuple>(world.block);
        tuple->_0 = 2;

        tuple = tuple_list::append<block_tuple>(world.block);
        tuple->_0 = 3;

        tuple = tuple_list::append<block_tuple>(world.block);
        tuple->_0 = 4;
    }

    {
        on_table_tuple* tuple;

        tuple = tuple_list::append<on_table_tuple>(world.on_table);
        tuple->_0 = 1;

        tuple = tuple_list::append<on_table_tuple>(world.on_table);
        tuple->_0 = 3;
    }

    {
        on_tuple* tuple;

        tuple = tuple_list::append<on_tuple>(world.on);
        tuple->_0 = 2;
        tuple->_1 = 1;

        tuple = tuple_list::append<on_tuple>(world.on);
        tuple->_0 = 4;
        tuple->_1 = 3;
    }

    {
        clear_tuple* tuple;

        tuple = tuple_list::append<clear_tuple>(world.clear);
        tuple->_0 = 2;

        tuple = tuple_list::append<clear_tuple>(world.clear);
        tuple->_0 = 4;
    }

    // Goals:
    {
        goal_on_table_tuple* tuple;

        tuple = tuple_list::append<goal_on_table_tuple>(world.goal_on_table);
        tuple->_0 = 1;

        tuple = tuple_list::append<goal_on_table_tuple>(world.goal_on_table);
        tuple->_0 = 3;
    }

    {
        goal_on_tuple* tuple;

        tuple = tuple_list::append<goal_on_tuple>(world.goal_on);
        tuple->_0 = 4;
        tuple->_1 = 1;

        tuple = tuple_list::append<goal_on_tuple>(world.goal_on);
        tuple->_0 = 2;
        tuple->_1 = 3;
    }

    {
        goal_clear_tuple* tuple;

        tuple = tuple_list::append<goal_clear_tuple>(world.goal_clear);
        tuple->_0 = 4;

        tuple = tuple_list::append<goal_clear_tuple>(world.goal_clear);
        tuple->_0 = 2;
    }

    plnnr::stack mstack(32768);
    plnnr::stack tstack(32768);
    plnnr::stack jstack(32768);

    planner_state pstate;
    pstate.top_method = 0;
    pstate.top_task = 0;
    pstate.mstack = &mstack;
    pstate.tstack = &tstack;
    pstate.journal = &jstack;

    bool result = find_plan(pstate, solve_branch_0_expand, &world);

    if (result)
    {
        printf("plan found:\n");

        task_instance* task = reverse_task_list(pstate.top_task);

        // print plan
        for (task_instance* t = task; t != 0; t = t->link)
        {
            printf("task_type=%d\n", t->type);
        }
    }
    else
    {
        printf("plan not found.\n");
    }

    return 0;
}
