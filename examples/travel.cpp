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

#define PLNNR_COROUTINE_BEGIN(state, label) switch (state->label) { case 0:
#define PLNNR_COROUTINE_YIELD(state, label, value) state->label = value; return true; case value:;
#define PLNNR_COROUTINE_END() } return false

#define PLNNR_STATIC_ARRAY_SIZE(array) sizeof(array)/sizeof(array[0])

static bool root_branch_0_expand(Planning_State*, Expansion_Frame*, Fact_Database*);
static bool travel_branch_0_expand(Planning_State*, Expansion_Frame*, Fact_Database*);
static bool travel_branch_1_expand(Planning_State*, Expansion_Frame*, Fact_Database*);
static bool travel_by_plane_branch_0_expand(Planning_State*, Expansion_Frame*, Fact_Database*);

static const char* s_fact_names[] = { "start", "finish", "short_distance", "long_distance", "airport" };
static const char* s_task_names[] = { "!go_by_taxi", "!go_by_plane", "root", "travel", "travel_by_plane" };

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
	{ 2, s_layout_types + 0, s_layout_offsets + 0 },
	{ 2, s_layout_types + 0, s_layout_offsets + 0 },
	{ 0, 0, 0 },
	{ 2, s_layout_types + 0, s_layout_offsets + 0 },
	{ 2, s_layout_types + 0, s_layout_offsets + 0 },
};

static Param_Layout s_precondition_output[] = {
	{ 2, s_layout_types + 0, s_layout_offsets + 0 },
	{ 0, 0, 0 },
	{ 0, 0, 0 },
	{ 2, s_layout_types + 0, s_layout_offsets + 0 },
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
	int first_layout_param = 0;
	for (int i = 0; i < PLNNR_STATIC_ARRAY_SIZE(s_layout_num_params); ++i)
	{
		int num_layout_params = s_layout_num_params[i];

		Type type = s_layout_types[first_layout_param];
		s_layout_offsets[first_layout_param] = 0;
		size_t offset = get_type_size(type);

		for (int j = first_layout_param + 1; j < first_layout_param + num_layout_params; ++j)
		{
			Type type = s_layout_types[j];
			size_t alignment = get_type_alignment(type);
			size_t size = get_type_size(type);

			s_layout_offsets[j] = offset;
			offset += size;
		}

		first_layout_param += num_layout_params;
	}

	return &s_domain_info;
}
///////////////////////////////////////////////////////////////////////////////

/// Precondition iterators

