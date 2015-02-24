#include <string.h>
#include "derplanner/runtime/runtime.h"
#include "derplanner/runtime/worldstate.h"
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

#define PLNNR_COROUTINE_BEGIN(state) switch ((state).stage) { case 0:
#define PLNNR_COROUTINE_YIELD(state, label) (state).stage = label; return true; case label:;
#define PLNNR_COROUTINE_END() } return false


static bool root_branch_0_expand(Planning_State*, Expansion_Frame*, Fact_Database*);
static bool travel_branch_0_expand(Planning_State*, Expansion_Frame*, Fact_Database*);
static bool travel_branch_1_expand(Planning_State*, Expansion_Frame*, Fact_Database*);
static bool travel_by_air_branch_0_expand(Planning_State*, Expansion_Frame*, Fact_Database*);

static const char* s_fact_names[] = { "start", "finish", "short_distance", "long_distance", "airport" };
static const char* s_task_names[] = { "!go_by_taxi", "!go_by_plane", "root", "travel", "travel_by_plane" };

static Composite_Task_Expand s_task_expands[] = { root_branch_0_expand, travel_branch_0_expand, travel_by_air_branch_0_expand };

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

const Domain_Info* plnnr_domain_info()
{
	return &s_domain_info;
}

namespace travel {

static const char* atom_type_to_name[] =
{
	"start",
	"finish",
	"short_distance",
	"long_distance",
	"airport",
	"<none>",
};

const char* atom_name(Atom_Type type) { return atom_type_to_name[type]; }

}

namespace travel {

static const char* task_type_to_name[] =
{
	"!ride_taxi",
	"!fly",
	"root",
	"travel",
	"travel_by_air",
	"<none>",
};

const char* task_name(Task_Type type) { return task_type_to_name[type]; }

// method root [12:9]
struct p0_state
{
	// s [12:17]
	int _0;
	// f [12:28]
	int _1;
	start_tuple* start_0;
	finish_tuple* finish_1;
	int stage;
};

bool next(p0_state& state, Worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.start_0 = tuple_list::head<start_tuple>(world.atoms[atom_start]); state.start_0 != 0; state.start_0 = state.start_0->next)
	{
		state._0 = state.start_0->_0;

		for (state.finish_1 = tuple_list::head<finish_tuple>(world.atoms[atom_finish]); state.finish_1 != 0; state.finish_1 = state.finish_1->next)
		{
			state._1 = state.finish_1->_0;

			PLNNR_COROUTINE_YIELD(state, 2);
		}
	}

	PLNNR_COROUTINE_END();
}

struct p0_state
{
	int stage;
	Fact_Handle	start_0;
	Fact_Handle finish_1;
};

// method travel [17:9]
struct p1_state
{
	// x [17:25]
	int _0;
	// y [17:27]
	int _1;
	short_distance_tuple* short_distance_0;
	int stage;
};

bool next(p1_state& state, Worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.short_distance_0 = tuple_list::head<short_distance_tuple>(world.atoms[atom_short_distance]); state.short_distance_0 != 0; state.short_distance_0 = state.short_distance_0->next)
	{
		if (state.short_distance_0->_0 != state._0)
		{
			continue;
		}

		if (state.short_distance_0->_1 != state._1)
		{
			continue;
		}

		PLNNR_COROUTINE_YIELD(state, 1);
	}

	PLNNR_COROUTINE_END();
}

// method travel [20:9]
struct p2_state
{
	// x [20:24]
	int _0;
	// y [20:26]
	int _1;
	long_distance_tuple* long_distance_0;
	int stage;
};

bool next(p2_state& state, Worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.long_distance_0 = tuple_list::head<long_distance_tuple>(world.atoms[atom_long_distance]); state.long_distance_0 != 0; state.long_distance_0 = state.long_distance_0->next)
	{
		if (state.long_distance_0->_0 != state._0)
		{
			continue;
		}

		if (state.long_distance_0->_1 != state._1)
		{
			continue;
		}

		PLNNR_COROUTINE_YIELD(state, 1);
	}

	PLNNR_COROUTINE_END();
}

// method travel_by_air [25:9]
struct p3_state
{
	// x [25:19]
	int _0;
	// ax [25:21]
	int _1;
	// y [25:34]
	int _2;
	// ay [25:36]
	int _3;
	airport_tuple* airport_0;
	airport_tuple* airport_1;
	int stage;
};

bool next(p3_state& state, Worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.airport_0 = tuple_list::head<airport_tuple>(world.atoms[atom_airport]); state.airport_0 != 0; state.airport_0 = state.airport_0->next)
	{
		if (state.airport_0->_0 != state._0)
		{
			continue;
		}

		state._1 = state.airport_0->_1;

		for (state.airport_1 = tuple_list::head<airport_tuple>(world.atoms[atom_airport]); state.airport_1 != 0; state.airport_1 = state.airport_1->next)
		{
			if (state.airport_1->_0 != state._2)
			{
				continue;
			}

			state._3 = state.airport_1->_1;

			PLNNR_COROUTINE_YIELD(state, 2);
		}
	}

	PLNNR_COROUTINE_END();
}

bool root_branch_0_expand(Planning_State* state, Expansion_Frame* frame, Fact_Database* db)
{
	p0_state* precondition = plnnr::precondition<p0_state>(method);
	Worldstate* wstate = static_cast<Worldstate*>(world);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push_precondition<p0_state>(pstate, method);

	while (next(*precondition, *wstate))
	{
		{
			Method_Instance* t = push_method(pstate, task_travel, travel_branch_0_expand);
			travel_args* a = push_arguments<travel_args>(pstate, t);
			a->_0 = precondition->_0;
			a->_1 = precondition->_1;
		}

		method->flags |= method_flags_expanded;
		PLNNR_COROUTINE_YIELD(*method, 1);
	}

	PLNNR_COROUTINE_END();
}

bool travel_branch_0_expand(Planning_State* state, Expansion_Frame* frame, Fact_Database* db)
{
	p1_state* precondition = plnnr::precondition<p1_state>(method);
	travel_args* method_args = plnnr::arguments<travel_args>(method);
	Worldstate* wstate = static_cast<Worldstate*>(world);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push_precondition<p1_state>(pstate, method);
	precondition->_0 = method_args->_0;
	precondition->_1 = method_args->_1;

	while (next(*precondition, *wstate))
	{
		{
			Task_Instance* t = push_task(pstate, task_ride_taxi, 0);
			ride_taxi_args* a = push_arguments<ride_taxi_args>(pstate, t);
			a->_0 = method_args->_0;
			a->_1 = method_args->_1;
		}

		method->flags |= method_flags_expanded;
		PLNNR_COROUTINE_YIELD(*method, 1);
	}

	return expand_next_branch(pstate, travel_branch_1_expand, world);
	PLNNR_COROUTINE_END();
}

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
			Method_Instance* t = push_method(pstate, task_travel, travel_branch_0_expand);
			travel_args* a = push_arguments<travel_args>(pstate, t);
			a->_0 = precondition->_3;
			a->_1 = method_args->_1;
		}

		method->flags |= method_flags_expanded;
		PLNNR_COROUTINE_YIELD(*method, 3);
	}

	PLNNR_COROUTINE_END();
}

}
