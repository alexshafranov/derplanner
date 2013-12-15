#include <derplanner/runtime/runtime.h>
#include "blocks.h"

using namespace plnnr;

#pragma GCC diagnostic ignored "-Wunused-variable"

namespace blocks {

static const char* task_type_to_name[] =
{
	"!putdown",
	"!unstack",
	"!pickup",
	"!stack",
	"solve",
	"mark-all-blocks",
	"mark-block",
	"mark-block-recursive",
	"mark-block-term",
	"find-all-movable",
	"mark-move-type",
	"move-block",
	"check",
	"check2",
	"check3",
	"move-block1",
	"<none>",
};

const char* task_name(task_type type) { return task_type_to_name[type]; }

static const char* atom_type_to_name[] =
{
	"block",
	"on-table",
	"on",
	"clear",
	"goal-on-table",
	"goal-on",
	"goal-clear",
	"holding",
	"dont-move",
	"need-to-move",
	"put-on-table",
	"stack-on-block",
	"<none>",
};

const char* atom_name(atom_type type) { return atom_type_to_name[type]; }

// method solve [43:10]
struct p0_state
{
	int stage;
};

bool next(p0_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	PLNNR_COROUTINE_YIELD(state);

	PLNNR_COROUTINE_END();
}

// method mark-all-blocks [49:14]
struct p1_state
{
	// x [49:16]
	int _0;
	block_tuple* block_0;
	int stage;
};

bool next(p1_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.block_0 = tuple_list::head<block_tuple>(world.atoms[atom_block]); state.block_0 != 0; state.block_0 = state.block_0->next)
	{
		state._0 = state.block_0->_0;

		PLNNR_COROUTINE_YIELD(state);
	}

	PLNNR_COROUTINE_END();
}

// method mark-block [55:10]
struct p2_state
{
	// x [55:14]
	int _0;
	dont_move_tuple* dont_move_0;
	need_to_move_tuple* need_to_move_1;
	int stage;
};

bool next(p2_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.dont_move_0 = tuple_list::head<dont_move_tuple>(world.atoms[atom_dont_move]); state.dont_move_0 != 0; state.dont_move_0 = state.dont_move_0->next)
	{
		if (state.dont_move_0->_0 == state._0)
		{
			break;
		}
	}

	if (state.dont_move_0 == 0)
	{
		for (state.need_to_move_1 = tuple_list::head<need_to_move_tuple>(world.atoms[atom_need_to_move]); state.need_to_move_1 != 0; state.need_to_move_1 = state.need_to_move_1->next)
		{
			if (state.need_to_move_1->_0 == state._0)
			{
				break;
			}
		}

		if (state.need_to_move_1 == 0)
		{
			PLNNR_COROUTINE_YIELD(state);
		}
	}

	PLNNR_COROUTINE_END();
}

// method mark-block [58:10]
struct p3_state
{
	int stage;
};

bool next(p3_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	PLNNR_COROUTINE_YIELD(state);

	PLNNR_COROUTINE_END();
}

// method mark-block-recursive [63:10]
struct p4_state
{
	// x [63:12]
	int _0;
	// w [63:13]
	int _1;
	on_tuple* on_0;
	int stage;
};

bool next(p4_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.on_0 = tuple_list::head<on_tuple>(world.atoms[atom_on]); state.on_0 != 0; state.on_0 = state.on_0->next)
	{
		if (state.on_0->_0 != state._0)
		{
			continue;
		}

		state._1 = state.on_0->_1;

		PLNNR_COROUTINE_YIELD(state);
	}

	PLNNR_COROUTINE_END();
}

// method mark-block-recursive [66:10]
struct p5_state
{
	int stage;
};

bool next(p5_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	PLNNR_COROUTINE_YIELD(state);

	PLNNR_COROUTINE_END();
}

// method mark-block-term [71:10]
struct p6_state
{
	// x [71:12]
	int _0;
	// y [71:13]
	int _1;
	// z [71:18]
	int _2;
	on_tuple* on_0;
	goal_on_tuple* goal_on_1;
	int stage;
};

bool next(p6_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.on_0 = tuple_list::head<on_tuple>(world.atoms[atom_on]); state.on_0 != 0; state.on_0 = state.on_0->next)
	{
		if (state.on_0->_0 != state._0)
		{
			continue;
		}

		state._1 = state.on_0->_1;

		for (state.goal_on_1 = tuple_list::head<goal_on_tuple>(world.atoms[atom_goal_on]); state.goal_on_1 != 0; state.goal_on_1 = state.goal_on_1->next)
		{
			if (state.goal_on_1->_0 != state._0)
			{
				continue;
			}

			state._2 = state.goal_on_1->_1;

			if (state._1 != state._2)
			{
				PLNNR_COROUTINE_YIELD(state);
			}
		}
	}

	PLNNR_COROUTINE_END();
}

// method mark-block-term [74:10]
struct p7_state
{
	// x [74:12]
	int _0;
	// z [74:17]
	int _1;
	on_table_tuple* on_table_0;
	goal_on_tuple* goal_on_1;
	int stage;
};

bool next(p7_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.on_table_0 = tuple_list::head<on_table_tuple>(world.atoms[atom_on_table]); state.on_table_0 != 0; state.on_table_0 = state.on_table_0->next)
	{
		if (state.on_table_0->_0 != state._0)
		{
			continue;
		}

		for (state.goal_on_1 = tuple_list::head<goal_on_tuple>(world.atoms[atom_goal_on]); state.goal_on_1 != 0; state.goal_on_1 = state.goal_on_1->next)
		{
			if (state.goal_on_1->_0 != state._0)
			{
				continue;
			}

			state._1 = state.goal_on_1->_1;

			PLNNR_COROUTINE_YIELD(state);
		}
	}

	PLNNR_COROUTINE_END();
}

// method mark-block-term [77:10]
struct p8_state
{
	// x [77:12]
	int _0;
	// y [77:13]
	int _1;
	on_tuple* on_0;
	goal_on_table_tuple* goal_on_table_1;
	int stage;
};

bool next(p8_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.on_0 = tuple_list::head<on_tuple>(world.atoms[atom_on]); state.on_0 != 0; state.on_0 = state.on_0->next)
	{
		if (state.on_0->_0 != state._0)
		{
			continue;
		}

		state._1 = state.on_0->_1;

		for (state.goal_on_table_1 = tuple_list::head<goal_on_table_tuple>(world.atoms[atom_goal_on_table]); state.goal_on_table_1 != 0; state.goal_on_table_1 = state.goal_on_table_1->next)
		{
			if (state.goal_on_table_1->_0 != state._0)
			{
				continue;
			}

			PLNNR_COROUTINE_YIELD(state);
		}
	}

	PLNNR_COROUTINE_END();
}

// method mark-block-term [80:10]
struct p9_state
{
	// x [80:12]
	int _0;
	// y [80:13]
	int _1;
	on_tuple* on_0;
	goal_clear_tuple* goal_clear_1;
	int stage;
};

bool next(p9_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.on_0 = tuple_list::head<on_tuple>(world.atoms[atom_on]); state.on_0 != 0; state.on_0 = state.on_0->next)
	{
		if (state.on_0->_0 != state._0)
		{
			continue;
		}

		state._1 = state.on_0->_1;

		for (state.goal_clear_1 = tuple_list::head<goal_clear_tuple>(world.atoms[atom_goal_clear]); state.goal_clear_1 != 0; state.goal_clear_1 = state.goal_clear_1->next)
		{
			if (state.goal_clear_1->_0 != state._1)
			{
				continue;
			}

			PLNNR_COROUTINE_YIELD(state);
		}
	}

	PLNNR_COROUTINE_END();
}

// method mark-block-term [83:10]
struct p10_state
{
	// x [83:12]
	int _0;
	// z [83:13]
	int _1;
	// y [83:17]
	int _2;
	on_tuple* on_0;
	goal_on_tuple* goal_on_1;
	int stage;
};

