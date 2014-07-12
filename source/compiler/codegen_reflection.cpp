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
#include "formatter.h"
#include "codegen_reflection.h"

namespace plnnrc {

class Paste_Fully_Qualified_Namespace : public Paste_Func
{
public:
    ast::Node* namespace_node;

    Paste_Fully_Qualified_Namespace(ast::Node* namespace_node)
        : namespace_node(namespace_node)
    {
    }

    virtual void operator()(Formatter& output)
    {
        for (sexpr::Node* name_expr = namespace_node->s_expr->first_child; name_expr != 0; name_expr = name_expr->next_sibling)
        {
            output.put_id(name_expr->token);

            if (!is_last(name_expr))
            {
                output.put_str("::");
            }
        }
    }
};

void generate_worldstate_reflectors(ast::Tree& ast, ast::Node* worldstate, Formatter& output)
{
    ast::Node* worldstate_namespace = worldstate->first_child;
    plnnrc_assert(worldstate_namespace && ast::is_namespace(worldstate_namespace));

    Paste_Fully_Qualified_Namespace paste_world_namespace(worldstate_namespace);

    output.writeln("template <typename V>");
    output.writeln("struct Generated_Type_Reflector<%p::Worldstate, V>", &paste_world_namespace);
    {
        Class_Scope s(output);

        output.writeln("void operator()(const %p::Worldstate& world, V& visitor)", &paste_world_namespace);
        {
            Scope s(output, false);

            for (ast::Node* atom = worldstate_namespace->next_sibling; atom != 0; atom = atom->next_sibling)
            {
                if (!ast::is_atom(atom))
                {
                    continue;
                }

                output.writeln("PLNNR_GENCODE_VISIT_ATOM_LIST(%p, atom_%i, %i_tuple, visitor);", &paste_world_namespace, atom->s_expr->token, atom->s_expr->token);
            }
        }
    }

    for (ast::Node* atom = worldstate_namespace->next_sibling; atom != 0; atom = atom->next_sibling)
    {
        if (!ast::is_atom(atom))
        {
            continue;
        }

        generate_atom_reflector(ast, atom, &paste_world_namespace, "atom_name", "tuple", "atom", output);
    }
}

void generate_domain_reflectors(ast::Tree& ast, ast::Node* domain, Formatter& output)
{
    ast::Node* domain_namespace = domain->first_child;
    plnnrc_assert(domain_namespace && ast::is_namespace(domain_namespace));

    Paste_Fully_Qualified_Namespace paste_domain_namespace(domain_namespace);

    for (Id_Table_Values operators = ast.operators.values(); !operators.empty(); operators.pop())
    {
        ast::Node* atom = operators.value()->first_child;

        if (!atom->first_child)
        {
            continue;
        }

        generate_atom_reflector(ast, atom, &paste_domain_namespace, "task_name", "args", "task", output);
    }

    for (ast::Node* method = domain->first_child; method != 0; method = method->next_sibling)
    {
        if (!ast::is_method(method))
        {
            continue;
        }

        ast::Node* atom = method->first_child;

        if (!atom->first_child)
        {
            continue;
        }

        generate_atom_reflector(ast, atom, &paste_domain_namespace, "task_name", "args", "task", output);
    }
}

void generate_atom_reflector(ast::Tree& /*ast*/, ast::Node* atom, Paste_Func* paste_namespace, const char* name_function, const char* tuple_struct_postfix, const char* tuple_id_prefix, Formatter& output)
{
    output.writeln("template <typename V>");
    output.writeln("struct Generated_Type_Reflector<%p::%i_%s, V>", paste_namespace, atom->s_expr->token, tuple_struct_postfix);
    {
        Class_Scope s(output);

        output.writeln("void operator()(const %p::%i_%s& tuple, V& visitor)", paste_namespace, atom->s_expr->token, tuple_struct_postfix);
        {
            Scope s(output, false);

            int param_count = 0;

            for (ast::Node* child = atom->first_child; child != 0; child = child->next_sibling)
            {
                param_count++;
            }

            output.writeln("PLNNR_GENCODE_VISIT_TUPLE_BEGIN(visitor, %p, %s, %s_%i, %d);", paste_namespace, name_function, tuple_id_prefix, atom->s_expr->token, param_count);

            for (int i = 0; i < param_count; ++i)
            {
                output.writeln("PLNNR_GENCODE_VISIT_TUPLE_ELEMENT(visitor, tuple, %d);", i);
            }

            output.writeln("PLNNR_GENCODE_VISIT_TUPLE_END(visitor, %p, %s, %s_%i, %d);", paste_namespace, name_function, tuple_id_prefix, atom->s_expr->token, param_count);
        }
    }
}

void generate_task_type_dispatcher(ast::Tree& ast, ast::Node* domain, Formatter& output)
{
    ast::Node* domain_namespace = domain->first_child;
    plnnrc_assert(domain_namespace && ast::is_namespace(domain_namespace));

    Paste_Fully_Qualified_Namespace paste_namespace(domain_namespace);

    output.writeln("template <typename V>");
    output.writeln("struct Task_Type_Dispatcher<%p::Task_Type, V>", &paste_namespace);
    {
        Class_Scope s(output);
        output.writeln("void operator()(const %p::Task_Type& task_type, void* args, V& visitor)", &paste_namespace);
        {
            Scope s(output, false);
            output.writeln("switch (task_type)");
            {
                Scope s(output, false);

                for (ast::Node* method = domain->first_child; method != 0; method = method->next_sibling)
                {
                    if (!ast::is_method(method))
                    {
                        continue;
                    }

                    ast::Node* method_atom = method->first_child;
                    plnnrc_assert(method_atom);

                    output.writeln("case %p::task_%i:", &paste_namespace, method_atom->s_expr->token);
                    {
                        Indented s(output);

                        const char* atom_id = method_atom->s_expr->token;

                        if (method_atom->first_child)
                        {
                            output.writeln("PLNNR_GENCODE_VISIT_TASK_WITH_ARGS(visitor, %p, task_%i, %i_args);", &paste_namespace, atom_id, atom_id);
                        }
                        else
                        {
                            output.writeln("PLNNR_GENCODE_VISIT_TASK_NO_ARGS(visitor, %p, task_%i);", &paste_namespace, atom_id);
                        }

                        output.writeln("break;");
                    }
                }

                for (Id_Table_Values operators = ast.operators.values(); !operators.empty(); operators.pop())
                {
                    ast::Node* operatr = operators.value();
                    ast::Node* operator_atom = operatr->first_child;

                    output.writeln("case %p::task_%i:", &paste_namespace, operator_atom->s_expr->token);
                    {
                        Indented s(output);

                        const char* atom_id = operator_atom->s_expr->token;

                        if (operator_atom->first_child)
                        {
                            output.writeln("PLNNR_GENCODE_VISIT_TASK_WITH_ARGS(visitor, %p, task_%i, %i_args);", &paste_namespace, atom_id, atom_id);
                        }
                        else
                        {
                            output.writeln("PLNNR_GENCODE_VISIT_TASK_NO_ARGS(visitor, %p, task_%i);", &paste_namespace, atom_id);
                        }

                        output.writeln("break;");
                    }
                }

                output.writeln("default:");
                {
                    Indented s(output);
                    output.writeln("plnnr_assert(false);");
                    output.writeln("break;");
                }
            }
        }
    }
}

}
