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

#include <string>
#include <algorithm>
#include <stdio.h>
#include <string.h>

#include "derplanner/compiler/memory.h"
#include "derplanner/compiler/io.h"
#include "derplanner/compiler/entry.h"

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
"   --help, -h\n"
"       Print this help message and exit.\n"
"\n"
"   --compiler-debug\n"
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

struct Commandline
{
    std::string output_dir;
    std::string input_path;
    bool compiler_debug;
};

bool parse_cmdline(int argc, char** argv, Commandline& result)
{
    result.compiler_debug = false;

    for (int i = 1; i < argc; ++i)
    {
        const char* argument = argv[i];

        if (!argument[0])
        {
            fprintf(stderr, "error: received empty cmdline argument!\n");
            return false;
        }

        std::string name;
        std::string value;

        bool expecting_value = parse_argument(argument, name, value);

        if (expecting_value)
        {
            if (name == "h" || name == "help")
            {
                print_help();
                return false;
            }

            if (name == "compiler-debug")
            {
                result.compiler_debug = true;
                continue;
            }

            if (i + 1 >= argc || argv[i + 1][0] == '-')
            {
                fprintf(stderr, "error: missing value for flag: %s\n", name.c_str());
                return false;
            }

            value = std::string(argv[++i]);
        }

        if (!name.empty())
        {
            if (name == "out" || name == "o")
            {
                if (!result.output_dir.empty())
                {
                    fprintf(stderr, "error: multiple values for flag: %s\n", name.c_str());
                    return false;
                }

                result.output_dir = value;
                continue;
            }
        }
        else
        {
            if (!result.input_path.empty())
            {
                fprintf(stderr, "error: multiple inputs specified.\n");
                return false;
            }

            result.input_path = value;
        }
    }

    if (result.input_path.empty())
    {
        fprintf(stderr, "error: no source file specified.\n");
        return false;
    }

    return true;
}

int main(int argc, char** argv)
{
    Commandline cmdline;

    if (!parse_cmdline(argc, argv, cmdline))
    {
        return 1;
    }

    {
        File_Context ctx(cmdline.input_path.c_str(), "rb");

        if (!ctx.fd)
        {
            fprintf(stderr, "error: can't open input file: '%s'.\n", cmdline.input_path.c_str());
            return 1;
        }
    }

    if (cmdline.output_dir.empty())
    {
        cmdline.output_dir = ".";
    }

    std::string output_name = get_output_name(normalize(cmdline.input_path));

    std::string header_name = output_name + ".h";
    std::replace(header_name.begin(), header_name.end(), '-', '_');

    std::string source_name = output_name + ".cpp";
    std::replace(source_name.begin(), source_name.end(), '-', '_');

    std::string header_path = cmdline.output_dir + "/" + header_name;
    std::string source_path = cmdline.output_dir + "/" + source_name;

    std::string header_guard = output_name + "_H_";
    std::replace(header_guard.begin(), header_guard.end(), '-', '_');

    plnnrc::Memory_Stack_Context mem_ctx_data(32 * 1024);
    plnnrc::Memory_Stack_Context mem_ctx_scratch(32 * 1024);

    plnnrc::Memory_Stack* mem_data = mem_ctx_data.mem;
    plnnrc::Memory_Stack* mem_scratch = mem_ctx_scratch.mem;

    size_t input_size = file_size(cmdline.input_path.c_str());
    char*  input_buffer = static_cast<char*>(mem_scratch->allocate(input_size + 1));
    {
        File_Context ctx(cmdline.input_path.c_str(), "rb");
        size_t rb = fread(input_buffer, sizeof(char), input_size, ctx.fd);
        (void)rb;
    }

    input_buffer[input_size] = 0;

    File_Context header_context(header_path.c_str(), "wb");
    if (!header_context.fd)
    {
        fprintf(stderr, "error: can't open output file: '%s'.\n", header_path.c_str());
        return 1;
    }

    File_Context source_context(source_path.c_str(), "wb");
    if (!source_context.fd)
    {
        fprintf(stderr, "error: can't open output file: '%s'.\n", source_path.c_str());
        return 1;
    }

    plnnrc::Writer_Crt stderr_writer = plnnrc::make_stderr_writer();
    plnnrc::Writer_Crt stdout_writer = plnnrc::make_stdout_writer();

    plnnrc::Writer_Crt header_writer(header_context.fd);
    plnnrc::Writer_Crt source_writer(source_context.fd);

    plnnrc::Compiler_Config compiler_config;
    compiler_config.diag_writer = &stderr_writer;
    compiler_config.debug_writer = &stdout_writer;
    compiler_config.data_allocator = mem_data;
    compiler_config.scratch_allocator = mem_scratch;
    compiler_config.print_debug_info = cmdline.compiler_debug;
    compiler_config.header_guard = header_guard.c_str();
    compiler_config.header_file_name = header_name.c_str();
    compiler_config.header_writer = &header_writer;
    compiler_config.source_writer = &source_writer;

    bool successful = compile(&compiler_config, input_buffer);

    return successful ? 0 : 1;
}
