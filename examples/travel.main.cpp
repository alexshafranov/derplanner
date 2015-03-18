#include <stdio.h>
#include "derplanner/runtime/database.h"
#include "derplanner/runtime/planning.h"
#include "travel.h"

// city
static const int SPB = 0;
// airport
static const int LED = 1;
// city
static const int MSC = 2;
// airport
static const int SVO = 3;

static const char* objects[] = { "SPB", "LED", "MSC", "SVO" };

void print_tuple(const char* name, const void* values, plnnr::Param_Layout layout)
{
    printf("%s(", name);

    for (uint32_t i = 0; i < layout.num_params; ++i)
    {
        int32_t value = plnnr::as_Int32(values, layout, i);
        printf("%s", objects[value]);

        if (i + 1 != layout.num_params)
        {
            printf(", ");
        }
    }

    printf(")");
}

void print_plan(const plnnr::Planning_State* state, const plnnr::Domain_Info* domain)
{
    for (uint32_t i = 0; i < state->task_stack.size; ++i)
    {
        plnnr::Task_Frame task = state->task_stack.frames[i];
        const char* name = domain->task_info.names[task.task_type];
        plnnr::Param_Layout layout = domain->task_info.parameters[task.task_type];

        print_tuple(name, task.arguments, layout);
        printf("\n");
    }
}

int main()
{
    // initialize static data in domain code.
    travel_init_domain_info();
    // get the domain description.
    const plnnr::Domain_Info* domain = travel_get_domain_info();

    plnnr::Memory_Default default_mem;

    // create database using format provided in domain info.
    plnnr::Fact_Database db = plnnr::create_fact_database(&default_mem, domain->database_req);

    // start & finish
    plnnr::add_entry(db.tables[0], SPB);
    plnnr::add_entry(db.tables[1], MSC);

    // short_distance
    plnnr::add_entry(db.tables[2], SPB, LED);
    plnnr::add_entry(db.tables[2], LED, SPB);
    plnnr::add_entry(db.tables[2], MSC, SVO);
    plnnr::add_entry(db.tables[2], SVO, MSC);

    // long_distance
    plnnr::add_entry(db.tables[3], SPB, MSC);
    plnnr::add_entry(db.tables[3], MSC, SPB);
    plnnr::add_entry(db.tables[3], LED, SVO);
    plnnr::add_entry(db.tables[3], SVO, LED);
    plnnr::add_entry(db.tables[3], SPB, SVO);
    plnnr::add_entry(db.tables[3], SVO, SPB);
    plnnr::add_entry(db.tables[3], MSC, LED);
    plnnr::add_entry(db.tables[3], LED, MSC);

    // airport
    plnnr::add_entry(db.tables[4], SPB, LED);
    plnnr::add_entry(db.tables[4], MSC, SVO);

    // create planning state.
    plnnr::Planning_State_Config config;
    config.max_depth = 10;
    config.max_plan_length = 10;
    config.expansion_data_size = 1024;
    config.plan_data_size = 1024;

    plnnr::Planning_State pstate = plnnr::create_planning_state(&default_mem, config);

    plnnr::find_plan(domain, &db, &pstate);

    // resulting plan is stored on the task stack.
    printf("plan:\n");
    print_plan(&pstate, domain);

    plnnr::destroy(&default_mem, pstate);
    plnnr::destroy(&default_mem, db);

    return 0;
}