bool next(p10_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.on_0 = tuple_list::head<on_tuple>(world.atoms[atom_on]); state.on_0 != 0; state.on_0 = state.on_0->next)
	{
		if (state.on_0->_0 != state._0)
		{
			continue;
		}

		state._1 = state.on_0->_1;

		for (state.goal_on_1 = tuple_list::head<goal_on_tuple>(world.atoms[atom_goal_on]); state.goal_on_1 != 0; state.goal_on_1 = state.goal_on_1->next)
		{
			if (state.goal_on_1->_1 != state._1)
			{
				continue;
			}

			state._2 = state.goal_on_1->_0;

			if (state._0 != state._2)
			{
				PLNNR_COROUTINE_YIELD(state);
			}
		}
	}

	PLNNR_COROUTINE_END();
}

// method mark-block-term [86:10]
struct p11_state
{
	// x [86:12]
	int _0;
	// w [86:13]
	int _1;
	on_tuple* on_0;
	need_to_move_tuple* need_to_move_1;
	int stage;
};

bool next(p11_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.on_0 = tuple_list::head<on_tuple>(world.atoms[atom_on]); state.on_0 != 0; state.on_0 = state.on_0->next)
	{
		if (state.on_0->_0 != state._0)
		{
			continue;
		}

		state._1 = state.on_0->_1;

		for (state.need_to_move_1 = tuple_list::head<need_to_move_tuple>(world.atoms[atom_need_to_move]); state.need_to_move_1 != 0; state.need_to_move_1 = state.need_to_move_1->next)
		{
			if (state.need_to_move_1->_0 != state._1)
			{
				continue;
			}

			PLNNR_COROUTINE_YIELD(state);
		}
	}

	PLNNR_COROUTINE_END();
}

// method mark-block-term [89:10]
struct p12_state
{
	int stage;
};

bool next(p12_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	PLNNR_COROUTINE_YIELD(state);

	PLNNR_COROUTINE_END();
}

// method find-all-movable [95:14]
struct p13_state
{
	// x [95:16]
	int _0;
	clear_tuple* clear_0;
	need_to_move_tuple* need_to_move_1;
	int stage;
};

bool next(p13_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.clear_0 = tuple_list::head<clear_tuple>(world.atoms[atom_clear]); state.clear_0 != 0; state.clear_0 = state.clear_0->next)
	{
		state._0 = state.clear_0->_0;

		for (state.need_to_move_1 = tuple_list::head<need_to_move_tuple>(world.atoms[atom_need_to_move]); state.need_to_move_1 != 0; state.need_to_move_1 = state.need_to_move_1->next)
		{
			if (state.need_to_move_1->_0 != state._0)
			{
				continue;
			}

			PLNNR_COROUTINE_YIELD(state);
		}
	}

	PLNNR_COROUTINE_END();
}

// method mark-move-type [101:10]
struct p14_state
{
	// x [101:12]
	int _0;
	goal_on_table_tuple* goal_on_table_0;
	put_on_table_tuple* put_on_table_1;
	int stage;
};

bool next(p14_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.goal_on_table_0 = tuple_list::head<goal_on_table_tuple>(world.atoms[atom_goal_on_table]); state.goal_on_table_0 != 0; state.goal_on_table_0 = state.goal_on_table_0->next)
	{
		if (state.goal_on_table_0->_0 != state._0)
		{
			continue;
		}

		for (state.put_on_table_1 = tuple_list::head<put_on_table_tuple>(world.atoms[atom_put_on_table]); state.put_on_table_1 != 0; state.put_on_table_1 = state.put_on_table_1->next)
		{
			if (state.put_on_table_1->_0 == state._0)
			{
				break;
			}
		}

		if (state.put_on_table_1 == 0)
		{
			PLNNR_COROUTINE_YIELD(state);
		}
	}

	PLNNR_COROUTINE_END();
}

// method mark-move-type [104:10]
struct p15_state
{
	// x [104:12]
	int _0;
	// y [104:13]
	int _1;
	goal_on_tuple* goal_on_0;
	stack_on_block_tuple* stack_on_block_1;
	dont_move_tuple* dont_move_2;
	clear_tuple* clear_3;
	int stage;
};

bool next(p15_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.goal_on_0 = tuple_list::head<goal_on_tuple>(world.atoms[atom_goal_on]); state.goal_on_0 != 0; state.goal_on_0 = state.goal_on_0->next)
	{
		if (state.goal_on_0->_0 != state._0)
		{
			continue;
		}

		state._1 = state.goal_on_0->_1;

		for (state.stack_on_block_1 = tuple_list::head<stack_on_block_tuple>(world.atoms[atom_stack_on_block]); state.stack_on_block_1 != 0; state.stack_on_block_1 = state.stack_on_block_1->next)
		{
			if (state.stack_on_block_1->_0 == state._0)
			{
				break;
			}

			if (state.stack_on_block_1->_1 == state._1)
			{
				break;
			}
		}

		if (state.stack_on_block_1 == 0)
		{
			for (state.dont_move_2 = tuple_list::head<dont_move_tuple>(world.atoms[atom_dont_move]); state.dont_move_2 != 0; state.dont_move_2 = state.dont_move_2->next)
			{
				if (state.dont_move_2->_0 != state._1)
				{
					continue;
				}

				for (state.clear_3 = tuple_list::head<clear_tuple>(world.atoms[atom_clear]); state.clear_3 != 0; state.clear_3 = state.clear_3->next)
				{
					if (state.clear_3->_0 != state._1)
					{
						continue;
					}

					PLNNR_COROUTINE_YIELD(state);
				}
			}
		}
	}

	PLNNR_COROUTINE_END();
}

// method mark-move-type [107:10]
struct p16_state
{
	int stage;
};

bool next(p16_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	PLNNR_COROUTINE_YIELD(state);

	PLNNR_COROUTINE_END();
}

// method move-block [112:10]
struct p17_state
{
	// x [112:12]
	int _0;
	// y [112:13]
	int _1;
	stack_on_block_tuple* stack_on_block_0;
	int stage;
};

bool next(p17_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.stack_on_block_0 = tuple_list::head<stack_on_block_tuple>(world.atoms[atom_stack_on_block]); state.stack_on_block_0 != 0; state.stack_on_block_0 = state.stack_on_block_0->next)
	{
		state._0 = state.stack_on_block_0->_0;

		state._1 = state.stack_on_block_0->_1;

		PLNNR_COROUTINE_YIELD(state);
	}

	PLNNR_COROUTINE_END();
}

// method move-block [115:10]
struct p18_state
{
	// x [115:12]
	int _0;
	// y [115:17]
	int _1;
	put_on_table_tuple* put_on_table_0;
	on_tuple* on_1;
	int stage;
};

bool next(p18_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.put_on_table_0 = tuple_list::head<put_on_table_tuple>(world.atoms[atom_put_on_table]); state.put_on_table_0 != 0; state.put_on_table_0 = state.put_on_table_0->next)
	{
		state._0 = state.put_on_table_0->_0;

		for (state.on_1 = tuple_list::head<on_tuple>(world.atoms[atom_on]); state.on_1 != 0; state.on_1 = state.on_1->next)
		{
			if (state.on_1->_0 != state._0)
			{
				continue;
			}

			state._1 = state.on_1->_1;

			PLNNR_COROUTINE_YIELD(state);
		}
	}

	PLNNR_COROUTINE_END();
}

// method move-block [118:10]
struct p19_state
{
	// x [118:12]
	int _0;
	// y [118:21]
	int _1;
	clear_tuple* clear_0;
	need_to_move_tuple* need_to_move_1;
	on_tuple* on_2;
	int stage;
};

