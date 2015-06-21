
#include <string>
#include <stdio.h>
#include "run_6.h"
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

TEST(run_6)
{
    plnnr::Memory_Default default_mem;

    run_6_init_domain_info();
    const plnnr::Domain_Info* domain = run_6_get_domain_info();

    plnnr::Fact_Database db;
    plnnr::init(db, &default_mem, domain->database_req);

    plnnr::Planning_State_Config config;
    config.max_depth = 1024;
    config.max_plan_length = 512;
    config.expansion_data_size = 4096;
    config.plan_data_size = 4096;

    plnnr::Planning_State pstate;
    plnnr::init(pstate, &default_mem, config);

    // expressions in preconditions and task lists.

plnnr::Fact_Table* a = plnnr::find_table(db, "a");
plnnr::add_entry(a, int8_t(10), int32_t(10), int64_t(29), float(2.0f));
plnnr::add_entry(a, int8_t(50), int32_t(25), int64_t(99), float(0.1f));
plnnr::add_entry(a, int8_t(11), int32_t(10), int64_t(30), float(1.0f));


    plnnr::Find_Plan_Status status = plnnr::find_plan(domain, &db, &pstate);
    CHECK_EQUAL(plnnr::Find_Plan_Succeeded, status);

    check_plan("p!(10) p!(11)", pstate, domain);

}

}
