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

#include <string.h>
#include "derplanner/compiler/assert.h"
#include "derplanner/compiler/memory.h"
#include "derplanner/compiler/parser.h"
#include "derplanner/compiler/lexer.h"
#include "derplanner/compiler/id_table.h"
#include "pool.h"

using namespace plnnrc;

void plnnrc::init(Parser& state, Lexer* lexer)
{
    plnnrc_assert(lexer != 0);
    // randomly chosen initial number of facts.
    init(state.fact_ids, 1024);
    // randomly chosen initial number of tasks.
    init(state.task_ids, 1024);
    state.world = 0;
    state.domain = 0;
    state.lexer = lexer;
    // pool will have 1MB sized pages.
    state.pool = create_paged_pool(1024*1024);
}

void plnnrc::destroy(Parser& state)
{
    destroy(state.pool);
    destroy(state.task_ids);
    destroy(state.fact_ids);
    memset(&state, 0, sizeof(Parser));
}

template <typename T>
T* allocate(Parser& state)
{
    return static_cast<T*>(allocate(state.pool, sizeof(T), plnnrc_alignof(T)));
}

template <typename T>
T* allocate(Parser& state, size_t count)
{
    return static_cast<T*>(allocate(state.pool, sizeof(T)*count, plnnrc_alignof(T)));
}

static ast::Fact_Type* alloc_Fact_Type(Parser& state, uint32_t num_params)
{
    ast::Fact_Type* result = allocate<ast::Fact_Type>(state);
    result->num_params = num_params;
    result->param_types = allocate<Token_Type>(state, num_params);
    return result;
}

static ast::World* alloc_World(Parser& state, uint32_t num_facts)
{
    ast::World* result = allocate<ast::World>(state);
    result->num_facts = num_facts;
    result->facts = allocate<ast::Fact_Type*>(state, num_facts);
    return result;
}

static ast::Task* alloc_Task(Parser& state, uint32_t num_cases, uint32_t num_params)
{
    ast::Task* result = allocate<ast::Task>(state);
    result->num_cases = num_cases;
    result->num_params = num_params;
    result->param_names = allocate<Token_Value>(state, num_params);
    result->param_types = allocate<Token_Type>(state, num_params);
    result->cases = allocate<ast::Case*>(state, num_cases);
    return result;
}

static ast::Case* alloc_Case(Parser& state, uint32_t num_tasks)
{
    ast::Case* result = allocate<ast::Case>(state);
    result->num_tasks = num_tasks;
    result->task_list = allocate<ast::Expr*>(state, num_tasks);
    return result;
}

static ast::Expr* alloc_Expr(Parser& state)
{
    return allocate<ast::Expr>(state);
}

void plnnrc::parse(Parser& /*state*/)
{
}