static bool p0_next(Planning_State* state, Expansion_Frame* frame, Fact_Database* db)
{
	Fact_Handle* handles = frame->handles;

	PLNNR_COROUTINE_BEGIN(frame, precond_label);
	allocate_handles(state, frame, 2);
	allocate_output(state, frame, s_precondition_output[0]);

	for (handles[0] = first(db, 0); is_valid(db, handles[0]); handles[0] = next(db, handles[0]))
	{
		for (handles[1] = first(db, 1); is_valid(db, handles[1]); handles[1] = next(db, handles[1]))
		{
			write_output(frame, s_precondition_output[0], 0, as_Int32(db, handles[0], 0));
			write_output(frame, s_precondition_output[0], 1, as_Int32(db, handles[1], 0));

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

static bool p3_next(Planning_State* state, Expansion_Frame* frame, Fact_Database* db, p3_input* args)
{
	Fact_Handle* handles = frame->handles;

	PLNNR_COROUTINE_BEGIN(frame, precond_label);
	allocate_handles(state, frame, 2);
	allocate_output(state, frame, s_precondition_output[3]);

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

			write_output(frame, s_precondition_output[3], 0, as_Int32(db, handles[0], 1));
			write_output(frame, s_precondition_output[3], 1, as_Int32(db, handles[1], 1));

			PLNNR_COROUTINE_YIELD(frame, precond_label, 1);
		}
	}

	PLNNR_COROUTINE_END();
}
///////////////////////////////////////////////////////////////////////////////

/// Composite task expansions

static bool root_branch_0_expand(Planning_State* state, Expansion_Frame* frame, Fact_Database* db)
{
	PLNNR_COROUTINE_BEGIN(frame, expand_label);

	while (p0_next(state, frame, db))
	{
		begin_composite(state, 3, travel_branch_0_expand, s_task_parameters[3]);
		set_composite_arg(state, s_task_parameters[3], 0, as_Int32(frame->precond_output, s_precondition_output[0], 0));
		set_composite_arg(state, s_task_parameters[3], 1, as_Int32(frame->precond_output, s_precondition_output[0], 1));
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
		begin_task(state, 0, s_task_parameters[0]);
		set_task_arg(state, s_task_parameters[0], 0, _0);
		set_task_arg(state, s_task_parameters[0], 1, _1);
		frame->flags |= Expansion_Frame::Flags_Expanded;
		PLNNR_COROUTINE_YIELD(frame, expand_label, 1);
	}

	return expand_next_case(state, frame, db, travel_branch_1_expand);

	PLNNR_COROUTINE_END();
}
///////////////////////////////////////////////////////////////////////////////

static bool travel_branch_1_expand(Planning_State* state, Expansion_Frame* frame, Fact_Database* db)
{
	int _0 = as_Int32(frame->arguments, s_task_parameters[3], 0);
	int _1 = as_Int32(frame->arguments, s_task_parameters[3], 1);
	p2_input p2_args;
	p2_args._0 = _0;
	p2_args._1 = _1;

	PLNNR_COROUTINE_BEGIN(frame, expand_label);

	while (p2_next(state, frame, db, &p2_args))
	{
		begin_composite(state, 4, travel_by_plane_branch_0_expand, s_task_parameters[4]);
		set_composite_arg(state, s_task_parameters[4], 0, _0);
		set_composite_arg(state, s_task_parameters[4], 1, _1);
		frame->flags |= Expansion_Frame::Flags_Expanded;
		PLNNR_COROUTINE_YIELD(frame, expand_label, 1);
	}

	PLNNR_COROUTINE_END();
}
///////////////////////////////////////////////////////////////////////////////

static bool travel_by_plane_branch_0_expand(Planning_State* state, Expansion_Frame* frame, Fact_Database* db)
{
	int _0 = as_Int32(frame->arguments, s_task_parameters[4], 0);
	int _1 = as_Int32(frame->arguments, s_task_parameters[4], 1);
	p3_input p3_args;
	p3_args._0 = _0;
	p3_args._1 = _1;

	PLNNR_COROUTINE_BEGIN(frame, expand_label);

	while (p3_next(state, frame, db, &p3_args))
	{
		begin_composite(state, 3, travel_branch_0_expand, s_task_parameters[3]);
		set_composite_arg(state, s_task_parameters[3], 0, _0);
		set_composite_arg(state, s_task_parameters[3], 1, as_Int32(frame->precond_output, s_precondition_output[3], 0));

		PLNNR_COROUTINE_YIELD(frame, expand_label, 1);

		if ((frame->flags & Expansion_Frame::Flags_Failed) != 0)
		{
			continue;
		}

		begin_task(state, 1, s_task_parameters[1]);
		set_task_arg(state, s_task_parameters[1], 0, as_Int32(frame->precond_output, s_precondition_output[3], 0));
		set_task_arg(state, s_task_parameters[1], 1, as_Int32(frame->precond_output, s_precondition_output[3], 1));

		PLNNR_COROUTINE_YIELD(frame, expand_label, 2);

		begin_composite(state, 3, travel_branch_0_expand, s_task_parameters[3]);
		set_composite_arg(state, s_task_parameters[3], 0, as_Int32(frame->precond_output, s_precondition_output[3], 1));
		set_composite_arg(state, s_task_parameters[3], 1, _1);

		frame->flags |= Expansion_Frame::Flags_Expanded;
		PLNNR_COROUTINE_YIELD(frame, expand_label, 3);
	}

	PLNNR_COROUTINE_END();
}
///////////////////////////////////////////////////////////////////////////////
