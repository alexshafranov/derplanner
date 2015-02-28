#include <string.h>
#include "derplanner/runtime/database.h"
#include "derplanner/runtime/planning.h"
#include "travel.h"

using namespace plnnr;

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif

#ifdef _MSC_VER
#pragma warning(disable: 4100) // unreferenced formal parameter
#pragma warning(disable: 4189) // local variable is initialized but not referenced
#endif

#define PLNNR_COROUTINE_BEGIN(state, label) switch (state->label) { case 0:
#define PLNNR_COROUTINE_YIELD(state, label, value) state->label = value; return true; case value:;
#define PLNNR_COROUTINE_END() } return false

static bool root_branch_0_expand(Planning_State*, Expansion_Frame*, Fact_Database*);
static bool travel_branch_0_expand(Planning_State*, Expansion_Frame*, Fact_Database*);
static bool travel_branch_1_expand(Planning_State*, Expansion_Frame*, Fact_Database*);
static bool travel_by_air_branch_0_expand(Planning_State*, Expansion_Frame*, Fact_Database*);

static const char* s_fact_names[] = { "start", "finish", "short_distance", "long_distance", "airport" };
static const char* s_task_names[] = { "!go_by_taxi", "!go_by_plane", "root", "travel", "travel_by_plane" };

static Composite_Task_Expand* s_task_expands[] = { root_branch_0_expand, travel_branch_0_expand, travel_by_air_branch_0_expand };

static Fact_Type s_fact_types[] = {
	{ 1, { Type_Int32, } },
	{ 1, { Type_Int32, } },
	{ 2, { Type_Int32, Type_Int32 } },
	{ 2, { Type_Int32, Type_Int32 } },
	{ 2, { Type_Int32, Type_Int32 } },
};

static Fact_Type s_task_parameters[] = {
	{ 2, { Type_Int32, Type_Int32 } },
	{ 2, { Type_Int32, Type_Int32 } },
	{ 0, {  } },
	{ 2, { Type_Int32, Type_Int32 } },
	{ 2, { Type_Int32, Type_Int32 } },
};

static Domain_Info s_domain_info = {
	// task_info
	{
		5,	// num_tasks
		2,	// num_primitive
		0,	// hashes
		s_task_names,
		s_task_parameters,
		s_task_expands,
	},
	// database_req
	{
		5, // num_tables
		0, // size_hints
		s_fact_types,
		0, // hashes
		s_fact_names,
	},
};
///////////////////////////////////////////////////////////////////////////////

const Domain_Info* travel_domain_info()
{
	return &s_domain_info;
}
///////////////////////////////////////////////////////////////////////////////

/// Precondition iterators

struct p0_output { int _0; int _1; };

static bool p0_next(Planning_State* state, Expansion_Frame* frame, Fact_Database* db, p0_output* result)
{
	Fact_Handle* handles = frame->handles;

	PLNNR_COROUTINE_BEGIN(frame, precond_label);
	allocate_handles(state, frame, 2);

	for (handles[0] = first(db, 0); is_valid(db, handles[0]); handles[0] = next(db, handles[0]))
	{
		for (handles[1] = first(db, 1); is_valid(db, handles[1]); handles[1] = next(db, handles[1]))
		{
			result->_0 = as_Int32(db, handles[0], 0);
			result->_1 = as_Int32(db, handles[1], 0);
			PLNNR_COROUTINE_YIELD(frame, precond_label, 1);
		}
	}

	PLNNR_COROUTINE_END();
}
///////////////////////////////////////////////////////////////////////////////

struct p1_input { int _0; int _1; };

static bool p1_next(Planning_State* state, Expansion_Frame* frame, Fact_Database* db, p1_input* args)
{
	Fact_Handle* handles = frame->handles;

	PLNNR_COROUTINE_BEGIN(frame, precond_label);
	allocate_handles(state, frame, 1);

	for (handles[0] = first(db, 2); is_valid(db, handles[0]); handles[0] = next(db, handles[0]))
	{
		if (as_Int32(db, handles[0], 0) != args->_0)
		{
			continue;
		}

		if (as_Int32(db, handles[0], 1) != args->_1)
		{
			continue;
		}

		PLNNR_COROUTINE_YIELD(frame, precond_label, 1);
	}

	PLNNR_COROUTINE_END();
}
///////////////////////////////////////////////////////////////////////////////

struct p2_input { int _0; int _1; };

static bool p2_next(Planning_State* state, Expansion_Frame* frame, Fact_Database* db, p2_input* args)
{
	Fact_Handle* handles = frame->handles;

	PLNNR_COROUTINE_BEGIN(frame, precond_label);
	allocate_handles(state, frame, 1);

	for (handles[0] = first(db, 3); is_valid(db, handles[0]); handles[0] = next(db, handles[0]))
	{
		if (as_Int32(db, handles[0], 0) != args->_0)
		{
			continue;
		}

		if (as_Int32(db, handles[0], 1) != args->_1)
		{
			continue;
		}

		PLNNR_COROUTINE_YIELD(frame, precond_label, 1);
	}

	PLNNR_COROUTINE_END();
}
///////////////////////////////////////////////////////////////////////////////

struct p3_input { int _0; int _1; };

struct p3_output { int _0; int _1; };