bool next(p19_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.clear_0 = tuple_list::head<clear_tuple>(world.atoms[atom_clear]); state.clear_0 != 0; state.clear_0 = state.clear_0->next)
	{
		state._0 = state.clear_0->_0;

		for (state.need_to_move_1 = tuple_list::head<need_to_move_tuple>(world.atoms[atom_need_to_move]); state.need_to_move_1 != 0; state.need_to_move_1 = state.need_to_move_1->next)
		{
			if (state.need_to_move_1->_0 != state._0)
			{
				continue;
			}

			for (state.on_2 = tuple_list::head<on_tuple>(world.atoms[atom_on]); state.on_2 != 0; state.on_2 = state.on_2->next)
			{
				if (state.on_2->_0 != state._0)
				{
					continue;
				}

				state._1 = state.on_2->_1;

				PLNNR_COROUTINE_YIELD(state);
			}
		}
	}

	PLNNR_COROUTINE_END();
}

// method move-block [121:10]
struct p20_state
{
	int stage;
};

bool next(p20_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	PLNNR_COROUTINE_YIELD(state);

	PLNNR_COROUTINE_END();
}

// method check [126:10]
struct p21_state
{
	// y [126:12]
	int _0;
	// x [126:13]
	int _1;
	goal_on_tuple* goal_on_0;
	clear_tuple* clear_1;
	int stage;
};

bool next(p21_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.goal_on_0 = tuple_list::head<goal_on_tuple>(world.atoms[atom_goal_on]); state.goal_on_0 != 0; state.goal_on_0 = state.goal_on_0->next)
	{
		if (state.goal_on_0->_1 != state._1)
		{
			continue;
		}

		state._0 = state.goal_on_0->_0;

		for (state.clear_1 = tuple_list::head<clear_tuple>(world.atoms[atom_clear]); state.clear_1 != 0; state.clear_1 = state.clear_1->next)
		{
			if (state.clear_1->_0 != state._0)
			{
				continue;
			}

			PLNNR_COROUTINE_YIELD(state);
		}
	}

	PLNNR_COROUTINE_END();
}

// method check [129:10]
struct p22_state
{
	int stage;
};

bool next(p22_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	PLNNR_COROUTINE_YIELD(state);

	PLNNR_COROUTINE_END();
}

// method check2 [134:10]
struct p23_state
{
	// x [134:12]
	int _0;
	// y [134:16]
	int _1;
	dont_move_tuple* dont_move_0;
	goal_on_tuple* goal_on_1;
	clear_tuple* clear_2;
	int stage;
};

bool next(p23_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.dont_move_0 = tuple_list::head<dont_move_tuple>(world.atoms[atom_dont_move]); state.dont_move_0 != 0; state.dont_move_0 = state.dont_move_0->next)
	{
		if (state.dont_move_0->_0 != state._0)
		{
			continue;
		}

		for (state.goal_on_1 = tuple_list::head<goal_on_tuple>(world.atoms[atom_goal_on]); state.goal_on_1 != 0; state.goal_on_1 = state.goal_on_1->next)
		{
			if (state.goal_on_1->_1 != state._0)
			{
				continue;
			}

			state._1 = state.goal_on_1->_0;

			for (state.clear_2 = tuple_list::head<clear_tuple>(world.atoms[atom_clear]); state.clear_2 != 0; state.clear_2 = state.clear_2->next)
			{
				if (state.clear_2->_0 != state._1)
				{
					continue;
				}

				PLNNR_COROUTINE_YIELD(state);
			}
		}
	}

	PLNNR_COROUTINE_END();
}

// method check2 [137:10]
struct p24_state
{
	int stage;
};

bool next(p24_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	PLNNR_COROUTINE_YIELD(state);

	PLNNR_COROUTINE_END();
}

// method check3 [142:10]
struct p25_state
{
	// x [142:12]
	int _0;
	dont_move_tuple* dont_move_0;
	int stage;
};

bool next(p25_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.dont_move_0 = tuple_list::head<dont_move_tuple>(world.atoms[atom_dont_move]); state.dont_move_0 != 0; state.dont_move_0 = state.dont_move_0->next)
	{
		if (state.dont_move_0->_0 != state._0)
		{
			continue;
		}

		PLNNR_COROUTINE_YIELD(state);
	}

	PLNNR_COROUTINE_END();
}

// method check3 [145:10]
struct p26_state
{
	// x [145:12]
	int _0;
	// y [145:13]
	int _1;
	goal_on_tuple* goal_on_0;
	clear_tuple* clear_1;
	dont_move_tuple* dont_move_2;
	int stage;
};

bool next(p26_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.goal_on_0 = tuple_list::head<goal_on_tuple>(world.atoms[atom_goal_on]); state.goal_on_0 != 0; state.goal_on_0 = state.goal_on_0->next)
	{
		if (state.goal_on_0->_0 != state._0)
		{
			continue;
		}

		state._1 = state.goal_on_0->_1;

		for (state.clear_1 = tuple_list::head<clear_tuple>(world.atoms[atom_clear]); state.clear_1 != 0; state.clear_1 = state.clear_1->next)
		{
			if (state.clear_1->_0 != state._1)
			{
				continue;
			}

			for (state.dont_move_2 = tuple_list::head<dont_move_tuple>(world.atoms[atom_dont_move]); state.dont_move_2 != 0; state.dont_move_2 = state.dont_move_2->next)
			{
				if (state.dont_move_2->_0 != state._1)
				{
					continue;
				}

				PLNNR_COROUTINE_YIELD(state);
			}
		}
	}

	PLNNR_COROUTINE_END();
}

// method check3 [148:10]
struct p27_state
{
	// x [148:12]
	int _0;
	goal_on_table_tuple* goal_on_table_0;
	int stage;
};

bool next(p27_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.goal_on_table_0 = tuple_list::head<goal_on_table_tuple>(world.atoms[atom_goal_on_table]); state.goal_on_table_0 != 0; state.goal_on_table_0 = state.goal_on_table_0->next)
	{
		if (state.goal_on_table_0->_0 != state._0)
		{
			continue;
		}

		PLNNR_COROUTINE_YIELD(state);
	}

	PLNNR_COROUTINE_END();
}

// method check3 [151:10]
struct p28_state
{
	int stage;
};

bool next(p28_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	PLNNR_COROUTINE_YIELD(state);

	PLNNR_COROUTINE_END();
}

// method move-block1 [156:10]
struct p29_state
{
	// x [156:12]
	int _0;
	// y [156:13]
	int _1;
	on_tuple* on_0;
	int stage;
};

bool next(p29_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.on_0 = tuple_list::head<on_tuple>(world.atoms[atom_on]); state.on_0 != 0; state.on_0 = state.on_0->next)
	{
		if (state.on_0->_0 != state._0)
		{
			continue;
		}

		state._1 = state.on_0->_1;

		PLNNR_COROUTINE_YIELD(state);
	}

	PLNNR_COROUTINE_END();
}

// method move-block1 [159:10]
struct p30_state
{
	int stage;
};

bool next(p30_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	PLNNR_COROUTINE_YIELD(state);

	PLNNR_COROUTINE_END();
}

bool solve_branch_0_expand(method_instance* method, planner_state& pstate, void* world)
{
	p0_state* precondition = plnnr::precondition<p0_state>(method);
	worldstate* wstate = static_cast<worldstate*>(world);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push_precondition<p0_state>(pstate, method);

	while (next(*precondition, *wstate))
	{
		{
			method_instance* t = push_method(pstate, task_mark_all_blocks, mark_all_blocks_branch_0_expand);
		}

		PLNNR_COROUTINE_YIELD(*method);

		if (method->flags & method_flags_failed)
		{
			continue;
		}

		{
			method_instance* t = push_method(pstate, task_find_all_movable, find_all_movable_branch_0_expand);
		}

		PLNNR_COROUTINE_YIELD(*method);

		if (method->flags & method_flags_failed)
		{
			continue;
		}

		{
			method_instance* t = push_method(pstate, task_move_block, move_block_branch_0_expand);
		}

		method->flags |= method_flags_expanded;
		PLNNR_COROUTINE_YIELD(*method);
	}

	PLNNR_COROUTINE_END();
}

