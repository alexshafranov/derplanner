#include <stdio.h>
#include "derplanner/runtime/database.h"
#include "derplanner/runtime/planning.h"
#include "travel.h"

// city
static const plnnr::Id32 SPB = 0;
// airport
static const plnnr::Id32 LED = 1;
// city
static const plnnr::Id32 MSC = 2;
// airport
static const plnnr::Id32 SVO = 3;

static const char* objects[] = { "SPB", "LED", "MSC", "SVO" };

void print_tuple(const char* name, const void* values, plnnr::Param_Layout layout)
{
    printf("%s(", name);

    for (uint32_t i = 0; i < layout.num_params; ++i)
    {
        int32_t value = plnnr::as_Id32(values, layout, i);
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
    plnnr::Plan plan = plnnr::get_plan(state);
    for (uint32_t i = 0; i < plan.length; ++i)
    {
        plnnr::Task_Frame task = plan.tasks[i];
        const char* name = plnnr::get_task_name(domain, task.task_type);
        plnnr::Param_Layout layout = get_task_param_layout(domain, task.task_type);

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
    plnnr::Fact_Database db;
    plnnr::init(&db, &default_mem, &domain->database_req);

    plnnr::Fact_Table* start            = plnnr::find_table(&db, "start");
    plnnr::Fact_Table* finish           = plnnr::find_table(&db, "finish");
    plnnr::Fact_Table* short_distance   = plnnr::find_table(&db, "short_distance");
    plnnr::Fact_Table* long_distance    = plnnr::find_table(&db, "long_distance");
    plnnr::Fact_Table* airport          = plnnr::find_table(&db, "airport");

    plnnr::add_entry(start, SPB);
    plnnr::add_entry(finish, MSC);

    plnnr::add_entry(short_distance, SPB, LED);
    plnnr::add_entry(short_distance, LED, SPB);
    plnnr::add_entry(short_distance, MSC, SVO);
    plnnr::add_entry(short_distance, SVO, MSC);

    plnnr::add_entry(long_distance, SPB, MSC);
    plnnr::add_entry(long_distance, MSC, SPB);
    plnnr::add_entry(long_distance, LED, SVO);
    plnnr::add_entry(long_distance, SVO, LED);
    plnnr::add_entry(long_distance, SPB, SVO);
    plnnr::add_entry(long_distance, SVO, SPB);
    plnnr::add_entry(long_distance, MSC, LED);
    plnnr::add_entry(long_distance, LED, MSC);

    plnnr::add_entry(airport, SPB, LED);
    plnnr::add_entry(airport, MSC, SVO);

    // create planning state.
    plnnr::Planning_State_Config config;
    config.max_depth = 5;
    config.max_plan_length = 3;
    config.expansion_data_size = 1024;
    config.plan_data_size = 1024;
    config.max_bound_tables = domain->database_req.num_tables;

    plnnr::Planning_State pstate;
    plnnr::init(&pstate, &default_mem, &config);
    plnnr::bind(&pstate, domain, &db);

    plnnr::Find_Plan_Status status = plnnr::find_plan(&pstate, &db, domain);
    if (status == plnnr::Find_Plan_Max_Depth_Exceeded)
    {
        printf("maximum expansion depth exceeded!\n");
    }

    if (status == plnnr::Find_Plan_Max_Plan_Length_Exceeded)
    {
        printf("maximum plan length exceeded!\n");
    }

    // resulting plan is stored on the task stack.
    printf("plan:\n");
    print_plan(&pstate, domain);

    return 0;
}
