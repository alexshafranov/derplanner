//
// Copyright (c) 2013 Alexander Shafranov shafranov@gmail.com
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#include "derplanner/compiler/assert.h"
#include "derplanner/compiler/s_expression.h"
#include "derplanner/compiler/ast.h"
#include "formatter.h"
#include "codegen_source.h"

namespace plnnrc {

void generate_source_top(const char* header_file_name, Formatter& output)
{
    output.writeln("#include \"derplanner/runtime/runtime.h\"");
    output.writeln("#include \"%s\"", header_file_name);
    output.newline();

    output.writeln("using namespace plnnr;");
    output.newline();

    output.writeln("#ifdef __GNUC__");
    output.writeln("#pragma GCC diagnostic ignored \"-Wunused-parameter\"");
    output.writeln("#pragma GCC diagnostic ignored \"-Wunused-variable\"");
    output.writeln("#endif");
    output.newline();

    output.writeln("#ifdef _MSC_VER");
    output.writeln("#pragma warning(disable: 4100) // unreferenced formal parameter");
    output.writeln("#pragma warning(disable: 4189) // local variable is initialized but not referenced");
    output.writeln("#endif");
    output.newline();

    output.writeln("#define PLNNR_COROUTINE_BEGIN(state) switch ((state).stage) { case 0:");
    output.writeln("#define PLNNR_COROUTINE_YIELD(state, label) (state).stage = label; return true; case label:;");
    output.writeln("#define PLNNR_COROUTINE_END() } return false");
    output.newline();
}

void generate_task_name_function(ast::Tree& ast, ast::Node* domain, bool enabled, Formatter& output)
{
    if (!enabled)
    {
        output.writeln("const char* task_name(Task_Type type) { return \"<none>\"; }");
        output.newline();
    }
    else
    {
        output.writeln("static const char* task_type_to_name[] =");
        {
            Class_Scope s(output);

            for (Id_Table_Values operators = ast.operators.values(); !operators.empty(); operators.pop())
            {
                ast::Node* operatr = operators.value();
                ast::Node* operator_atom = operatr->first_child;
                output.writeln("\"%s\",", operator_atom->s_expr->token);
            }

            for (ast::Node* method = domain->first_child; method != 0; method = method->next_sibling)
            {
                if (!ast::is_method(method))
                {
                    continue;
                }

                ast::Node* method_atom = method->first_child;
                plnnrc_assert(method_atom);

                output.writeln("\"%s\",", method_atom->s_expr->token);
            }

            output.writeln("\"<none>\",");
        }

        output.writeln("const char* task_name(Task_Type type) { return task_type_to_name[type]; }");
        output.newline();
    }
}

void generate_atom_name_function(ast::Tree& /*ast*/, ast::Node* worldstate, bool enabled, Formatter& output)
{
    if (!enabled)
    {
        output.writeln("const char* atom_name(Atom_Type type) { return \"<none>\"; }");
        output.newline();
    }
    else
    {
        output.writeln("static const char* atom_type_to_name[] =");
        {
            Class_Scope s(output);

            for (ast::Node* atom = worldstate->first_child->next_sibling; atom != 0; atom = atom->next_sibling)
            {
                if (!ast::is_atom(atom))
                {
                    continue;
                }

                output.writeln("\"%s\",", atom->s_expr->token);
            }

            output.writeln("\"<none>\",");
        }

        output.writeln("const char* atom_name(Atom_Type type) { return atom_type_to_name[type]; }");
        output.newline();
    }
}

}