bool mark_all_blocks_branch_0_expand(method_instance* method, planner_state& pstate, void* world)
{
	p1_state* precondition = plnnr::precondition<p1_state>(method);
	worldstate* wstate = static_cast<worldstate*>(world);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push_precondition<p1_state>(pstate, method);

	while (next(*precondition, *wstate))
	{
		{
			method_instance* t = push_method(pstate, task_mark_block, mark_block_branch_0_expand);
			mark_block_args* a = push_arguments<mark_block_args>(pstate, t);
			a->_0 = precondition->_0;
		}

		PLNNR_COROUTINE_YIELD(*method);
	}

	if (precondition->stage > 0)
	{
		method->flags |= method_flags_expanded;
		PLNNR_COROUTINE_YIELD(*method);
	}

	PLNNR_COROUTINE_END();
}

bool mark_block_branch_0_expand(method_instance* method, planner_state& pstate, void* world)
{
	p2_state* precondition = plnnr::precondition<p2_state>(method);
	mark_block_args* method_args = plnnr::arguments<mark_block_args>(method);
	worldstate* wstate = static_cast<worldstate*>(world);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push_precondition<p2_state>(pstate, method);
	precondition->_0 = method_args->_0;

	while (next(*precondition, *wstate))
	{
		{
			method_instance* t = push_method(pstate, task_mark_block_recursive, mark_block_recursive_branch_0_expand);
			mark_block_recursive_args* a = push_arguments<mark_block_recursive_args>(pstate, t);
			a->_0 = method_args->_0;
		}

		method->flags |= method_flags_expanded;
		PLNNR_COROUTINE_YIELD(*method);
	}

	return expand_next_branch(pstate, mark_block_branch_1_expand, world);
	PLNNR_COROUTINE_END();
}

bool mark_block_branch_1_expand(method_instance* method, planner_state& pstate, void* world)
{
	p3_state* precondition = plnnr::precondition<p3_state>(method);
	mark_block_args* method_args = plnnr::arguments<mark_block_args>(method);
	worldstate* wstate = static_cast<worldstate*>(world);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push_precondition<p3_state>(pstate, method);

	while (next(*precondition, *wstate))
	{
		method->flags |= method_flags_expanded;
		PLNNR_COROUTINE_YIELD(*method);
	}

	PLNNR_COROUTINE_END();
}

bool mark_block_recursive_branch_0_expand(method_instance* method, planner_state& pstate, void* world)
{
	p4_state* precondition = plnnr::precondition<p4_state>(method);
	mark_block_recursive_args* method_args = plnnr::arguments<mark_block_recursive_args>(method);
	worldstate* wstate = static_cast<worldstate*>(world);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push_precondition<p4_state>(pstate, method);
	precondition->_0 = method_args->_0;

	while (next(*precondition, *wstate))
	{
		{
			method_instance* t = push_method(pstate, task_mark_block_recursive, mark_block_recursive_branch_0_expand);
			mark_block_recursive_args* a = push_arguments<mark_block_recursive_args>(pstate, t);
			a->_0 = precondition->_1;
		}

		PLNNR_COROUTINE_YIELD(*method);

		if (method->flags & method_flags_failed)
		{
			continue;
		}

		{
			method_instance* t = push_method(pstate, task_mark_block_term, mark_block_term_branch_0_expand);
			mark_block_term_args* a = push_arguments<mark_block_term_args>(pstate, t);
			a->_0 = method_args->_0;
		}

		method->flags |= method_flags_expanded;
		PLNNR_COROUTINE_YIELD(*method);
	}

	return expand_next_branch(pstate, mark_block_recursive_branch_1_expand, world);
	PLNNR_COROUTINE_END();
}

bool mark_block_recursive_branch_1_expand(method_instance* method, planner_state& pstate, void* world)
{
	p5_state* precondition = plnnr::precondition<p5_state>(method);
	mark_block_recursive_args* method_args = plnnr::arguments<mark_block_recursive_args>(method);
	worldstate* wstate = static_cast<worldstate*>(world);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push_precondition<p5_state>(pstate, method);

	while (next(*precondition, *wstate))
	{
		{
			method_instance* t = push_method(pstate, task_mark_block_term, mark_block_term_branch_0_expand);
			mark_block_term_args* a = push_arguments<mark_block_term_args>(pstate, t);
			a->_0 = method_args->_0;
		}

		method->flags |= method_flags_expanded;
		PLNNR_COROUTINE_YIELD(*method);
	}

	PLNNR_COROUTINE_END();
}

