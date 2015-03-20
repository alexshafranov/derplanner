#include <string.h>
#include "derplanner/runtime/memory.h"
#include "derplanner/runtime/database.h"
#include "derplanner/runtime/planning.h"
#include "derplanner/runtime/domain_support.h"
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

#define plnnr_coroutine_begin(state, label) switch (state->label) { case 0:
#define plnnr_coroutine_yield(state, label, value) state->label = value; return true; case value:;
#define plnnr_coroutine_end() } return false

#define PLNNR_STATIC_ARRAY_SIZE(array) sizeof(array)/sizeof(array[0])

static bool root_branch_0_expand(Planning_State*, Expansion_Frame*, Fact_Database*);
static bool travel_branch_0_expand(Planning_State*, Expansion_Frame*, Fact_Database*);
static bool travel_branch_1_expand(Planning_State*, Expansion_Frame*, Fact_Database*);
static bool travel_by_plane_branch_0_expand(Planning_State*, Expansion_Frame*, Fact_Database*);

static const char* s_fact_names[] = { "start", "finish", "short_distance", "long_distance", "airport" };
static const char* s_task_names[] = { "!taxi", "!plane", "root", "travel", "travel_by_plane" };

static Composite_Task_Expand* s_task_expands[] = { root_branch_0_expand, travel_branch_0_expand, travel_by_plane_branch_0_expand };

// Fact types for database access.
static Fact_Type s_fact_types[] = {
	{ 1, { Type_Int32, } },
	{ 1, { Type_Int32, } },
	{ 2, { Type_Int32, Type_Int32 } },
	{ 2, { Type_Int32, Type_Int32 } },
	{ 2, { Type_Int32, Type_Int32 } },
};

static Type s_layout_types[] = { Type_Int32, Type_Int32 };
static size_t s_layout_offsets[2];
static int s_layout_num_params[] = { 2 };

static Param_Layout s_task_parameters[] = {
	{ 2, s_layout_types + 0, 0, s_layout_offsets + 0 },
	{ 2, s_layout_types + 0, 0, s_layout_offsets + 0 },
	{ 0, 0, 0, 0 },
	{ 2, s_layout_types + 0, 0, s_layout_offsets + 0 },
	{ 2, s_layout_types + 0, 0, s_layout_offsets + 0 },
};

static Param_Layout s_precond_results[] = {
	{ 2, s_layout_types + 0, 0, s_layout_offsets + 0 },
	{ 0, 0, 0, 0 },
	{ 0, 0, 0, 0 },
	{ 2, s_layout_types + 0, 0, s_layout_offsets + 0 },
};

static uint32_t s_num_cases[] = { 1, 2, 1 };

static uint32_t s_size_hints[] = { 1, 1, 0, 0, 0 };

static uint32_t s_hashes[] = { 0, 0, 0, 0, 0 };

static Domain_Info s_domain_info = {
	// task_info
	{
		5,				// num_tasks
		2,				// num_primitive
		3,  			// num_composite
		s_num_cases, 	// num_cases
		0,				// hashes
		s_task_names,
		s_task_parameters,
		s_precond_results,
		s_task_expands,
	},
	// database_req
	{
		5,				// num_tables
		s_size_hints,	// size_hints
		s_fact_types,
		s_hashes,		// hashes
		s_fact_names,
	},
};
///////////////////////////////////////////////////////////////////////////////

void travel_init_domain_info()
{
	for (size_t i = 0; i < PLNNR_STATIC_ARRAY_SIZE(s_task_parameters); ++i)
	{
		compute_offsets_and_size(&s_task_parameters[i]);
	}

	for (size_t i = 0; i < PLNNR_STATIC_ARRAY_SIZE(s_precond_results); ++i)
	{
		compute_offsets_and_size(&s_precond_results[i]);
	}
}
///////////////////////////////////////////////////////////////////////////////

const Domain_Info* travel_get_domain_info()
{
	return &s_domain_info;
}
///////////////////////////////////////////////////////////////////////////////

/// Precondition iterators

static bool p0_next(Planning_State* state, Expansion_Frame* frame, Fact_Database* db)
{
	Fact_Handle* handles = frame->handles;

	plnnr_coroutine_begin(frame, precond_label);
	handles = allocate_precond_handles(state, frame, 2);
	allocate_precond_result(state, frame, s_precond_results[0]);

	for (handles[0] = first(db, 0); is_valid(db, handles[0]); handles[0] = next(db, handles[0]))
	{
		for (handles[1] = first(db, 1); is_valid(db, handles[1]); handles[1] = next(db, handles[1]))
		{
			set_precond_result(frame, s_precond_results[0], 0, as_Int32(db, handles[0], 0));
			set_precond_result(frame, s_precond_results[0], 1, as_Int32(db, handles[1], 0));

			plnnr_coroutine_yield(frame, precond_label, 1);
		}
	}

	plnnr_coroutine_end();
}
///////////////////////////////////////////////////////////////////////////////

struct p1_input { int _0; int _1; };

static bool p1_next(Planning_State* state, Expansion_Frame* frame, Fact_Database* db, p1_input* args)
{
	Fact_Handle* handles = frame->handles;

	plnnr_coroutine_begin(frame, precond_label);
	handles = allocate_precond_handles(state, frame, 1);

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

		plnnr_coroutine_yield(frame, precond_label, 1);
	}

	plnnr_coroutine_end();
}
///////////////////////////////////////////////////////////////////////////////

struct p2_input { int _0; int _1; };

