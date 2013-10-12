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
#include <derplanner/compiler/s_expression.h>
#include <derplanner/compiler/ast.h>
#include <derplanner/compiler/domain.h>
#include <derplanner/compiler/codegen.h>

using namespace plnnrc;

size_t file_size(const char* path)
{
    FILE* fd = fopen(path, "rb");
    fseek(fd, 0, SEEK_END);
    size_t input_size = ftell(fd);
    fseek(fd, 0, SEEK_SET);
    fclose(fd);
    return input_size;
}

int main(int argc, char** argv)
{
    const char* input_path = argv[1];
    const char* output_path = argv[2];

    FILE* fd = fopen(input_path, "rt");

    size_t input_size = file_size(input_path);

    char* input_data = new char[input_size];

    {
        size_t rb = fread(input_data, sizeof(char), input_size, fd);
        (void)rb;
    }

    fclose(fd);

    fd = fopen(output_path, "wt");

    stdio_file_writer writer(fd);

    sexpr::tree expr;
    expr.parse(input_data);

    ast::tree tree;
    ast::build_worldstate(tree, expr.root()->first_child);
    ast::build_domain(tree, expr.root()->first_child->next_sibling);

    ast::infer_types(tree);
    ast::annotate(tree);

    codegen_options options;
    options.tab = "\t";
    options.newline = "\n";
    options.include_guard = "XXX_H_";

    generate_header(tree, writer, options);
    generate_source(tree, writer, options);

    if (writer.error())
    {
        printf("i/o error occured.");
    }

    delete [] input_data;
    fclose(fd);
}
