
#include <string>
#include <stdio.h>
#include "run_13.h"
#include "derplanner/runtime/database.h"
#include "derplanner/runtime/planning.h"
#include "unittestpp.h"

namespace {

void check_plan(const char* expected, plnnr::Planning_State& pstate, const plnnr::Domain_Info* domain)
{
    plnnr::Plan plan = plnnr::get_plan(&pstate);

    std::string actual;
    for (uint32_t i = 0; i < plan.length; ++i)
    {
        plnnr::Task_Frame& task = plan.tasks[i];
        plnnr::Param_Layout layout = plnnr::get_task_param_layout(domain, task.task_type);
        const char* name = plnnr::get_task_name(domain, task.task_type);
        actual += name;
        actual += "(";

        for (uint8_t p = 0; p < layout.num_params; ++p)
        {
            switch (layout.types[p])
            {
            case plnnr::Type_Id32:
                actual += std::to_string(plnnr::as_Id32(task.arguments, layout, p));
                break;
            case plnnr::Type_Id64:
                actual += std::to_string(plnnr::as_Id64(task.arguments, layout, p));
                break;
            case plnnr::Type_Int8:
                actual += std::to_string(plnnr::as_Int8(task.arguments, layout, p));
                break;
            case plnnr::Type_Int32:
                actual += std::to_string(plnnr::as_Int32(task.arguments, layout, p));
                break;
            case plnnr::Type_Int64:
                actual += std::to_string(plnnr::as_Int64(task.arguments, layout, p));
                break;
            case plnnr::Type_Float:
                actual += std::to_string(plnnr::as_Float(task.arguments, layout, p));
                break;
            case plnnr::Type_Vec3: {
                plnnr::Vec3 v = plnnr::as_Vec3(task.arguments, layout, p);
                actual = actual + "vec3(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ", " + std::to_string(v.z) + ")";
                break;
            }
            default:
                CHECK(false);
            }

            if (p < layout.num_params - 1)
                actual += ", ";
        }

        actual += ")";

        if (i < plan.length - 1)
            actual += " ";
    }

    CHECK_EQUAL(expected, actual.c_str());
}

TEST(run_13)
{
    plnnr::Memory_Default default_mem;

    run_13_init_domain_info();
    const plnnr::Domain_Info* domain = run_13_get_domain_info();

    plnnr::Fact_Database db;
    plnnr::init(&db, &default_mem, &domain->database_req);

    plnnr::Planning_State_Config config;
    config.max_depth = 1024;
    config.max_plan_length = 512;
    config.max_bound_tables = domain->database_req.num_tables;
    config.expansion_data_size = 4096;
    config.plan_data_size = 4096;

    plnnr::Planning_State pstate;
    plnnr::init(&pstate, &default_mem, &config);
    plnnr::bind(&pstate, domain, &db);

    // `_` variables

plnnr::Fact_Table* a = plnnr::find_table(&db, "a");
plnnr::Fact_Table* b = plnnr::find_table(&db, "b");

plnnr::add_entry(a, int8_t(1), int8_t(2));
plnnr::add_entry(b, int8_t(3), int8_t(4));

plnnr::add_entry(a, int8_t(1), int8_t(6));
plnnr::add_entry(b, int8_t(8), int8_t(7));


    plnnr::Find_Plan_Status status = plnnr::find_plan(&pstate, &db, domain);
    CHECK_EQUAL(plnnr::Find_Plan_Succeeded, status);

    check_plan("p!(2) p!(3) p!(7)", pstate, domain);

}

}
