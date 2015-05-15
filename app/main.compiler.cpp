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

#include <string>
#include <algorithm>
#include <stdio.h>
#include <string.h>

#include "derplanner/compiler/array.h"
#include "derplanner/compiler/io.h"
#include "derplanner/compiler/memory.h"
#include "derplanner/compiler/lexer.h"
#include "derplanner/compiler/ast.h"
#include "derplanner/compiler/parser.h"
#include "derplanner/compiler/codegen.h"
#include "derplanner/compiler/errors.h"

using namespace plnnrc;

struct File_Context
{
    File_Context(const char* path, const char* mode)
        : fd(0)
    {
        fd = fopen(path, mode);
    }

    ~File_Context()
    {
        if (fd != 0)
        {
            fclose(fd);
        }
    }

    FILE* fd;
};

size_t file_size(const char* path)
{
    File_Context ctx(path, "rb");
    fseek(ctx.fd, 0, SEEK_END);
    size_t input_size = ftell(ctx.fd);
    fseek(ctx.fd, 0, SEEK_SET);
    return input_size;
}

void print_help()
{
    fprintf(stderr,
"Usage: derplannerc [options] <domain-file-path>\n"
"Options:\n"
"   --out, -o <dir>\n"
"       Set the directory for generated files.\n"
"       (default: current directory)\n"
"\n"
"   --custom-header, -c <header-name>\n"
"       Custom header.\n"
"\n"
"   --help, -h\n"
"       Print this help message and exit.\n"
"\n"
"   --debug, -d\n"
"       Outputs lexer and parser debugging info.\n"
"\n");
}

bool parse_argument(const char* argument, std::string& name, std::string& value)
{
    // [value]
    if (argument[0] != '-')
    {
        value = std::string(argument);
        return false;
    }

    // [--long-option=value] or [--long-option value]
    if (argument[1] == '-')
    {
        const char* begin = argument + 2;
        const char* end = strchr(argument, '=');

        if (end)
        {
            name = std::string(begin, end - begin);
            value = std::string(end + 1);
            return false;
        }

        name = std::string(begin);
        return true;
    }

    // [-s value]
    name = std::string(argument + 1);
    return true;
}

std::string normalize(std::string path)
{
    for (size_t i = 0; i < path.length(); ++i)
    {
        if (path[i] == '\\')
        {
            path[i] = '/';
        }
    }

    return path;
}

std::string get_output_name(std::string path)
{
    std::string::size_type s = path.rfind("/");

    if (s != std::string::npos)
    {
        path = path.substr(s + 1);
    }

    std::string::size_type d = path.rfind(".");

    if (d != std::string::npos)
    {
        path = path.substr(0, d);
    }

    return path;
}