static bool p3_next(Planning_State* state, Expansion_Frame* frame, Fact_Database* db, p3_input* args, p3_output* result)
{
	Fact_Handle* handles = frame->handles;

	PLNNR_COROUTINE_BEGIN(frame, precond_label);
	allocate_handles(state, frame, 2);

	for (handles[0] = first(db, 4); is_valid(db, handles[0]); handles[0] = next(db, handles[0]))
	{
		if (as_Int32(db, handles[0], 0) != args->_0)
		{
			continue;
		}

		for (handles[1] = first(db, 4); is_valid(db, handles[1]); handles[1] = next(db, handles[1]))
		{
			if (as_Int32(db, handles[1], 0) != args->_1)
			{
				continue;
			}

			result->_0 = as_Int32(db, handles[0], 1);
			result->_1 = as_Int32(db, handles[1], 1);
			PLNNR_COROUTINE_YIELD(frame, precond_label, 1);
		}
	}

	PLNNR_COROUTINE_END();
}
///////////////////////////////////////////////////////////////////////////////

/// Composite task expansions

static bool root_branch_0_expand(Planning_State* state, Expansion_Frame* frame, Fact_Database* db)
{
	p0_output p0_result;

	PLNNR_COROUTINE_BEGIN(frame, expand_label);

	while (p0_next(state, frame, db, &p0_result))
	{
		push_composite(state, 3, travel_branch_0_expand, 2);
		push_composite_arg(state, Type_Int32, p0_result._0);
		push_composite_arg(state, Type_Int32, p0_result._1);
		frame->flags |= Expansion_Frame::Flags_Expanded;
		PLNNR_COROUTINE_YIELD(frame, expand_label, 1);
	}

	PLNNR_COROUTINE_END();
}
///////////////////////////////////////////////////////////////////////////////

static bool travel_branch_0_expand(Planning_State* state, Expansion_Frame* frame, Fact_Database* db)
{
	int _0 = as_Int32(frame->arguments, s_task_parameters[3], 0);
	int _1 = as_Int32(frame->arguments, s_task_parameters[3], 1);
	p1_input p1_args;
	p1_args._0 = _0;
	p1_args._1 = _1;

	PLNNR_COROUTINE_BEGIN(frame, expand_label);

	while (p1_next(state, frame, db, &p1_args))
	{
		push_task(state, 0, 2);
		push_task_arg(state, Type_Int32, _0);
		push_task_arg(state, Type_Int32, _1);
		frame->flags |= Expansion_Frame::Flags_Expanded;
		PLNNR_COROUTINE_YIELD(frame, expand_label, 1);
	}

	return expand_next_case(state, frame, db, travel_branch_1_expand);

	PLNNR_COROUTINE_END();
}
///////////////////////////////////////////////////////////////////////////////

bool travel_branch_1_expand(Planning_State* state, Expansion_Frame* frame, Fact_Database* db)
{
	p2_state* precondition = plnnr::precondition<p2_state>(method);
	travel_args* method_args = plnnr::arguments<travel_args>(method);
	Worldstate* wstate = static_cast<Worldstate*>(world);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push_precondition<p2_state>(pstate, method);
	precondition->_0 = method_args->_0;
	precondition->_1 = method_args->_1;

	while (next(*precondition, *wstate))
	{
		{
			Method_Instance* t = push_method(pstate, task_travel_by_air, travel_by_air_branch_0_expand);
			travel_by_air_args* a = push_arguments<travel_by_air_args>(pstate, t);
			a->_0 = method_args->_0;
			a->_1 = method_args->_1;
		}

		method->flags |= method_flags_expanded;
		PLNNR_COROUTINE_YIELD(*method, 1);
	}

	PLNNR_COROUTINE_END();
}

bool travel_by_air_branch_0_expand(Planning_State* state, Expansion_Frame* frame, Fact_Database* db)
{
	p3_state* precondition = plnnr::precondition<p3_state>(method);
	travel_by_air_args* method_args = plnnr::arguments<travel_by_air_args>(method);
	Worldstate* wstate = static_cast<Worldstate*>(world);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push_precondition<p3_state>(pstate, method);
	precondition->_0 = method_args->_0;
	precondition->_2 = method_args->_1;

	while (next(*precondition, *wstate))
	{
		{
			Method_Instance* t = push_method(pstate, task_travel, travel_branch_0_expand);
			travel_args* a = push_arguments<travel_args>(pstate, t);
			a->_0 = method_args->_0;
			a->_1 = precondition->_1;
		}

		PLNNR_COROUTINE_YIELD(*method, 1);

		if (method->flags & method_flags_failed)
		{
			continue;
		}

		{
			Task_Instance* t = push_task(pstate, task_fly, 0);
			fly_args* a = push_arguments<fly_args>(pstate, t);
			a->_0 = precondition->_1;
			a->_1 = precondition->_3;
		}

		PLNNR_COROUTINE_YIELD(*method, 2);

		{
			// Method_Instance* t = push_method(pstate, task_travel, travel_branch_0_expand);
			// travel_args* a = push_arguments<travel_args>(pstate, t);
			// a->_0 = precondition->_3;
			// a->_1 = method_args->_1;
			push_composite(state, 3, travel_branch_0_expand);
			push_argument(state, precondition->_3);
			push_argument(state, method_args->_1);
		}

		method->flags |= method_flags_expanded;
		PLNNR_COROUTINE_YIELD(*method, 3);
	}

	PLNNR_COROUTINE_END();
}
