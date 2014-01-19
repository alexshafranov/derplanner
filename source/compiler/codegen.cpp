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

#include "formatter.h"
#include "tree_tools.h"
#include "ast_tools.h"
#include "derplanner/compiler/config.h"
#include "derplanner/compiler/assert.h"
#include "derplanner/compiler/io.h"
#include "derplanner/compiler/s_expression.h"
#include "derplanner/compiler/ast.h"
#include "codegen_precondition.h"
#include "codegen_branch.h"
#include "derplanner/compiler/codegen.h"

namespace plnnrc {

using namespace ast;

namespace
{
    struct namespace_wrap
    {
        namespace_wrap(node* namespace_node, formatter& output, bool end_with_empty_line=true)
            : namespace_node(namespace_node)
            , output(output)
            , end_with_empty_line(end_with_empty_line)
        {
            for (sexpr::node* name_expr = namespace_node->s_expr->first_child; name_expr != 0; name_expr = name_expr->next_sibling)
            {
                output.writeln("namespace %i {", name_expr->token);

                if (is_last(name_expr))
                {
                    output.newline();
                }
            }
        }

        namespace_wrap(const char* namespace_id, formatter& output, bool end_with_empty_line=true)
            : namespace_node(0)
            , output(output)
            , end_with_empty_line(end_with_empty_line)
        {
            output.writeln("namespace %s {", namespace_id);
            output.newline();
        }

        ~namespace_wrap()
        {
            if (namespace_node)
            {
                for (sexpr::node* name_expr = namespace_node->s_expr->first_child; name_expr != 0; name_expr = name_expr->next_sibling)
                {
                    output.writeln("}");
                }
            }
            else
            {
                output.writeln("}");
            }

            if (end_with_empty_line)
            {
                output.newline();
            }
        }

        node* namespace_node;
        formatter& output;
        bool end_with_empty_line;

        namespace_wrap& operator=(const namespace_wrap&);
    };

    class paste_fully_qualified_namespace : public paste_func
    {
    public:
        node* namespace_node;

        paste_fully_qualified_namespace(node* namespace_node)
            : namespace_node(namespace_node)
        {
        }

        virtual void operator()(formatter& output)
        {
            for (sexpr::node* name_expr = namespace_node->s_expr->first_child; name_expr != 0; name_expr = name_expr->next_sibling)
            {
                output.put_id(name_expr->token);

                if (!is_last(name_expr))
                {
                    output.put_str("::");
                }
            }
        }
    };

    class paste_function_parameters : public paste_func
    {
    public:
        node* function_atom;

        paste_function_parameters(node* function_atom)
            : function_atom(function_atom)
        {
        }

