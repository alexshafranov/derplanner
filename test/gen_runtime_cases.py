import os
import glob
import subprocess

template = '''
#include <string>
#include <stdio.h>
#include <stdarg.h>
#include "%(domain_header)s"
#include "derplanner/runtime/database.h"
#include "derplanner/runtime/planning.h"
#include "unittestpp.h"

#if defined(_MSC_VER)
    #define PRIu64 "I64u"
    #define PRIi64 "I64i"
#else
    #define __STDC_FORMAT_MACROS
    #include <inttypes.h>
#endif

namespace {

enum { FORMAT_BUFFER_SIZE = 1024 };

void format(char* output, const char* format, ...)
{
    va_list arg_list;
    va_start(arg_list, format);
    vsnprintf(output, FORMAT_BUFFER_SIZE, format, arg_list);
    va_end(arg_list);
}

void check_plan(const char* expected, plnnr::Planning_State& pstate, const plnnr::Domain_Info* domain)
{
    char buffer[FORMAT_BUFFER_SIZE];

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
                {
                    plnnr::Id32 v = plnnr::as_Id32(task.arguments, layout, p);
                    format(buffer, "%%u", (uint32_t)(v));
                    actual += buffer;
                    break;
                }
            case plnnr::Type_Id64:
                {
                    plnnr::Id64 v = plnnr::as_Id64(task.arguments, layout, p);
                    format(buffer, "%%" PRIu64, (uint64_t)(v));
                    actual += buffer;
                    break;
                }
            case plnnr::Type_Int8:
                {
                    int8_t v = plnnr::as_Int8(task.arguments, layout, p);
                    format(buffer, "%%d", (int8_t)(v));
                    actual += buffer;
                    break;
                }
            case plnnr::Type_Int32:
                {
                    int32_t v = plnnr::as_Int32(task.arguments, layout, p);
                    format(buffer, "%%d", (int32_t)(v));
                    actual += buffer;
                    break;
                }
            case plnnr::Type_Int64:
                {
                    int64_t v = plnnr::as_Int64(task.arguments, layout, p);
                    format(buffer, "%%" PRIi64, (int64_t)(v));
                    actual += buffer;
                    break;
                }
            case plnnr::Type_Float:
                {
                    float v = plnnr::as_Float(task.arguments, layout, p);
                    format(buffer, "%%f", (float)(v));
                    actual += buffer;
                    break;
                }
            case plnnr::Type_Vec3:
                {
                    plnnr::Vec3 v = plnnr::as_Vec3(task.arguments, layout, p);
                    format(buffer, "vec3(%%f, %%f, %%f)", v.x, v.y, v.z);
                    actual += buffer;
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

TEST(%(test_case_name)s)
{
    plnnr::Memory_Default default_mem;

    %(domain_name)s_init_domain_info();
    const plnnr::Domain_Info* domain = %(domain_name)s_get_domain_info();

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

    %(database_source)s

    plnnr::Find_Plan_Status status = plnnr::find_plan(&pstate, &db, domain);
    CHECK_EQUAL(plnnr::Find_Plan_Succeeded, status);

    %(checks_source)s
}

}
'''

def make_test(domain_path):
    test_output_path = os.path.dirname(__file__)
    domain_name = os.path.splitext(os.path.basename(domain_path))[0]
    domain_name = domain_name.replace('-', '_')
    domain_source = open(domain_path, 'rt').read()

    exe = os.path.join('bin', 'x64', 'debug', 'derplannerc')
    p = subprocess.Popen(args=[exe, '-o', test_output_path, domain_path], stderr=subprocess.PIPE)
    (stdoutdata, stderrdata) = p.communicate()
    if p.returncode != 0:
        print "ERROR: failed to generate code for", domain_path
        print stderrdata
        return

    database_source_lines = []
    for line in domain_source.splitlines():
        if line.startswith('//:'):
            database_source_lines += [line.lstrip('//:').strip()]

    checks_source_lines = []
    for line in domain_source.splitlines():
        if line.startswith('//!'):
            checks_source_lines += [line.lstrip('//!').strip()]

    database_source = '\n'.join(database_source_lines)
    checks_source = '\n'.join(checks_source_lines)

    test_source = template % {
        'test_case_name': domain_name,
        'domain_header': domain_name + '.h',
        'domain_name': domain_name,
        'database_source': database_source,
        'checks_source': checks_source}

    with open(os.path.join(test_output_path, 'test_' + domain_name + '.cpp'), 'wb') as fd:
        fd.write(test_source)

def generate(base_path):
    runtime_domains = glob.glob(os.path.join(base_path, 'run-*.domain'))
    for domain_path in runtime_domains:
        make_test(domain_path)


if __name__ == '__main__':
    generate(base_path=os.path.join(os.path.dirname(__file__), 'cases'))
