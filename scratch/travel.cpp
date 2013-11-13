#include <derplanner/runtime/runtime.h>
#include "travel.h"

using namespace plnnr;

#pragma GCC diagnostic ignored "-Wunused-variable"

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

const char* task_name(task_type type) { return task_type_to_name[type]; }

static const char* atom_type_to_name[] =
{
	"start",
	"finish",
	"short_distance",
	"long_distance",
	"airport",
	"<none>",
};

const char* atom_name(atom_type type) { return atom_type_to_name[type]; }

// method root [12:10]
struct p0_state
{
	// s [12:12]
	int _0;
	// f [12:16]
	int _1;
	start_tuple* start_0;
	finish_tuple* finish_1;
	int stage;
};

bool next(p0_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.start_0 = tuple_list::head<start_tuple>(world.atoms[atom_start]); state.start_0 != 0; state.start_0 = state.start_0->next)
	{
		state._0 = state.start_0->_0;

		for (state.finish_1 = tuple_list::head<finish_tuple>(world.atoms[atom_finish]); state.finish_1 != 0; state.finish_1 = state.finish_1->next)
		{
			state._1 = state.finish_1->_0;

			PLNNR_COROUTINE_YIELD(state);
		}
	}

	PLNNR_COROUTINE_END();
}

// method travel [17:10]
struct p1_state
{
	// x [17:12]
	int _0;
	// y [17:13]
	int _1;
	short_distance_tuple* short_distance_0;
	int stage;
};

bool next(p1_state& state, worldstate& world)
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

		PLNNR_COROUTINE_YIELD(state);
	}

	PLNNR_COROUTINE_END();
}

// method travel [20:10]
struct p2_state
{
	// x [20:12]
	int _0;
	// y [20:13]
	int _1;
	long_distance_tuple* long_distance_0;
	int stage;
};

bool next(p2_state& state, worldstate& world)
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

		PLNNR_COROUTINE_YIELD(state);
	}

	PLNNR_COROUTINE_END();
}

// method travel_by_air [25:10]
struct p3_state
{
	// x [25:12]
	int _0;
	// ax [25:13]
	int _1;
	// y [25:17]
	int _2;
	// ay [25:18]
	int _3;
	airport_tuple* airport_0;
	airport_tuple* airport_1;
	int stage;
};

bool next(p3_state& state, worldstate& world)
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

			PLNNR_COROUTINE_YIELD(state);
		}
	}

	PLNNR_COROUTINE_END();
}

bool root_branch_0_expand(planner_state& pstate, void* world)
{
	method_instance* method = pstate.top_method;
	p0_state* precondition = static_cast<p0_state*>(method->precondition);
	worldstate* wstate = static_cast<worldstate*>(world);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push<p0_state>(pstate.mstack);
	precondition->stage = 0;

	method->precondition = precondition;
	method->mrewind = pstate.mstack->top();
	method->trewind = pstate.tstack->top();
	method->jrewind = pstate.journal->top();

	while (next(*precondition, *wstate))
	{
		{
			method_instance* t = push_method(pstate, task_travel, travel_branch_0_expand);
			travel_args* a = push<travel_args>(pstate.mstack);
			a->_0 = precondition->_0;
			a->_1 = precondition->_1;
			t->args = a;
		}

		method->expanded = true;
		PLNNR_COROUTINE_YIELD(*method);
	}

	PLNNR_COROUTINE_END();
}

bool travel_branch_0_expand(planner_state& pstate, void* world)
{
	method_instance* method = pstate.top_method;
	p1_state* precondition = static_cast<p1_state*>(method->precondition);
	worldstate* wstate = static_cast<worldstate*>(world);
	travel_args* method_args = static_cast<travel_args*>(method->args);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push<p1_state>(pstate.mstack);
	precondition->stage = 0;
	precondition->_0 = method_args->_0;
	precondition->_1 = method_args->_1;

	method->precondition = precondition;
	method->mrewind = pstate.mstack->top();
	method->trewind = pstate.tstack->top();
	method->jrewind = pstate.journal->top();

	while (next(*precondition, *wstate))
	{
		{
			task_instance* t = push_task(pstate, task_ride_taxi);
			ride_taxi_args* a = push<ride_taxi_args>(pstate.tstack);
			a->_0 = method_args->_0;
			a->_1 = method_args->_1;
			t->args = a;
		}

		method->expanded = true;
		PLNNR_COROUTINE_YIELD(*method);
	}

	return next_branch(pstate, travel_branch_1_expand, world);
	PLNNR_COROUTINE_END();
}

bool travel_branch_1_expand(planner_state& pstate, void* world)
{
	method_instance* method = pstate.top_method;
	p2_state* precondition = static_cast<p2_state*>(method->precondition);
	worldstate* wstate = static_cast<worldstate*>(world);
	travel_args* method_args = static_cast<travel_args*>(method->args);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push<p2_state>(pstate.mstack);
	precondition->stage = 0;
	precondition->_0 = method_args->_0;
	precondition->_1 = method_args->_1;

	method->precondition = precondition;
	method->mrewind = pstate.mstack->top();
	method->trewind = pstate.tstack->top();
	method->jrewind = pstate.journal->top();

	while (next(*precondition, *wstate))
	{
		{
			method_instance* t = push_method(pstate, task_travel_by_air, travel_by_air_branch_0_expand);
			travel_by_air_args* a = push<travel_by_air_args>(pstate.mstack);
			a->_0 = method_args->_0;
			a->_1 = method_args->_1;
			t->args = a;
		}

		method->expanded = true;
		PLNNR_COROUTINE_YIELD(*method);
	}

	PLNNR_COROUTINE_END();
}

bool travel_by_air_branch_0_expand(planner_state& pstate, void* world)
{
	method_instance* method = pstate.top_method;
	p3_state* precondition = static_cast<p3_state*>(method->precondition);
	worldstate* wstate = static_cast<worldstate*>(world);
	travel_by_air_args* method_args = static_cast<travel_by_air_args*>(method->args);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push<p3_state>(pstate.mstack);
	precondition->stage = 0;
	precondition->_0 = method_args->_0;
	precondition->_2 = method_args->_1;

	method->precondition = precondition;
	method->mrewind = pstate.mstack->top();
	method->trewind = pstate.tstack->top();
	method->jrewind = pstate.journal->top();

	while (next(*precondition, *wstate))
	{
		{
			method_instance* t = push_method(pstate, task_travel, travel_branch_0_expand);
			travel_args* a = push<travel_args>(pstate.mstack);
			a->_0 = method_args->_0;
			a->_1 = precondition->_1;
			t->args = a;
		}

		PLNNR_COROUTINE_YIELD(*method);

		{
			task_instance* t = push_task(pstate, task_fly);
			fly_args* a = push<fly_args>(pstate.tstack);
			a->_0 = precondition->_1;
			a->_1 = precondition->_3;
			t->args = a;
		}

		PLNNR_COROUTINE_YIELD(*method);

		{
			method_instance* t = push_method(pstate, task_travel, travel_branch_0_expand);
			travel_args* a = push<travel_args>(pstate.mstack);
			a->_0 = precondition->_3;
			a->_1 = method_args->_1;
			t->args = a;
		}

		method->expanded = true;
		PLNNR_COROUTINE_YIELD(*method);
	}

	PLNNR_COROUTINE_END();
}

}