        virtual void operator()(formatter& output)
        {
            for (node* worldstate_type = function_atom->first_child; worldstate_type != 0; worldstate_type = worldstate_type->next_sibling)
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

    void generate_source_top(ast::tree& /*ast*/, node* /*worldstate_namespace*/, const char* header_file_name, formatter& output)
    {
        output.writeln("#include <derplanner/runtime/runtime.h>");
        output.writeln("#include \"%s\"", header_file_name);
        output.newline();

        output.writeln("using namespace plnnr;");
        output.newline();

        output.writeln("#pragma GCC diagnostic ignored \"-Wunused-variable\"");
        output.newline();
    }

    void generate_worldstate(tree& /*ast*/, node* worldstate, formatter& output)
    {
        plnnrc_assert(worldstate && worldstate->type == node_worldstate);

        output.writeln("enum atom_type");
        {
            class_scope s(output);

            for (node* atom = worldstate->first_child->next_sibling; atom != 0; atom = atom->next_sibling)
            {
                if (atom->type != node_atom)
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

            for (node* function_def = worldstate->first_child->next_sibling; function_def != 0; function_def = function_def->next_sibling)
            {
                if (function_def->type != node_function)
                {
                    continue;
                }

                node* function_atom = function_def->first_child;
                node* return_type = function_atom->next_sibling;

                paste_function_parameters paste(function_atom);

                output.writeln("%s (*%i)(%p);", return_type->s_expr->first_child->token, function_atom->s_expr->token, &paste);
            }
        }

        for (node* atom = worldstate->first_child->next_sibling; atom != 0; atom = atom->next_sibling)
        {
            if (atom->type != node_atom)
            {
                continue;
            }

            output.writeln("struct %i_tuple", atom->s_expr->token);
            {
                class_scope s(output);

                unsigned param_index = 0;

                for (node* param = atom->first_child; param != 0; param = param->next_sibling)
                {
                    output.writeln("%s _%d;", param->s_expr->first_child->token, param_index++);
                }

                output.writeln("%i_tuple* next;", atom->s_expr->token);
                output.writeln("%i_tuple* prev;", atom->s_expr->token);
                output.writeln("enum { id = atom_%i };", atom->s_expr->token);
            }
        }
    }

    void generate_task_type_enum(tree& ast, node* domain, formatter& output)
    {
        output.writeln("enum task_type");
        {
            class_scope s(output);

            for (id_table_values operators = ast.operators.values(); !operators.empty(); operators.pop())
            {
                node* operatr = operators.value();
                node* operator_atom = operatr->first_child;

                output.writeln("task_%i,", operator_atom->s_expr->token);
            }

            for (node* method = domain->first_child; method != 0; method = method->next_sibling)
            {
                if (method->type != node_method)
                {
                    continue;
                }

                node* method_atom = method->first_child;
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

    void generate_param_struct(tree& ast, node* task, formatter& output)
    {
        node* atom = task->first_child;

        if (!atom->first_child)
        {
            return;
        }

        output.writeln("struct %i_args", atom->s_expr->token);
        {
            class_scope s(output);

            for (node* param = atom->first_child; param != 0; param = param->next_sibling)
            {
                node* ws_type = ast.type_tag_to_node[type_tag(param)];
                output.writeln("%s _%d;", ws_type->s_expr->first_child->token, annotation<term_ann>(param)->var_index);
            }
        }

        output.writeln("inline bool operator==(const %i_args& a, const %i_args& b)", atom->s_expr->token, atom->s_expr->token);
        {
            scope s(output, true);
            int param_index = 0;

            output.writeln("return \\");
            {
                indented s(output);

                for (node* param = atom->first_child; param != 0; param = param->next_sibling)
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

    void generate_param_structs(tree& ast, node* domain, formatter& output)
    {
        for (id_table_values operators = ast.operators.values(); !operators.empty(); operators.pop())
        {
            generate_param_struct(ast, operators.value(), output);
        }

        for (node* method = domain->first_child; method != 0; method = method->next_sibling)
        {
            if (method->type != node_method)
            {
                continue;
            }

            generate_param_struct(ast, method, output);
        }
    }

    void generate_forward_decls(tree& ast, node* domain, formatter& output)
    {
        for (node* method = domain->first_child; method != 0; method = method->next_sibling)
        {
            if (method->type != node_method)
            {
                continue;
            }

            node* atom = method->first_child;
            const char* method_name = atom->s_expr->token;

            unsigned branch_index = 0;

            for (node* branch = method->first_child->next_sibling; branch != 0; branch = branch->next_sibling)
            {
                plnnrc_assert(branch->type == node_branch);
                output.writeln("bool %i_branch_%d_expand(plnnr::method_instance*, plnnr::planner_state&, void*);", method_name, branch_index);
                ++branch_index;
            }
        }

        if (ast.methods.count() > 0)
        {
            output.newline();
        }
    }

    void generate_task_name_function(ast::tree& ast, node* domain, bool enabled, formatter& output)
    {
        if (!enabled)
        {
            output.writeln("const char* task_name(task_type type) { return \"<none>\"; }");
            output.newline();
        }
        else
        {
            output.writeln("static const char* task_type_to_name[] =");
            {
                class_scope s(output);

                for (id_table_values operators = ast.operators.values(); !operators.empty(); operators.pop())
                {
                    node* operatr = operators.value();
                    node* operator_atom = operatr->first_child;
                    output.writeln("\"%s\",", operator_atom->s_expr->token);
                }

                for (node* method = domain->first_child; method != 0; method = method->next_sibling)
                {
                    if (method->type != node_method)
                    {
                        continue;
                    }

                    node* method_atom = method->first_child;
                    plnnrc_assert(method_atom);

                    output.writeln("\"%s\",", method_atom->s_expr->token);
                }

                output.writeln("\"<none>\",");
            }

            output.writeln("const char* task_name(task_type type) { return task_type_to_name[type]; }");
            output.newline();
        }
    }

    void generate_atom_name_function(ast::tree& /*ast*/, node* worldstate, bool enabled, formatter& output)
    {
        if (!enabled)
        {
            output.writeln("const char* atom_name(atom_type type) { return \"<none>\"; }");
            output.newline();
        }
        else
        {
            output.writeln("static const char* atom_type_to_name[] =");
            {
                class_scope s(output);

                for (node* atom = worldstate->first_child->next_sibling; atom != 0; atom = atom->next_sibling)
                {
                    if (atom->type != node_atom)
                    {
                        continue;
                    }

                    output.writeln("\"%s\",", atom->s_expr->token);
                }

                output.writeln("\"<none>\",");
            }

            output.writeln("const char* atom_name(atom_type type) { return atom_type_to_name[type]; }");
            output.newline();
        }
    }

    void generate_atom_reflector(ast::tree& /*ast*/, node* atom, paste_func* paste_namespace, const char* name_function, const char* tuple_struct_postfix, const char* tuple_id_prefix, formatter& output)
    {
        output.writeln("template <typename V>");
        output.writeln("struct generated_type_reflector<%p::%i_%s, V>", paste_namespace, atom->s_expr->token, tuple_struct_postfix);
        {
            class_scope s(output);

            output.writeln("void operator()(const %p::%i_%s& tuple, V& visitor)", paste_namespace, atom->s_expr->token, tuple_struct_postfix);
            {
                scope s(output, false);

                int param_count = 0;

                for (node* child = atom->first_child; child != 0; child = child->next_sibling)
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

    void generate_reflectors(ast::tree& ast, node* worldstate, node* domain, formatter& output)
    {
        node* worldstate_namespace = worldstate->first_child;
        plnnrc_assert(worldstate_namespace);
        plnnrc_assert(worldstate_namespace->type == node_namespace);

        node* domain_namespace = domain->first_child;
        plnnrc_assert(domain_namespace);
        plnnrc_assert(domain_namespace->type == node_namespace);

        paste_fully_qualified_namespace paste_world_namespace(worldstate_namespace);
        paste_fully_qualified_namespace paste_domain_namespace(domain_namespace);

        output.writeln("template <typename V>");
        output.writeln("struct generated_type_reflector<%p::worldstate, V>", &paste_world_namespace);
        {
            class_scope s(output);

            output.writeln("void operator()(const %p::worldstate& world, V& visitor)", &paste_world_namespace);
            {
                scope s(output, false);

                for (node* atom = worldstate_namespace->next_sibling; atom != 0; atom = atom->next_sibling)
                {
                    if (atom->type != node_atom)
                    {
                        continue;
                    }

                    output.writeln("PLNNR_GENCODE_VISIT_ATOM_LIST(%p, atom_%i, %i_tuple, visitor);", &paste_world_namespace, atom->s_expr->token, atom->s_expr->token);
                }
            }
        }

        for (node* atom = worldstate_namespace->next_sibling; atom != 0; atom = atom->next_sibling)
        {
            if (atom->type != node_atom)
            {
                continue;
            }

            generate_atom_reflector(ast, atom, &paste_world_namespace, "atom_name", "tuple", "atom", output);
        }

        for (id_table_values operators = ast.operators.values(); !operators.empty(); operators.pop())
        {
            node* atom = operators.value()->first_child;

            if (!atom->first_child)
            {
                continue;
            }

            generate_atom_reflector(ast, atom, &paste_domain_namespace, "task_name", "args", "task", output);
        }

        for (node* method = domain->first_child; method != 0; method = method->next_sibling)
        {
            if (method->type != node_method)
            {
                continue;
            }

            node* atom = method->first_child;

            if (!atom->first_child)
            {
                continue;
            }

            generate_atom_reflector(ast, atom, &paste_domain_namespace, "task_name", "args", "task", output);
        }
    }

    void generate_task_type_dispatcher(ast::tree& ast, node* domain, formatter& output)
    {
        node* domain_namespace = domain->first_child;
        plnnrc_assert(domain_namespace);
        plnnrc_assert(domain_namespace->type == node_namespace);

        paste_fully_qualified_namespace paste_namespace(domain_namespace);

        output.writeln("template <typename V>");
        output.writeln("struct task_type_dispatcher<%p::task_type, V>", &paste_namespace);
        {
            class_scope s(output);
            output.writeln("void operator()(const %p::task_type& task_type, void* args, V& visitor)", &paste_namespace);
            {
                scope s(output, false);
                output.writeln("switch (task_type)");
                {
                    scope s(output, false);

                    for (node* method = domain->first_child; method != 0; method = method->next_sibling)
                    {
                        if (method->type != node_method)
                        {
                            continue;
                        }

                        node* method_atom = method->first_child;
                        plnnrc_assert(method_atom);

                        output.writeln("case %p::task_%i:", &paste_namespace, method_atom->s_expr->token);
                        {
                            indented s(output);

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

                    for (id_table_values operators = ast.operators.values(); !operators.empty(); operators.pop())
                    {
                        node* operatr = operators.value();
                        node* operator_atom = operatr->first_child;

                        output.writeln("case %p::task_%i:", &paste_namespace, operator_atom->s_expr->token);
                        {
                            indented s(output);

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
                        indented s(output);
                        output.writeln("plnnr_assert(false);");
                        output.writeln("break;");
                    }
                }
            }
        }
    }
}

bool generate_header(ast::tree& ast, writer& writer, codegen_options options)
{
    formatter output(writer, options.tab, options.newline);

    if (!output.init(DERPLANNER_CODEGEN_OUTPUT_BUFFER_SIZE))
    {
        return false;
    }

    node* worldstate = find_child(ast.root(), node_worldstate);
    plnnrc_assert(worldstate);

    node* domain = find_child(ast.root(), node_domain);
    plnnrc_assert(domain);

    output.writeln("#ifndef %s", options.include_guard);
    output.writeln("#define %s", options.include_guard);
    output.newline();

    node* worldstate_namespace = worldstate->first_child;
    plnnrc_assert(worldstate_namespace);
    plnnrc_assert(worldstate_namespace->type == node_namespace);

    node* domain_namespace = domain->first_child;
    plnnrc_assert(domain_namespace);
    plnnrc_assert(domain_namespace->type == node_namespace);

    generate_header_top(ast, options.custom_header, output);

    {
        namespace_wrap wrap(worldstate_namespace, output);
        generate_worldstate(ast, worldstate, output);
    }

    {
        namespace_wrap wrap(domain_namespace, output);
        generate_task_type_enum(ast, domain, output);
        generate_param_structs(ast, domain, output);
        generate_forward_decls(ast, domain, output);
    }

    if (options.enable_reflection)
    {
        namespace_wrap wrap("plnnr", output);
        generate_reflectors(ast, worldstate, domain, output);
        generate_task_type_dispatcher(ast, domain, output);
    }

    output.writeln("#endif");

    return true;
}

bool generate_source(ast::tree& ast, writer& writer, codegen_options options)
{
    formatter output(writer, options.tab, options.newline);

    if (!output.init(DERPLANNER_CODEGEN_OUTPUT_BUFFER_SIZE))
    {
        return false;
    }

    node* worldstate = find_child(ast.root(), node_worldstate);
    plnnrc_assert(worldstate);

    node* domain = find_child(ast.root(), node_domain);
    plnnrc_assert(domain);

    node* worldstate_namespace = worldstate->first_child;
    plnnrc_assert(worldstate_namespace);
    plnnrc_assert(worldstate_namespace->type == node_namespace);

    node* domain_namespace = domain->first_child;
    plnnrc_assert(domain_namespace);
    plnnrc_assert(domain_namespace->type == node_namespace);

    generate_source_top(ast, worldstate_namespace, options.header_file_name, output);

    {
        namespace_wrap wrap(domain_namespace, output, false);
        generate_task_name_function(ast, domain, options.runtime_task_names, output);
        generate_atom_name_function(ast, worldstate, options.runtime_atom_names, output);
        generate_preconditions(ast, domain, output);
        generate_branch_expands(ast, domain, output);
    }

    return true;
}

}