bool mark_block_term_branch_0_expand(method_instance* method, planner_state& pstate, void* world)
{
	p6_state* precondition = plnnr::precondition<p6_state>(method);
	mark_block_term_args* method_args = plnnr::arguments<mark_block_term_args>(method);
	worldstate* wstate = static_cast<worldstate*>(world);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push_precondition<p6_state>(pstate, method);
	precondition->_0 = method_args->_0;

	while (next(*precondition, *wstate))
	{
		{
			{
				tuple_list::handle* list = wstate->atoms[atom_need_to_move];
				need_to_move_tuple* tuple = tuple_list::append<need_to_move_tuple>(list);
				tuple->_0 = method_args->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		method->flags |= method_flags_expanded;
		PLNNR_COROUTINE_YIELD(*method);
	}

	return expand_next_branch(pstate, mark_block_term_branch_1_expand, world);
	PLNNR_COROUTINE_END();
}

bool mark_block_term_branch_1_expand(method_instance* method, planner_state& pstate, void* world)
{
	p7_state* precondition = plnnr::precondition<p7_state>(method);
	mark_block_term_args* method_args = plnnr::arguments<mark_block_term_args>(method);
	worldstate* wstate = static_cast<worldstate*>(world);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push_precondition<p7_state>(pstate, method);
	precondition->_0 = method_args->_0;

	while (next(*precondition, *wstate))
	{
		{
			{
				tuple_list::handle* list = wstate->atoms[atom_need_to_move];
				need_to_move_tuple* tuple = tuple_list::append<need_to_move_tuple>(list);
				tuple->_0 = method_args->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		method->flags |= method_flags_expanded;
		PLNNR_COROUTINE_YIELD(*method);
	}

	return expand_next_branch(pstate, mark_block_term_branch_2_expand, world);
	PLNNR_COROUTINE_END();
}

bool mark_block_term_branch_2_expand(method_instance* method, planner_state& pstate, void* world)
{
	p8_state* precondition = plnnr::precondition<p8_state>(method);
	mark_block_term_args* method_args = plnnr::arguments<mark_block_term_args>(method);
	worldstate* wstate = static_cast<worldstate*>(world);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push_precondition<p8_state>(pstate, method);
	precondition->_0 = method_args->_0;

	while (next(*precondition, *wstate))
	{
		{
			{
				tuple_list::handle* list = wstate->atoms[atom_need_to_move];
				need_to_move_tuple* tuple = tuple_list::append<need_to_move_tuple>(list);
				tuple->_0 = method_args->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		method->flags |= method_flags_expanded;
		PLNNR_COROUTINE_YIELD(*method);
	}

	return expand_next_branch(pstate, mark_block_term_branch_3_expand, world);
	PLNNR_COROUTINE_END();
}

bool mark_block_term_branch_3_expand(method_instance* method, planner_state& pstate, void* world)
{
	p9_state* precondition = plnnr::precondition<p9_state>(method);
	mark_block_term_args* method_args = plnnr::arguments<mark_block_term_args>(method);
	worldstate* wstate = static_cast<worldstate*>(world);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push_precondition<p9_state>(pstate, method);
	precondition->_0 = method_args->_0;

	while (next(*precondition, *wstate))
	{
		{
			{
				tuple_list::handle* list = wstate->atoms[atom_need_to_move];
				need_to_move_tuple* tuple = tuple_list::append<need_to_move_tuple>(list);
				tuple->_0 = method_args->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		method->flags |= method_flags_expanded;
		PLNNR_COROUTINE_YIELD(*method);
	}

	return expand_next_branch(pstate, mark_block_term_branch_4_expand, world);
	PLNNR_COROUTINE_END();
}

bool mark_block_term_branch_4_expand(method_instance* method, planner_state& pstate, void* world)
{
	p10_state* precondition = plnnr::precondition<p10_state>(method);
	mark_block_term_args* method_args = plnnr::arguments<mark_block_term_args>(method);
	worldstate* wstate = static_cast<worldstate*>(world);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push_precondition<p10_state>(pstate, method);
	precondition->_0 = method_args->_0;

	while (next(*precondition, *wstate))
	{
		{
			{
				tuple_list::handle* list = wstate->atoms[atom_need_to_move];
				need_to_move_tuple* tuple = tuple_list::append<need_to_move_tuple>(list);
				tuple->_0 = method_args->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		method->flags |= method_flags_expanded;
		PLNNR_COROUTINE_YIELD(*method);
	}

	return expand_next_branch(pstate, mark_block_term_branch_5_expand, world);
	PLNNR_COROUTINE_END();
}

bool mark_block_term_branch_5_expand(method_instance* method, planner_state& pstate, void* world)
{
	p11_state* precondition = plnnr::precondition<p11_state>(method);
	mark_block_term_args* method_args = plnnr::arguments<mark_block_term_args>(method);
	worldstate* wstate = static_cast<worldstate*>(world);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push_precondition<p11_state>(pstate, method);
	precondition->_0 = method_args->_0;

	while (next(*precondition, *wstate))
	{
		{
			{
				tuple_list::handle* list = wstate->atoms[atom_need_to_move];
				need_to_move_tuple* tuple = tuple_list::append<need_to_move_tuple>(list);
				tuple->_0 = method_args->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		method->flags |= method_flags_expanded;
		PLNNR_COROUTINE_YIELD(*method);
	}

	return expand_next_branch(pstate, mark_block_term_branch_6_expand, world);
	PLNNR_COROUTINE_END();
}

bool mark_block_term_branch_6_expand(method_instance* method, planner_state& pstate, void* world)
{
	p12_state* precondition = plnnr::precondition<p12_state>(method);
	mark_block_term_args* method_args = plnnr::arguments<mark_block_term_args>(method);
	worldstate* wstate = static_cast<worldstate*>(world);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push_precondition<p12_state>(pstate, method);

	while (next(*precondition, *wstate))
	{
		{
			{
				tuple_list::handle* list = wstate->atoms[atom_dont_move];
				dont_move_tuple* tuple = tuple_list::append<dont_move_tuple>(list);
				tuple->_0 = method_args->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		method->flags |= method_flags_expanded;
		PLNNR_COROUTINE_YIELD(*method);
	}

	PLNNR_COROUTINE_END();
}

bool find_all_movable_branch_0_expand(method_instance* method, planner_state& pstate, void* world)
{
	p13_state* precondition = plnnr::precondition<p13_state>(method);
	worldstate* wstate = static_cast<worldstate*>(world);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push_precondition<p13_state>(pstate, method);

	while (next(*precondition, *wstate))
	{
		{
			method_instance* t = push_method(pstate, task_mark_move_type, mark_move_type_branch_0_expand);
			mark_move_type_args* a = push_arguments<mark_move_type_args>(pstate, t);
			a->_0 = precondition->_0;
		}

		PLNNR_COROUTINE_YIELD(*method);
	}

	if (precondition->stage > 0)
	{
		method->flags |= method_flags_expanded;
		PLNNR_COROUTINE_YIELD(*method);
	}

	PLNNR_COROUTINE_END();
}

bool mark_move_type_branch_0_expand(method_instance* method, planner_state& pstate, void* world)
{
	p14_state* precondition = plnnr::precondition<p14_state>(method);
	mark_move_type_args* method_args = plnnr::arguments<mark_move_type_args>(method);
	worldstate* wstate = static_cast<worldstate*>(world);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push_precondition<p14_state>(pstate, method);
	precondition->_0 = method_args->_0;

	while (next(*precondition, *wstate))
	{
		{
			{
				tuple_list::handle* list = wstate->atoms[atom_put_on_table];
				put_on_table_tuple* tuple = tuple_list::append<put_on_table_tuple>(list);
				tuple->_0 = method_args->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		method->flags |= method_flags_expanded;
		PLNNR_COROUTINE_YIELD(*method);
	}

	return expand_next_branch(pstate, mark_move_type_branch_1_expand, world);
	PLNNR_COROUTINE_END();
}

bool mark_move_type_branch_1_expand(method_instance* method, planner_state& pstate, void* world)
{
	p15_state* precondition = plnnr::precondition<p15_state>(method);
	mark_move_type_args* method_args = plnnr::arguments<mark_move_type_args>(method);
	worldstate* wstate = static_cast<worldstate*>(world);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push_precondition<p15_state>(pstate, method);
	precondition->_0 = method_args->_0;

	while (next(*precondition, *wstate))
	{
		{
			{
				tuple_list::handle* list = wstate->atoms[atom_stack_on_block];
				stack_on_block_tuple* tuple = tuple_list::append<stack_on_block_tuple>(list);
				tuple->_0 = method_args->_0;
				tuple->_1 = precondition->_1;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		method->flags |= method_flags_expanded;
		PLNNR_COROUTINE_YIELD(*method);
	}

	return expand_next_branch(pstate, mark_move_type_branch_2_expand, world);
	PLNNR_COROUTINE_END();
}

bool mark_move_type_branch_2_expand(method_instance* method, planner_state& pstate, void* world)
{
	p16_state* precondition = plnnr::precondition<p16_state>(method);
	mark_move_type_args* method_args = plnnr::arguments<mark_move_type_args>(method);
	worldstate* wstate = static_cast<worldstate*>(world);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push_precondition<p16_state>(pstate, method);

	while (next(*precondition, *wstate))
	{
		method->flags |= method_flags_expanded;
		PLNNR_COROUTINE_YIELD(*method);
	}

	PLNNR_COROUTINE_END();
}

bool move_block_branch_0_expand(method_instance* method, planner_state& pstate, void* world)
{
	p17_state* precondition = plnnr::precondition<p17_state>(method);
	worldstate* wstate = static_cast<worldstate*>(world);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push_precondition<p17_state>(pstate, method);

	while (next(*precondition, *wstate))
	{
		{
			method_instance* t = push_method(pstate, task_move_block1, move_block1_branch_0_expand);
			move_block1_args* a = push_arguments<move_block1_args>(pstate, t);
			a->_0 = precondition->_0;
			a->_1 = precondition->_1;
		}

		PLNNR_COROUTINE_YIELD(*method);

		if (method->flags & method_flags_failed)
		{
			continue;
		}

		{
			method_instance* t = push_method(pstate, task_move_block, move_block_branch_0_expand);
		}

		method->flags |= method_flags_expanded;
		PLNNR_COROUTINE_YIELD(*method);
	}

	return expand_next_branch(pstate, move_block_branch_1_expand, world);
	PLNNR_COROUTINE_END();
}

bool move_block_branch_1_expand(method_instance* method, planner_state& pstate, void* world)
{
	p18_state* precondition = plnnr::precondition<p18_state>(method);
	worldstate* wstate = static_cast<worldstate*>(world);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push_precondition<p18_state>(pstate, method);

	while (next(*precondition, *wstate))
	{
		{
			task_instance* t = push_task(pstate, task_unstack, 0);
			unstack_args* a = push_arguments<unstack_args>(pstate, t);
			a->_0 = precondition->_0;
			a->_1 = precondition->_1;

			for (clear_tuple* tuple = tuple_list::head<clear_tuple>(wstate->atoms[atom_clear]); tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != a->_0)
				{
					continue;
				}

				tuple_list::handle* list = wstate->atoms[atom_clear];
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
				tuple_list::detach(list, tuple);

				break;
			}

			for (on_tuple* tuple = tuple_list::head<on_tuple>(wstate->atoms[atom_on]); tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != a->_0)
				{
					continue;
				}

				if (tuple->_1 != a->_1)
				{
					continue;
				}

				tuple_list::handle* list = wstate->atoms[atom_on];
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
				tuple_list::detach(list, tuple);

				break;
			}

			{
				tuple_list::handle* list = wstate->atoms[atom_holding];
				holding_tuple* tuple = tuple_list::append<holding_tuple>(list);
				tuple->_0 = a->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}

			{
				tuple_list::handle* list = wstate->atoms[atom_clear];
				clear_tuple* tuple = tuple_list::append<clear_tuple>(list);
				tuple->_0 = a->_1;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		PLNNR_COROUTINE_YIELD(*method);

		{
			task_instance* t = push_task(pstate, task_putdown, 0);
			putdown_args* a = push_arguments<putdown_args>(pstate, t);
			a->_0 = precondition->_0;

			for (holding_tuple* tuple = tuple_list::head<holding_tuple>(wstate->atoms[atom_holding]); tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != a->_0)
				{
					continue;
				}

				tuple_list::handle* list = wstate->atoms[atom_holding];
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
				tuple_list::detach(list, tuple);

				break;
			}

			{
				tuple_list::handle* list = wstate->atoms[atom_on_table];
				on_table_tuple* tuple = tuple_list::append<on_table_tuple>(list);
				tuple->_0 = a->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}

			{
				tuple_list::handle* list = wstate->atoms[atom_clear];
				clear_tuple* tuple = tuple_list::append<clear_tuple>(list);
				tuple->_0 = a->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		PLNNR_COROUTINE_YIELD(*method);

		{
			{
				tuple_list::handle* list = wstate->atoms[atom_dont_move];
				dont_move_tuple* tuple = tuple_list::append<dont_move_tuple>(list);
				tuple->_0 = precondition->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		{
			for (need_to_move_tuple* tuple = tuple_list::head<need_to_move_tuple>(wstate->atoms[atom_need_to_move]); tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != precondition->_0)
				{
					continue;
				}

				tuple_list::handle* list = wstate->atoms[atom_need_to_move];
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
				tuple_list::detach(list, tuple);

				break;
			}

			for (put_on_table_tuple* tuple = tuple_list::head<put_on_table_tuple>(wstate->atoms[atom_put_on_table]); tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != precondition->_0)
				{
					continue;
				}

				tuple_list::handle* list = wstate->atoms[atom_put_on_table];
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
				tuple_list::detach(list, tuple);

				break;
			}
		}

		{
			method_instance* t = push_method(pstate, task_check, check_branch_0_expand);
			check_args* a = push_arguments<check_args>(pstate, t);
			a->_0 = precondition->_0;
		}

		PLNNR_COROUTINE_YIELD(*method);

		if (method->flags & method_flags_failed)
		{
			continue;
		}

		{
			method_instance* t = push_method(pstate, task_check2, check2_branch_0_expand);
			check2_args* a = push_arguments<check2_args>(pstate, t);
			a->_0 = precondition->_1;
		}

		PLNNR_COROUTINE_YIELD(*method);

		if (method->flags & method_flags_failed)
		{
			continue;
		}

		{
			method_instance* t = push_method(pstate, task_check3, check3_branch_0_expand);
			check3_args* a = push_arguments<check3_args>(pstate, t);
			a->_0 = precondition->_1;
		}

		PLNNR_COROUTINE_YIELD(*method);

		if (method->flags & method_flags_failed)
		{
			continue;
		}

		{
			method_instance* t = push_method(pstate, task_move_block, move_block_branch_0_expand);
		}

		method->flags |= method_flags_expanded;
		PLNNR_COROUTINE_YIELD(*method);
	}

	return expand_next_branch(pstate, move_block_branch_2_expand, world);
	PLNNR_COROUTINE_END();
}

bool move_block_branch_2_expand(method_instance* method, planner_state& pstate, void* world)
{
	p19_state* precondition = plnnr::precondition<p19_state>(method);
	worldstate* wstate = static_cast<worldstate*>(world);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push_precondition<p19_state>(pstate, method);

	while (next(*precondition, *wstate))
	{
		{
			task_instance* t = push_task(pstate, task_unstack, 0);
			unstack_args* a = push_arguments<unstack_args>(pstate, t);
			a->_0 = precondition->_0;
			a->_1 = precondition->_1;

			for (clear_tuple* tuple = tuple_list::head<clear_tuple>(wstate->atoms[atom_clear]); tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != a->_0)
				{
					continue;
				}

				tuple_list::handle* list = wstate->atoms[atom_clear];
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
				tuple_list::detach(list, tuple);

				break;
			}

			for (on_tuple* tuple = tuple_list::head<on_tuple>(wstate->atoms[atom_on]); tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != a->_0)
				{
					continue;
				}

				if (tuple->_1 != a->_1)
				{
					continue;
				}

				tuple_list::handle* list = wstate->atoms[atom_on];
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
				tuple_list::detach(list, tuple);

				break;
			}

			{
				tuple_list::handle* list = wstate->atoms[atom_holding];
				holding_tuple* tuple = tuple_list::append<holding_tuple>(list);
				tuple->_0 = a->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}

			{
				tuple_list::handle* list = wstate->atoms[atom_clear];
				clear_tuple* tuple = tuple_list::append<clear_tuple>(list);
				tuple->_0 = a->_1;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		PLNNR_COROUTINE_YIELD(*method);

		{
			task_instance* t = push_task(pstate, task_putdown, 0);
			putdown_args* a = push_arguments<putdown_args>(pstate, t);
			a->_0 = precondition->_0;

			for (holding_tuple* tuple = tuple_list::head<holding_tuple>(wstate->atoms[atom_holding]); tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != a->_0)
				{
					continue;
				}

				tuple_list::handle* list = wstate->atoms[atom_holding];
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
				tuple_list::detach(list, tuple);

				break;
			}

			{
				tuple_list::handle* list = wstate->atoms[atom_on_table];
				on_table_tuple* tuple = tuple_list::append<on_table_tuple>(list);
				tuple->_0 = a->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}

			{
				tuple_list::handle* list = wstate->atoms[atom_clear];
				clear_tuple* tuple = tuple_list::append<clear_tuple>(list);
				tuple->_0 = a->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		PLNNR_COROUTINE_YIELD(*method);

		{
			method_instance* t = push_method(pstate, task_check2, check2_branch_0_expand);
			check2_args* a = push_arguments<check2_args>(pstate, t);
			a->_0 = precondition->_1;
		}

		PLNNR_COROUTINE_YIELD(*method);

		if (method->flags & method_flags_failed)
		{
			continue;
		}

		{
			method_instance* t = push_method(pstate, task_check3, check3_branch_0_expand);
			check3_args* a = push_arguments<check3_args>(pstate, t);
			a->_0 = precondition->_1;
		}

		PLNNR_COROUTINE_YIELD(*method);

		if (method->flags & method_flags_failed)
		{
			continue;
		}

		{
			method_instance* t = push_method(pstate, task_move_block, move_block_branch_0_expand);
		}

		method->flags |= method_flags_expanded;
		PLNNR_COROUTINE_YIELD(*method);
	}

	return expand_next_branch(pstate, move_block_branch_3_expand, world);
	PLNNR_COROUTINE_END();
}

bool move_block_branch_3_expand(method_instance* method, planner_state& pstate, void* world)
{
	p20_state* precondition = plnnr::precondition<p20_state>(method);
	worldstate* wstate = static_cast<worldstate*>(world);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push_precondition<p20_state>(pstate, method);

	while (next(*precondition, *wstate))
	{
		method->flags |= method_flags_expanded;
		PLNNR_COROUTINE_YIELD(*method);
	}

	PLNNR_COROUTINE_END();
}

bool check_branch_0_expand(method_instance* method, planner_state& pstate, void* world)
{
	p21_state* precondition = plnnr::precondition<p21_state>(method);
	check_args* method_args = plnnr::arguments<check_args>(method);
	worldstate* wstate = static_cast<worldstate*>(world);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push_precondition<p21_state>(pstate, method);
	precondition->_1 = method_args->_0;

	while (next(*precondition, *wstate))
	{
		{
			{
				tuple_list::handle* list = wstate->atoms[atom_stack_on_block];
				stack_on_block_tuple* tuple = tuple_list::append<stack_on_block_tuple>(list);
				tuple->_0 = precondition->_0;
				tuple->_1 = method_args->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		method->flags |= method_flags_expanded;
		PLNNR_COROUTINE_YIELD(*method);
	}

	return expand_next_branch(pstate, check_branch_1_expand, world);
	PLNNR_COROUTINE_END();
}

bool check_branch_1_expand(method_instance* method, planner_state& pstate, void* world)
{
	p22_state* precondition = plnnr::precondition<p22_state>(method);
	check_args* method_args = plnnr::arguments<check_args>(method);
	worldstate* wstate = static_cast<worldstate*>(world);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push_precondition<p22_state>(pstate, method);

	while (next(*precondition, *wstate))
	{
		method->flags |= method_flags_expanded;
		PLNNR_COROUTINE_YIELD(*method);
	}

	PLNNR_COROUTINE_END();
}

bool check2_branch_0_expand(method_instance* method, planner_state& pstate, void* world)
{
	p23_state* precondition = plnnr::precondition<p23_state>(method);
	check2_args* method_args = plnnr::arguments<check2_args>(method);
	worldstate* wstate = static_cast<worldstate*>(world);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push_precondition<p23_state>(pstate, method);
	precondition->_0 = method_args->_0;

	while (next(*precondition, *wstate))
	{
		{
			{
				tuple_list::handle* list = wstate->atoms[atom_stack_on_block];
				stack_on_block_tuple* tuple = tuple_list::append<stack_on_block_tuple>(list);
				tuple->_0 = precondition->_1;
				tuple->_1 = method_args->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		method->flags |= method_flags_expanded;
		PLNNR_COROUTINE_YIELD(*method);
	}

	return expand_next_branch(pstate, check2_branch_1_expand, world);
	PLNNR_COROUTINE_END();
}

bool check2_branch_1_expand(method_instance* method, planner_state& pstate, void* world)
{
	p24_state* precondition = plnnr::precondition<p24_state>(method);
	check2_args* method_args = plnnr::arguments<check2_args>(method);
	worldstate* wstate = static_cast<worldstate*>(world);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push_precondition<p24_state>(pstate, method);

	while (next(*precondition, *wstate))
	{
		method->flags |= method_flags_expanded;
		PLNNR_COROUTINE_YIELD(*method);
	}

	PLNNR_COROUTINE_END();
}

bool check3_branch_0_expand(method_instance* method, planner_state& pstate, void* world)
{
	p25_state* precondition = plnnr::precondition<p25_state>(method);
	check3_args* method_args = plnnr::arguments<check3_args>(method);
	worldstate* wstate = static_cast<worldstate*>(world);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push_precondition<p25_state>(pstate, method);
	precondition->_0 = method_args->_0;

	while (next(*precondition, *wstate))
	{
		method->flags |= method_flags_expanded;
		PLNNR_COROUTINE_YIELD(*method);
	}

	return expand_next_branch(pstate, check3_branch_1_expand, world);
	PLNNR_COROUTINE_END();
}

bool check3_branch_1_expand(method_instance* method, planner_state& pstate, void* world)
{
	p26_state* precondition = plnnr::precondition<p26_state>(method);
	check3_args* method_args = plnnr::arguments<check3_args>(method);
	worldstate* wstate = static_cast<worldstate*>(world);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push_precondition<p26_state>(pstate, method);
	precondition->_0 = method_args->_0;

	while (next(*precondition, *wstate))
	{
		{
			{
				tuple_list::handle* list = wstate->atoms[atom_stack_on_block];
				stack_on_block_tuple* tuple = tuple_list::append<stack_on_block_tuple>(list);
				tuple->_0 = method_args->_0;
				tuple->_1 = precondition->_1;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		method->flags |= method_flags_expanded;
		PLNNR_COROUTINE_YIELD(*method);
	}

	return expand_next_branch(pstate, check3_branch_2_expand, world);
	PLNNR_COROUTINE_END();
}

bool check3_branch_2_expand(method_instance* method, planner_state& pstate, void* world)
{
	p27_state* precondition = plnnr::precondition<p27_state>(method);
	check3_args* method_args = plnnr::arguments<check3_args>(method);
	worldstate* wstate = static_cast<worldstate*>(world);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push_precondition<p27_state>(pstate, method);
	precondition->_0 = method_args->_0;

	while (next(*precondition, *wstate))
	{
		{
			{
				tuple_list::handle* list = wstate->atoms[atom_put_on_table];
				put_on_table_tuple* tuple = tuple_list::append<put_on_table_tuple>(list);
				tuple->_0 = method_args->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		method->flags |= method_flags_expanded;
		PLNNR_COROUTINE_YIELD(*method);
	}

	return expand_next_branch(pstate, check3_branch_3_expand, world);
	PLNNR_COROUTINE_END();
}

bool check3_branch_3_expand(method_instance* method, planner_state& pstate, void* world)
{
	p28_state* precondition = plnnr::precondition<p28_state>(method);
	check3_args* method_args = plnnr::arguments<check3_args>(method);
	worldstate* wstate = static_cast<worldstate*>(world);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push_precondition<p28_state>(pstate, method);

	while (next(*precondition, *wstate))
	{
		method->flags |= method_flags_expanded;
		PLNNR_COROUTINE_YIELD(*method);
	}

	PLNNR_COROUTINE_END();
}

bool move_block1_branch_0_expand(method_instance* method, planner_state& pstate, void* world)
{
	p29_state* precondition = plnnr::precondition<p29_state>(method);
	move_block1_args* method_args = plnnr::arguments<move_block1_args>(method);
	worldstate* wstate = static_cast<worldstate*>(world);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push_precondition<p29_state>(pstate, method);
	precondition->_0 = method_args->_0;

	while (next(*precondition, *wstate))
	{
		{
			task_instance* t = push_task(pstate, task_unstack, 0);
			unstack_args* a = push_arguments<unstack_args>(pstate, t);
			a->_0 = method_args->_0;
			a->_1 = precondition->_1;

			for (clear_tuple* tuple = tuple_list::head<clear_tuple>(wstate->atoms[atom_clear]); tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != a->_0)
				{
					continue;
				}

				tuple_list::handle* list = wstate->atoms[atom_clear];
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
				tuple_list::detach(list, tuple);

				break;
			}

			for (on_tuple* tuple = tuple_list::head<on_tuple>(wstate->atoms[atom_on]); tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != a->_0)
				{
					continue;
				}

				if (tuple->_1 != a->_1)
				{
					continue;
				}

				tuple_list::handle* list = wstate->atoms[atom_on];
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
				tuple_list::detach(list, tuple);

				break;
			}

			{
				tuple_list::handle* list = wstate->atoms[atom_holding];
				holding_tuple* tuple = tuple_list::append<holding_tuple>(list);
				tuple->_0 = a->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}

			{
				tuple_list::handle* list = wstate->atoms[atom_clear];
				clear_tuple* tuple = tuple_list::append<clear_tuple>(list);
				tuple->_0 = a->_1;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		PLNNR_COROUTINE_YIELD(*method);

		{
			task_instance* t = push_task(pstate, task_stack, 0);
			stack_args* a = push_arguments<stack_args>(pstate, t);
			a->_0 = method_args->_0;
			a->_1 = method_args->_1;

			for (holding_tuple* tuple = tuple_list::head<holding_tuple>(wstate->atoms[atom_holding]); tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != a->_0)
				{
					continue;
				}

				tuple_list::handle* list = wstate->atoms[atom_holding];
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
				tuple_list::detach(list, tuple);

				break;
			}

			for (clear_tuple* tuple = tuple_list::head<clear_tuple>(wstate->atoms[atom_clear]); tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != a->_1)
				{
					continue;
				}

				tuple_list::handle* list = wstate->atoms[atom_clear];
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
				tuple_list::detach(list, tuple);

				break;
			}

			{
				tuple_list::handle* list = wstate->atoms[atom_on];
				on_tuple* tuple = tuple_list::append<on_tuple>(list);
				tuple->_0 = a->_0;
				tuple->_1 = a->_1;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}

			{
				tuple_list::handle* list = wstate->atoms[atom_clear];
				clear_tuple* tuple = tuple_list::append<clear_tuple>(list);
				tuple->_0 = a->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		PLNNR_COROUTINE_YIELD(*method);

		{
			{
				tuple_list::handle* list = wstate->atoms[atom_dont_move];
				dont_move_tuple* tuple = tuple_list::append<dont_move_tuple>(list);
				tuple->_0 = method_args->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		{
			for (need_to_move_tuple* tuple = tuple_list::head<need_to_move_tuple>(wstate->atoms[atom_need_to_move]); tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != method_args->_0)
				{
					continue;
				}

				tuple_list::handle* list = wstate->atoms[atom_need_to_move];
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
				tuple_list::detach(list, tuple);

				break;
			}

			for (stack_on_block_tuple* tuple = tuple_list::head<stack_on_block_tuple>(wstate->atoms[atom_stack_on_block]); tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != method_args->_0)
				{
					continue;
				}

				if (tuple->_1 != method_args->_1)
				{
					continue;
				}

				tuple_list::handle* list = wstate->atoms[atom_stack_on_block];
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
				tuple_list::detach(list, tuple);

				break;
			}
		}

		{
			method_instance* t = push_method(pstate, task_check, check_branch_0_expand);
			check_args* a = push_arguments<check_args>(pstate, t);
			a->_0 = method_args->_0;
		}

		PLNNR_COROUTINE_YIELD(*method);

		if (method->flags & method_flags_failed)
		{
			continue;
		}

		{
			method_instance* t = push_method(pstate, task_check2, check2_branch_0_expand);
			check2_args* a = push_arguments<check2_args>(pstate, t);
			a->_0 = precondition->_1;
		}

		PLNNR_COROUTINE_YIELD(*method);

		if (method->flags & method_flags_failed)
		{
			continue;
		}

		{
			method_instance* t = push_method(pstate, task_check3, check3_branch_0_expand);
			check3_args* a = push_arguments<check3_args>(pstate, t);
			a->_0 = precondition->_1;
		}

		method->flags |= method_flags_expanded;
		PLNNR_COROUTINE_YIELD(*method);
	}

	return expand_next_branch(pstate, move_block1_branch_1_expand, world);
	PLNNR_COROUTINE_END();
}

bool move_block1_branch_1_expand(method_instance* method, planner_state& pstate, void* world)
{
	p30_state* precondition = plnnr::precondition<p30_state>(method);
	move_block1_args* method_args = plnnr::arguments<move_block1_args>(method);
	worldstate* wstate = static_cast<worldstate*>(world);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push_precondition<p30_state>(pstate, method);

	while (next(*precondition, *wstate))
	{
		{
			task_instance* t = push_task(pstate, task_pickup, 0);
			pickup_args* a = push_arguments<pickup_args>(pstate, t);
			a->_0 = method_args->_0;

			for (clear_tuple* tuple = tuple_list::head<clear_tuple>(wstate->atoms[atom_clear]); tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != a->_0)
				{
					continue;
				}

				tuple_list::handle* list = wstate->atoms[atom_clear];
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
				tuple_list::detach(list, tuple);

				break;
			}

			for (on_table_tuple* tuple = tuple_list::head<on_table_tuple>(wstate->atoms[atom_on_table]); tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != a->_0)
				{
					continue;
				}

				tuple_list::handle* list = wstate->atoms[atom_on_table];
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
				tuple_list::detach(list, tuple);

				break;
			}

			{
				tuple_list::handle* list = wstate->atoms[atom_holding];
				holding_tuple* tuple = tuple_list::append<holding_tuple>(list);
				tuple->_0 = a->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		PLNNR_COROUTINE_YIELD(*method);

		{
			task_instance* t = push_task(pstate, task_stack, 0);
			stack_args* a = push_arguments<stack_args>(pstate, t);
			a->_0 = method_args->_0;
			a->_1 = method_args->_1;

			for (holding_tuple* tuple = tuple_list::head<holding_tuple>(wstate->atoms[atom_holding]); tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != a->_0)
				{
					continue;
				}

				tuple_list::handle* list = wstate->atoms[atom_holding];
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
				tuple_list::detach(list, tuple);

				break;
			}

			for (clear_tuple* tuple = tuple_list::head<clear_tuple>(wstate->atoms[atom_clear]); tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != a->_1)
				{
					continue;
				}

				tuple_list::handle* list = wstate->atoms[atom_clear];
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
				tuple_list::detach(list, tuple);

				break;
			}

			{
				tuple_list::handle* list = wstate->atoms[atom_on];
				on_tuple* tuple = tuple_list::append<on_tuple>(list);
				tuple->_0 = a->_0;
				tuple->_1 = a->_1;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}

			{
				tuple_list::handle* list = wstate->atoms[atom_clear];
				clear_tuple* tuple = tuple_list::append<clear_tuple>(list);
				tuple->_0 = a->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		PLNNR_COROUTINE_YIELD(*method);

		{
			{
				tuple_list::handle* list = wstate->atoms[atom_dont_move];
				dont_move_tuple* tuple = tuple_list::append<dont_move_tuple>(list);
				tuple->_0 = method_args->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		{
			for (need_to_move_tuple* tuple = tuple_list::head<need_to_move_tuple>(wstate->atoms[atom_need_to_move]); tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != method_args->_0)
				{
					continue;
				}

				tuple_list::handle* list = wstate->atoms[atom_need_to_move];
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
				tuple_list::detach(list, tuple);

				break;
			}

			for (stack_on_block_tuple* tuple = tuple_list::head<stack_on_block_tuple>(wstate->atoms[atom_stack_on_block]); tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != method_args->_0)
				{
					continue;
				}

				if (tuple->_1 != method_args->_1)
				{
					continue;
				}

				tuple_list::handle* list = wstate->atoms[atom_stack_on_block];
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
				tuple_list::detach(list, tuple);

				break;
			}
		}

		{
			method_instance* t = push_method(pstate, task_check, check_branch_0_expand);
			check_args* a = push_arguments<check_args>(pstate, t);
			a->_0 = method_args->_0;
		}

		method->flags |= method_flags_expanded;
		PLNNR_COROUTINE_YIELD(*method);
	}

	PLNNR_COROUTINE_END();
}

}
