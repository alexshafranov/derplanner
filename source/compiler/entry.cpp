//
// Copyright (c) 2015 Alexander Shafranov shafranov@gmail.com
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

#include <algorithm>

#include "derplanner/compiler/array.h"
#include "derplanner/compiler/memory.h"
#include "derplanner/compiler/io.h"
#include "derplanner/compiler/errors.h"
#include "derplanner/compiler/ast.h"
#include "derplanner/compiler/lexer.h"
#include "derplanner/compiler/parser.h"
#include "derplanner/compiler/codegen.h"
#include "derplanner/compiler/entry.h"

using namespace plnnrc;

struct Error_Location_Compare
{
    bool operator()(const Error& lhs, const Error& rhs)
    {
        const uint64_t lhs_key = ((uint64_t)(lhs.loc.line) << 32) | lhs.loc.column;
        const uint64_t rhs_key = ((uint64_t)(rhs.loc.line) << 32) | rhs.loc.column;
        return lhs_key < rhs_key;
    }
};

struct Debug
{
    const Compiler_Config* config;
    ast::Root* tree;
    const char* input_buffer;

    Debug(const Compiler_Config* config, ast::Root* tree, const char* input_buffer)
        : config(config)
        , tree(tree)
        , input_buffer(input_buffer)
        {}

    ~Debug()
    {
        if (config->print_debug_info)
        {
            plnnrc::debug_output_tokens(input_buffer, config->debug_writer);
            plnnrc::debug_output_ast(tree, config->debug_writer);

            const size_t requested_size = tree->pool->get_total_requested();
            const size_t total_size = tree->pool->get_total_allocated();

            plnnrc::Formatter fmtr;
            plnnrc::init(fmtr, "\t", "\n", config->debug_writer);

            writeln(fmtr, "requested size = %d", uint32_t(requested_size));
            writeln(fmtr, "total size = %d", uint32_t(total_size));
        }
    }
};

bool plnnrc::compile(const Compiler_Config* config, const char* input_buffer)
{
    Memory_Stack_Scope scratch_scope(config->scratch_allocator);

    Array<Error> errors;
    init(errors, config->data_allocator, 16);

    Formatter error_frmtr;
    init(error_frmtr, "\t", "\n", config->diag_writer);

    ast::Root tree;
    Lexer lexer;
    Parser parser;
    Codegen codegen;

    init(&tree, &errors, config->data_allocator, config->scratch_allocator);
    init(&lexer, input_buffer, config->scratch_allocator);
    init(&parser, &lexer, &tree, &errors, config->scratch_allocator);
    init(&codegen, &tree, config->scratch_allocator);

    Debug debug(config, &tree, input_buffer);

    for (;;)
    {
        // build AST.
        parse(&parser);

        if (!empty(errors))
        {
            break;
        }

        // process AST.
        inline_macros(&tree);
        convert_to_dnf(&tree);
        annotate(&tree);

        if (!empty(errors))
        {
            break;
        }

        infer_types(&tree);

        if (!empty(errors))
        {
            break;
        }

        generate_header(&codegen, config->header_guard, config->header_writer);
        generate_source(&codegen, config->header_file_name, config->source_writer);

        return true;
    }

    if (size(errors) > 0)
    {
        std::sort(&errors[0], &errors[0] + size(errors), Error_Location_Compare());

        format_error(errors[0], error_frmtr);

        for (uint32_t i = 1; i < size(errors); ++i)
        {
            if (errors[i] != errors[i-1])
            {
                format_error(errors[i], error_frmtr);
            }
        }
    }

    return false;
}
