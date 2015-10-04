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

#include "derplanner/compiler/lexer.h"
#include "derplanner/compiler/ast.h"
#include "derplanner/compiler/id_table.h"
#include "derplanner/compiler/signature_table.h"
#include "derplanner/compiler/function_table.h"

using namespace plnnrc;

void plnnrc::init(Function_Table& table, Memory* mem, uint32_t max_funcs)
{
    init(table.sigs, mem, max_funcs * 4); // allocate for 4 overloads per function on average
    init(table.infos, mem, max_funcs);
}

void plnnrc::destroy(Function_Table& table)
{
    destroy(table.sigs);
    destroy(table.infos);
}

static Function_Table::Info* get_info(Function_Table& table, const char* name)
{
    Function_Table::Info* func_info = get(table.infos, name);
    plnnrc_assert(!func_info || (func_info->first_sig + func_info->num_sigs == size_sparse(table.sigs)));

    if (!func_info)
    {
        Function_Table::Info new_info;
        memset(&new_info, 0, sizeof(new_info));
        new_info.first_sig = size_sparse(table.sigs);
        set(table.infos, name, new_info);
        func_info = get(table.infos, name);
    }

    return func_info;
}

void plnnrc::add_function(Function_Table& table, const char* name, Token_Type ret)
{
    Function_Table::Info* func_info = get_info(table, name);
    func_info->num_sigs += 1;

    begin_signature(table.sigs);
    add_param(table.sigs, ret);
    end_signature(table.sigs);
}

void plnnrc::add_function(Function_Table& table, const char* name, Token_Type ret, Token_Type arg_0)
{
    Function_Table::Info* func_info = get_info(table, name);
    func_info->num_sigs += 1;

    begin_signature(table.sigs);
    add_param(table.sigs, ret);
    add_param(table.sigs, arg_0);
    end_signature(table.sigs);
}

void plnnrc::add_function(Function_Table& table, const char* name, Token_Type ret, Token_Type arg_0, Token_Type arg_1)
{
    Function_Table::Info* func_info = get_info(table, name);
    func_info->num_sigs += 1;

    begin_signature(table.sigs);
    add_param(table.sigs, ret);
    add_param(table.sigs, arg_0);
    add_param(table.sigs, arg_1);
    end_signature(table.sigs);
}

void plnnrc::add_function(Function_Table& table, const char* name, Token_Type ret, Token_Type arg_0, Token_Type arg_1, Token_Type arg_2)
{
    Function_Table::Info* func_info = get_info(table, name);
    func_info->num_sigs += 1;

    begin_signature(table.sigs);
    add_param(table.sigs, ret);
    add_param(table.sigs, arg_0);
    add_param(table.sigs, arg_1);
    add_param(table.sigs, arg_2);
    end_signature(table.sigs);
}

bool plnnrc::has_function(const Function_Table& table, const char* name)
{
    return get(table.infos, name) != 0;
}

uint32_t plnnrc::num_signatures(const Function_Table& table)
{
    return size_sparse(table.sigs);
}

Token_Type plnnrc::get_return_type(const Function_Table& table, uint32_t signature_index)
{
    const Signature sig = get_sparse(table.sigs, signature_index);
    return sig.types[0];
}

Signature plnnrc::get_params_signature(const Function_Table& table, uint32_t signature_index)
{
    Signature sig = get_sparse(table.sigs, signature_index);
    ++sig.types;
    --sig.length;
    ++sig.offset;
    return sig;
}

static const uint32_t Num_Scalar_Numeric_Types = Token_Group_Scalar_Numeric_Last - Token_Group_Scalar_Numeric_First + 1;

static const uint32_t scalar_numeric_rank[Num_Scalar_Numeric_Types][Num_Scalar_Numeric_Types] =
{
//                Int8  Int32  Int64  Float
/* Int8 */      {   0,    1,    2,    5   },
/* Int32 */     {   3,    0,    1,    5   },
/* Int64 */     {   4,    3,    0,    5   },
/* Float */     {   8,    6,    7,    0   },
};

static uint32_t get_rank(Token_Type a, Token_Type b)
{
    plnnrc_assert(is_Scalar_Numeric(a));
    plnnrc_assert(is_Scalar_Numeric(b));
    return scalar_numeric_rank[a - Token_Group_Scalar_Numeric_First][b - Token_Group_Scalar_Numeric_First];
}

uint32_t plnnrc::resolve(const Function_Table& table, const Token_Value& name, const Array<Token_Type>& argument_types)
{
    const Function_Table::Info* func_info = get(table.infos, name);

    if (!func_info)
        return num_signatures(table);

    const uint32_t first_sig = func_info->first_sig;
    const uint32_t num_sigs = func_info->num_sigs;
    const uint32_t num_args = size(argument_types);

    uint32_t min_rank = 0xffffffff;
    uint32_t best_sig = num_signatures(table);

    for (uint32_t sig_idx = first_sig; sig_idx < first_sig + num_sigs; ++sig_idx)
    {
        Signature sig = get_params_signature(table, sig_idx);

        if (sig.length != num_args)
        {
            continue;
        }

        uint32_t func_rank = 0;

        for (uint32_t arg_idx = 0; arg_idx < num_args; ++arg_idx)
        {
            const Token_Type source_type = argument_types[arg_idx];
            const Token_Type target_type = sig.types[arg_idx];
            const Token_Type unified_type = unify(source_type, target_type);

            if (is_Not_A_Type(unified_type))
            {
                func_rank = 0xffffffff;
                break;
            }

            if (is_Scalar_Numeric(source_type) && is_Scalar_Numeric(target_type))
            {
                uint32_t rank = get_rank(source_type, target_type);
                func_rank += rank;
            }
        }

        if (func_rank < min_rank)
        {
            min_rank = func_rank;
            best_sig = sig_idx;
        }
    }

    return best_sig;
}
