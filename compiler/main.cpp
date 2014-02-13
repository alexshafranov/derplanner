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

#include <derplanner/compiler/io.h>
#include <derplanner/compiler/memory.h>
#include <derplanner/compiler/s_expression.h>
#include <derplanner/compiler/errors.h>
#include <derplanner/compiler/ast.h>
#include <derplanner/compiler/ast_build.h>
#include <derplanner/compiler/codegen.h>

using namespace plnnrc;

struct buffer_context
{
    buffer_context(size_t bytes)
    {
        data = static_cast<char*>(memory::allocate(bytes));
    }

    ~buffer_context()
    {
        memory::deallocate(data);
    }

    char* data;
};

struct file_context
{
    file_context(const char* path, const char* mode)
    {
        fd = fopen(path, mode);
    }

    ~file_context()
    {
        fclose(fd);
    }

    FILE* fd;
};

size_t file_size(const char* path)
{
    file_context ctx(path, "rb");
    fseek(ctx.fd, 0, SEEK_END);
    size_t input_size = ftell(ctx.fd);
    fseek(ctx.fd, 0, SEEK_SET);
    return input_size;
}

struct error_node_comparator
{
    bool operator()(const ast::node* a, const ast::node* b)
    {
        if (a->s_expr->line == b->s_expr->line)
        {
            return a->s_expr->column < b->s_expr->column;
        }

        return a->s_expr->line < b->s_expr->line;
    }
};

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
"       Print this help message and exit.\n");
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

    if (output_dir.empty())
    {
        output_dir = ".";
    }

    std::string output_name = get_output_name(normalize(input_path));

    size_t input_size = file_size(input_path.c_str());
    buffer_context input_buffer(input_size+1);
    {
        file_context ctx(input_path.c_str(), "rb");
        size_t rb = fread(input_buffer.data, sizeof(char), input_size, ctx.fd);
        (void)rb;
    }

    input_buffer.data[input_size] = 0;

    sexpr::tree expr;
    {
        sexpr::parse_result result = expr.parse(input_buffer.data);

        if (result.status != sexpr::parse_ok)
        {
            fprintf(stderr, "error: %d:%d\n", result.line, result.column);
            return 1;
        }
    }

    ast::tree tree;
    ast::build_translation_unit(tree, expr.root());

    if (tree.error_node_cache.size() > 0)
    {
        std::stable_sort(
            &tree.error_node_cache[0],
            &tree.error_node_cache[0] + tree.error_node_cache.size(),
            error_node_comparator());

        for (unsigned i = 0; i < tree.error_node_cache.size(); ++i)
        {
            stdio_file_writer writer(stderr);
            ast::node* error = tree.error_node_cache[i];
            ast::error_ann* error_annotation = ast::annotation<ast::error_ann>(error);
            format_error(error_annotation, writer);
        }

        return 1;
    }

    std::string header_file_name = output_name + ".h";
    std::string source_file_name = output_name + ".cpp";
    std::string header_file_path = std::string(output_dir) + "/" + header_file_name;
    std::string source_file_path = std::string(output_dir) + "/" + source_file_name;

    file_context header_file(header_file_path.c_str(), "wt");
    file_context source_file(source_file_path.c_str(), "wt");

    stdio_file_writer header_writer(header_file.fd);
    stdio_file_writer source_writer(source_file.fd);

    std::string include_guard(output_name);
    include_guard += "_H_";

    codegen_options options;
    options.tab = "\t";
    options.newline = "\n";
    options.include_guard = include_guard.c_str();
    options.header_file_name = header_file_name.c_str();

    if (!custom_header.empty())
    {
        options.custom_header = custom_header.c_str();
    }
    else
    {
        options.custom_header = 0;
    }

    options.runtime_atom_names = true;
    options.runtime_task_names = true;
    options.enable_reflection = true;

    generate_header(tree, header_writer, options);
    generate_source(tree, source_writer, options);

    if (header_writer.error() || source_writer.error())
    {
        fprintf(stderr, "i/o error occured.");
        return 1;
    }

    return 0;
}
