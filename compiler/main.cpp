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
#include <stdio.h>

#include <derplanner/compiler/io.h>
#include <derplanner/compiler/memory.h>
#include <derplanner/compiler/s_expression.h>
#include <derplanner/compiler/ast.h>
#include <derplanner/compiler/domain.h>
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

int main(int argc, char** argv)
{
    const char* input_path = argv[1];
    const char* output_dir = argv[2];
    const char* output_name = argv[3];

    size_t input_size = file_size(input_path);
    buffer_context input_buffer(input_size);
    {
        file_context ctx(input_path, "rt");
        size_t rb = fread(input_buffer.data, sizeof(char), input_size, ctx.fd);
        (void)rb;
    }

    sexpr::tree expr;
    expr.parse(input_buffer.data);

    ast::tree tree;
    ast::build_worldstate(tree, expr.root()->first_child);
    ast::build_domain(tree, expr.root()->first_child->next_sibling);

    ast::infer_types(tree);
    ast::annotate(tree);

    std::string header_file_name = std::string(output_name) + ".h";
    std::string source_file_name = std::string(output_name) + ".cpp";
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
    options.runtime_atom_names = true;
    options.runtime_task_names = true;
    options.enable_reflection = true;

    generate_header(tree, header_writer, options);
    generate_source(tree, source_writer, options);

    if (header_writer.error() || source_writer.error())
    {
        printf("i/o error occured.");
    }
}
