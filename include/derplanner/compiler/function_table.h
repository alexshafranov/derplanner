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

#ifndef DERPLANNER_COMPILER_FUNCTION_TABLE_H_
#define DERPLANNER_COMPILER_FUNCTION_TABLE_H_

#include "derplanner/compiler/types.h"

namespace plnnrc {

void init(Function_Table& table, Memory* mem, uint32_t max_funcs);
void destroy(Function_Table& table);

// returns true if the function with name `name` is in the table.
bool has_function(const Function_Table& table, const char* name);

// returns the number of signatures stored in the function table.
uint32_t num_signatures(const Function_Table& table);

/// Function_Table builders.

void add_function(Function_Table& table, const char* name, Token_Type ret);
void add_function(Function_Table& table, const char* name, Token_Type ret, Token_Type arg_0);
void add_function(Function_Table& table, const char* name, Token_Type ret, Token_Type arg_0, Token_Type arg_1);
void add_function(Function_Table& table, const char* name, Token_Type ret, Token_Type arg_0, Token_Type arg_1, Token_Type arg_2);

// finds the signature index in `table.sigs` which is the first function overload given specific `argument_types`. returns `num_signatures` in case of failure.
uint32_t resolve(const Function_Table& table, const Token_Value& name, const Array<Token_Type>& argument_types);

// get the function return type (stored as a first element of signature).
Token_Type get_return_type(const Function_Table& table, uint32_t signature_index);

// gets parameter type tuple (return type is skipped).
Signature get_params_signature(const Function_Table& table, uint32_t signature_index);

}

#endif