int main(int argc, char** argv)
{
    std::string output_dir;
    std::string custom_header;
    std::string input_path;
    bool enable_debug_info = false;

    for (int i = 1; i < argc; ++i)
    {
        const char* argument = argv[i];

        if (!argument[0])
        {
            fprintf(stderr, "error: received empty cmdline argument!\n");
            return 1;
        }

        std::string name;
        std::string value;

        bool expecting_value = parse_argument(argument, name, value);

        if (expecting_value)
        {
            if (name == "h" || name == "help")
            {
                print_help();
                return 1;
            }

            if (name == "d" || name == "debug")
            {
                enable_debug_info = true;
                continue;
            }

            if (i + 1 >= argc || argv[i + 1][0] == '-')
            {
                fprintf(stderr, "error: missing value for flag: %s\n", name.c_str());
                return 1;
            }

            value = std::string(argv[++i]);
        }

        if (!name.empty())
        {
            if (name == "out" || name == "o")
            {
                if (!output_dir.empty())
                {
                    fprintf(stderr, "error: multiple values for flag: %s\n", name.c_str());
                    return 1;
                }

                output_dir = value;
                continue;
            }

            if (name == "custom-header" || name == "c")
            {
                if (!custom_header.empty())
                {
                    fprintf(stderr, "error: multiple values for flag: %s\n", name.c_str());
                    return 1;
                }

                custom_header = value;
                continue;
            }
        }
        else
        {
            if (!input_path.empty())
            {
                fprintf(stderr, "error: multiple inputs specified.\n");
                return 1;
            }

            input_path = value;
        }
    }

    if (input_path.empty())
    {
        fprintf(stderr, "error: no source file specified.\n");
        return 1;
    }

    {
        File_Context ctx(input_path.c_str(), "rb");

        if (!ctx.fd)
        {
            fprintf(stderr, "error: can't open input file: '%s'.\n", input_path.c_str());
            return 1;
        }
    }

    if (output_dir.empty())
    {
        output_dir = ".";
    }

    std::string output_name = get_output_name(normalize(input_path));

    plnnrc::Memory_Stack_Context mem_ctx_ast(32 * 1024);
    plnnrc::Memory_Stack_Context mem_ctx_scratch(32 * 1024);

    plnnrc::Memory_Stack* mem_ast = mem_ctx_ast.mem;
    plnnrc::Memory_Stack* mem_scratch = mem_ctx_scratch.mem;

    size_t input_size = file_size(input_path.c_str());
    char*  input_buffer = static_cast<char*>(mem_scratch->allocate(input_size + 1));
    {
        File_Context ctx(input_path.c_str(), "rb");
        size_t rb = fread(input_buffer, sizeof(char), input_size, ctx.fd);
        (void)rb;
    }

    input_buffer[input_size] = 0;

    {
        Memory_Stack_Scope scratch_scope(mem_scratch);

        plnnrc::Writer_Crt error_writer = plnnrc::make_stderr_writer();
        plnnrc::Formatter error_frmtr;
        plnnrc::init(error_frmtr, "\t", "\n", &error_writer);

        plnnrc::ast::Root tree;
        plnnrc::init(tree, mem_ast, mem_scratch);

        plnnrc::Lexer lexer;
        plnnrc::init(lexer, input_buffer, mem_scratch);

        plnnrc::Parser parser;
        plnnrc::init(parser, &lexer, &tree, mem_scratch);

        // build AST.
        {
            plnnrc::parse(parser);

            for (uint32_t i = 0; i < plnnrc::size(parser.errs); ++i)
            {
                plnnrc::format_error(parser.errs[i], error_frmtr);
            }

            if (!plnnrc::empty(parser.errs))
            {
                return 1;
            }
        }

        // process AST.
        {
            plnnrc::inline_predicates(tree);
            plnnrc::convert_to_dnf(tree);
            plnnrc::annotate(tree);
            plnnrc::infer_types(tree);
        }

        // generate source code.
        {
            std::string header_name = output_name + ".h";
            File_Context header_context((output_dir + "/" + header_name).c_str(), "wb");
            plnnrc::Writer_Crt header_writer(header_context.fd);

            File_Context source_context((output_dir + "/" + output_name + ".cpp").c_str(), "wb");
            plnnrc::Writer_Crt source_writer(source_context.fd);

            plnnrc::Codegen codegen;
            plnnrc::init(codegen, &tree, mem_scratch);

            plnnrc::generate_header(codegen, (output_name + "_H_").c_str(), &header_writer);
            plnnrc::generate_source(codegen, header_name.c_str(), &source_writer);
        }

        if (enable_debug_info)
        {
            plnnrc::Writer_Crt standard_output = plnnrc::make_stdout_writer();
            plnnrc::debug_output_tokens(input_buffer, &standard_output);

            plnnrc::debug_output_ast(tree, &standard_output);

            const size_t requested_size = mem_ast->get_total_requested();
            const size_t total_size = mem_ast->get_total_allocated();
            printf("req_size  = %d\n", (uint32_t)requested_size);
            printf("total_size = %d\n", (uint32_t)total_size);
        }
    }

    return 0;
}
