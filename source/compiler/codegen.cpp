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

#include "derplanner/compiler/config.h"
#include "derplanner/compiler/assert.h"
#include "derplanner/compiler/io.h"
#include "derplanner/compiler/s_expression.h"
#include "derplanner/compiler/ast.h"
#include "formatter.h"
#include "tree_tools.h"
#include "ast_tools.h"
#include "codegen_header.h"
#include "codegen_source.h"
#include "codegen_precondition.h"
#include "codegen_branch.h"
#include "codegen_reflection.h"
#include "derplanner/compiler/codegen.h"

namespace plnnrc {

struct namespace_wrap
{
    namespace_wrap(ast::node* namespace_node, formatter& output, bool end_with_empty_line=true)
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

    ast::node* namespace_node;
    formatter& output;
    bool end_with_empty_line;

    namespace_wrap& operator=(const namespace_wrap&);
};

bool generate_header(ast::tree& ast, writer& writer, codegen_options options)
{
    formatter output(writer, options.tab, options.newline);

    if (!output.init(DERPLANNER_CODEGEN_OUTPUT_BUFFER_SIZE))
    {
        return false;
    }

    ast::node* worldstate = find_child(ast.root(), ast::node_worldstate);
    plnnrc_assert(worldstate);

    ast::node* domain = find_child(ast.root(), ast::node_domain);
    plnnrc_assert(domain);

    output.writeln("#ifndef %s", options.include_guard);
    output.writeln("#define %s", options.include_guard);
    output.newline();

    ast::node* worldstate_namespace = worldstate->first_child;
    plnnrc_assert(worldstate_namespace);
    plnnrc_assert(worldstate_namespace->type == ast::node_namespace);

    ast::node* domain_namespace = domain->first_child;
    plnnrc_assert(domain_namespace);
    plnnrc_assert(domain_namespace->type == ast::node_namespace);

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

    ast::node* worldstate = find_child(ast.root(), ast::node_worldstate);
    plnnrc_assert(worldstate);

    ast::node* domain = find_child(ast.root(), ast::node_domain);
    plnnrc_assert(domain);

    ast::node* worldstate_namespace = worldstate->first_child;
    plnnrc_assert(worldstate_namespace);
    plnnrc_assert(worldstate_namespace->type == ast::node_namespace);

    ast::node* domain_namespace = domain->first_child;
    plnnrc_assert(domain_namespace);
    plnnrc_assert(domain_namespace->type == ast::node_namespace);

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
