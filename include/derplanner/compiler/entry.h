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

#ifndef DERPLANNER_COMPILER_ENTRY_H_
#define DERPLANNER_COMPILER_ENTRY_H_

#include "derplanner/compiler/types.h"

namespace plnnrc {

struct Compiler_Config
{
    // compiler errors & warnings output.
    Writer*         diag_writer;
    // parser and lexer debug output.
    Writer*         debug_writer;
    // compiler data allocator used for Abstract-Synatax-Tree, symbol tables etc.
    Memory_Stack*   data_allocator;
    // scratchpad allocator for temporary data.
    Memory_Stack*   scratch_allocator;
    // if enabled, lexer tokens and AST will be printed to `debug_writer`.
    bool            print_debug_info;
    // string to be used as a preprocessor header guard symbol in domain header.
    const char*     header_guard;
    // name of domain header to be included from domain source.
    const char*     header_file_name;
    // domain header file writer.
    Writer*         header_writer;
    // domain source file writer.
    Writer*         source_writer;
};

// derplanner compiler entry point.
bool compile(const Compiler_Config* config, const char* input_buffer);

}

#endif