static bool p2_next(Planning_State* state, Expansion_Frame* frame, Fact_Database* db, p2_input* args)
{
	Fact_Handle* handles = frame->handles;

	plnnr_coroutine_begin(frame, precond_label);
	handles = allocate_precond_handles(state, frame, 1);

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

		plnnr_coroutine_yield(frame, precond_label, 1);
	}

	plnnr_coroutine_end();
}
///////////////////////////////////////////////////////////////////////////////

struct p3_input { int _0; int _1; };

static bool p3_next(Planning_State* state, Expansion_Frame* frame, Fact_Database* db, p3_input* args)
{
	Fact_Handle* handles = frame->handles;

	plnnr_coroutine_begin(frame, precond_label);
	handles = allocate_precond_handles(state, frame, 2);
	allocate_precond_result(state, frame, s_precond_results[3]);

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

			set_precond_result(frame, s_precond_results[3], 0, as_Int32(db, handles[0], 1));
			set_precond_result(frame, s_precond_results[3], 1, as_Int32(db, handles[1], 1));

			plnnr_coroutine_yield(frame, precond_label, 1);
		}
	}

	plnnr_coroutine_end();
}
///////////////////////////////////////////////////////////////////////////////

/// Composite task expansions

static bool root_branch_0_expand(Planning_State* state, Expansion_Frame* frame, Fact_Database* db)
{
	plnnr_coroutine_begin(frame, expand_label);

	while (p0_next(state, frame, db))
	{
		begin_composite(state, 3, travel_branch_0_expand, s_task_parameters[3]);
		set_composite_arg(state, s_task_parameters[3], 0, as_Int32(frame->precond_result, s_precond_results[0], 0));
		set_composite_arg(state, s_task_parameters[3], 1, as_Int32(frame->precond_result, s_precond_results[0], 1));
		frame->flags |= Expansion_Frame::Flags_Expanded;
		plnnr_coroutine_yield(frame, expand_label, 1);
	}

	plnnr_coroutine_end();
}
///////////////////////////////////////////////////////////////////////////////

static bool travel_branch_0_expand(Planning_State* state, Expansion_Frame* frame, Fact_Database* db)
{
	int _0 = as_Int32(frame->arguments, s_task_parameters[3], 0);
	int _1 = as_Int32(frame->arguments, s_task_parameters[3], 1);
	p1_input p1_args;
	p1_args._0 = _0;
	p1_args._1 = _1;

	plnnr_coroutine_begin(frame, expand_label);

	while (p1_next(state, frame, db, &p1_args))
	{
		begin_task(state, 0, s_task_parameters[0]);
		set_task_arg(state, s_task_parameters[0], 0, _0);
		set_task_arg(state, s_task_parameters[0], 1, _1);
		frame->flags |= Expansion_Frame::Flags_Expanded;
		plnnr_coroutine_yield(frame, expand_label, 1);
	}

	return expand_next_case(state, frame, db, travel_branch_1_expand, s_task_parameters[0]);

	plnnr_coroutine_end();
}
///////////////////////////////////////////////////////////////////////////////

static bool travel_branch_1_expand(Planning_State* state, Expansion_Frame* frame, Fact_Database* db)
{
	int _0 = as_Int32(frame->arguments, s_task_parameters[3], 0);
	int _1 = as_Int32(frame->arguments, s_task_parameters[3], 1);
	p2_input p2_args;
	p2_args._0 = _0;
	p2_args._1 = _1;

	plnnr_coroutine_begin(frame, expand_label);

	while (p2_next(state, frame, db, &p2_args))
	{
		begin_composite(state, 4, travel_by_plane_branch_0_expand, s_task_parameters[4]);
		set_composite_arg(state, s_task_parameters[4], 0, _0);
		set_composite_arg(state, s_task_parameters[4], 1, _1);
		frame->flags |= Expansion_Frame::Flags_Expanded;
		plnnr_coroutine_yield(frame, expand_label, 1);
	}

	plnnr_coroutine_end();
}
///////////////////////////////////////////////////////////////////////////////

static bool travel_by_plane_branch_0_expand(Planning_State* state, Expansion_Frame* frame, Fact_Database* db)
{
	int _0 = as_Int32(frame->arguments, s_task_parameters[4], 0);
	int _1 = as_Int32(frame->arguments, s_task_parameters[4], 1);
	p3_input p3_args;
	p3_args._0 = _0;
	p3_args._1 = _1;

	plnnr_coroutine_begin(frame, expand_label);

	while (p3_next(state, frame, db, &p3_args))
	{
		begin_composite(state, 3, travel_branch_0_expand, s_task_parameters[3]);
		set_composite_arg(state, s_task_parameters[3], 0, _0);
		set_composite_arg(state, s_task_parameters[3], 1, as_Int32(frame->precond_result, s_precond_results[3], 0));

		plnnr_coroutine_yield(frame, expand_label, 1);

		if ((frame->flags & Expansion_Frame::Flags_Failed) != 0)
		{
			continue;
		}

		begin_task(state, 1, s_task_parameters[1]);
		set_task_arg(state, s_task_parameters[1], 0, as_Int32(frame->precond_result, s_precond_results[3], 0));
		set_task_arg(state, s_task_parameters[1], 1, as_Int32(frame->precond_result, s_precond_results[3], 1));

		plnnr_coroutine_yield(frame, expand_label, 2);

		begin_composite(state, 3, travel_branch_0_expand, s_task_parameters[3]);
		set_composite_arg(state, s_task_parameters[3], 0, as_Int32(frame->precond_result, s_precond_results[3], 1));
		set_composite_arg(state, s_task_parameters[3], 1, _1);

		frame->flags |= Expansion_Frame::Flags_Expanded;
		plnnr_coroutine_yield(frame, expand_label, 3);
	}

	plnnr_coroutine_end();
}
///////////////////////////////////////////////////////////////////////////////
