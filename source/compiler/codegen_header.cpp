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
#include "tree_tools.h"
#include "ast_tools.h"
#include "formatter.h"
#include "codegen_header.h"

namespace plnnrc {

class paste_function_parameters : public paste_func
{
public:
    ast::node* function_atom;

    paste_function_parameters(ast::node* function_atom)
        : function_atom(function_atom)
    {
    }

    virtual void operator()(formatter& output)
    {
        for (ast::node* worldstate_type = function_atom->first_child; worldstate_type != 0; worldstate_type = worldstate_type->next_sibling)
        {
            output.put_str(worldstate_type->s_expr->first_child->token);

            if (!is_last(worldstate_type))
            {
                output.put_str(", ");
            }
        }
    }
};

void generate_header_top(ast::tree& /*ast*/, const char* custom_header, formatter& output)
{
    output.writeln("#include <derplanner/runtime/interface.h>");
    output.newline();

    if (custom_header)
    {
        output.writeln("#include \"%s\"", custom_header);
        output.newline();
    }

    output.writeln("namespace plnnr");
    {
        scope s(output);
        output.writeln("namespace tuple_list");
        {
            scope s(output, false);
            output.writeln("struct handle;");
        }
    }

    output.writeln("namespace plnnr");
    {
        scope s(output);
        output.writeln("struct planner_state;");
        output.writeln("struct method_instance;");
    }
}

void generate_worldstate(ast::tree& /*ast*/, ast::node* worldstate, formatter& output)
{
    plnnrc_assert(worldstate && ast::is_worldstate(worldstate));

    output.writeln("enum atom_type");
    {
        class_scope s(output);

        for (ast::node* atom = worldstate->first_child->next_sibling; atom != 0; atom = atom->next_sibling)
        {
            if (!ast::is_atom(atom))
            {
                continue;
            }

            output.writeln("atom_%i,", atom->s_expr->token);
        }

        output.writeln("atom_count,");
    }

    output.writeln("const char* atom_name(atom_type type);");
    output.newline();

    output.writeln("struct worldstate");
    {
        class_scope s(output);
        output.writeln("plnnr::tuple_list::handle* atoms[atom_count];");

        for (ast::node* function_def = worldstate->first_child->next_sibling; function_def != 0; function_def = function_def->next_sibling)
        {
            if (!ast::is_function(function_def))
            {
                continue;
            }

            ast::node* function_atom = function_def->first_child;
            ast::node* return_type = function_atom->next_sibling;

            paste_function_parameters paste(function_atom);

            output.writeln("%s (*%i)(%p);", return_type->s_expr->first_child->token, function_atom->s_expr->token, &paste);
        }
    }

    for (ast::node* atom = worldstate->first_child->next_sibling; atom != 0; atom = atom->next_sibling)
    {
        if (!ast::is_atom(atom))
        {
            continue;
        }

        output.writeln("struct %i_tuple", atom->s_expr->token);
        {
            class_scope s(output);

            unsigned param_index = 0;

            for (ast::node* param = atom->first_child; param != 0; param = param->next_sibling)
            {
                output.writeln("%s _%d;", param->s_expr->first_child->token, param_index++);
            }

            output.writeln("%i_tuple* next;", atom->s_expr->token);
            output.writeln("%i_tuple* prev;", atom->s_expr->token);
            output.writeln("enum { id = atom_%i };", atom->s_expr->token);
        }
    }
}

void generate_task_type_enum(ast::tree& ast, ast::node* domain, formatter& output)
{
    output.writeln("enum task_type");
    {
        class_scope s(output);

        for (id_table_values operators = ast.operators.values(); !operators.empty(); operators.pop())
        {
            ast::node* operatr = operators.value();
            ast::node* operator_atom = operatr->first_child;

            output.writeln("task_%i,", operator_atom->s_expr->token);
        }

        for (ast::node* method = domain->first_child; method != 0; method = method->next_sibling)
        {
            if (!ast::is_method(method))
            {
                continue;
            }

            ast::node* method_atom = method->first_child;
            plnnrc_assert(method_atom);

            output.writeln("task_%i,", method_atom->s_expr->token);
        }

        output.writeln("task_count,");
    }

    output.writeln("static const int operator_count = %d;", ast.operators.count());
    output.writeln("static const int method_count = %d;", ast.methods.count());
    output.newline();

    output.writeln("const char* task_name(task_type type);");
    output.newline();
}

void generate_param_structs(ast::tree& ast, ast::node* domain, formatter& output)
{
    for (id_table_values operators = ast.operators.values(); !operators.empty(); operators.pop())
    {
        generate_param_struct(ast, operators.value(), output);
    }

    for (ast::node* method = domain->first_child; method != 0; method = method->next_sibling)
    {
        if (!ast::is_method(method))
        {
            continue;
        }

        generate_param_struct(ast, method, output);
    }
}

void generate_param_struct(ast::tree& ast, ast::node* task, formatter& output)
{
    ast::node* atom = task->first_child;

    if (!atom->first_child)
    {
        return;
    }

    output.writeln("struct %i_args", atom->s_expr->token);
    {
        class_scope s(output);

        for (ast::node* param = atom->first_child; param != 0; param = param->next_sibling)
        {
            ast::node* ws_type = ast.type_tag_to_node[type_tag(param)];
            output.writeln("%s _%d;", ws_type->s_expr->first_child->token, ast::annotation<ast::term_ann>(param)->var_index);
        }
    }

    output.writeln("inline bool operator==(const %i_args& a, const %i_args& b)", atom->s_expr->token, atom->s_expr->token);
    {
        scope s(output, true);
        int param_index = 0;

        output.writeln("return \\");
        {
            indented s(output);

            for (ast::node* param = atom->first_child; param != 0; param = param->next_sibling)
            {
                if (!is_last(param))
                {
                    output.writeln("a._%d == b._%d &&", param_index, param_index);
                }
                else
                {
                    output.writeln("a._%d == b._%d ;", param_index, param_index);
                }

                param_index++;
            }
        }
    }
}

void generate_forward_decls(ast::tree& ast, ast::node* domain, formatter& output)
{
    for (ast::node* method = domain->first_child; method != 0; method = method->next_sibling)
    {
        if (!ast::is_method(method))
        {
            continue;
        }

        ast::node* atom = method->first_child;
        const char* method_name = atom->s_expr->token;

        unsigned branch_index = 0;

        for (ast::node* branch = method->first_child->next_sibling; branch != 0; branch = branch->next_sibling)
        {
            plnnrc_assert(ast::is_branch(branch));
            output.writeln("bool %i_branch_%d_expand(plnnr::method_instance*, plnnr::planner_state&, void*);", method_name, branch_index);
            ++branch_index;
        }
    }

    if (ast.methods.count() > 0)
    {
        output.newline();
    }
}

}
