#include <derplanner/runtime/runtime.h>
#include "blocks.h"

using namespace plnnr;

static const char* task_type_to_name[] =
{
	"<none>",
	"!putdown",
	"!unstack",
	"!pickup",
	"!stack",
};

const char* task_name(task_type type) { return task_type_to_name[type]; }

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

	for (state.block_0 = tuple_list::head<block_tuple>(world.block); state.block_0 != 0; state.block_0 = state.block_0->next)
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

	for (state.dont_move_0 = tuple_list::head<dont_move_tuple>(world.dont_move); state.dont_move_0 != 0; state.dont_move_0 = state.dont_move_0->next)
	{
		if (state.dont_move_0->_0 == state._0)
		{
			break;
		}
	}

	if (state.dont_move_0 == 0)
	{
		for (state.need_to_move_1 = tuple_list::head<need_to_move_tuple>(world.need_to_move); state.need_to_move_1 != 0; state.need_to_move_1 = state.need_to_move_1->next)
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

	for (state.on_0 = tuple_list::head<on_tuple>(world.on); state.on_0 != 0; state.on_0 = state.on_0->next)
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

	for (state.on_0 = tuple_list::head<on_tuple>(world.on); state.on_0 != 0; state.on_0 = state.on_0->next)
	{
		if (state.on_0->_0 != state._0)
		{
			continue;
		}

		state._1 = state.on_0->_1;

		for (state.goal_on_1 = tuple_list::head<goal_on_tuple>(world.goal_on); state.goal_on_1 != 0; state.goal_on_1 = state.goal_on_1->next)
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

	for (state.on_table_0 = tuple_list::head<on_table_tuple>(world.on_table); state.on_table_0 != 0; state.on_table_0 = state.on_table_0->next)
	{
		if (state.on_table_0->_0 != state._0)
		{
			continue;
		}

		for (state.goal_on_1 = tuple_list::head<goal_on_tuple>(world.goal_on); state.goal_on_1 != 0; state.goal_on_1 = state.goal_on_1->next)
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

	for (state.on_0 = tuple_list::head<on_tuple>(world.on); state.on_0 != 0; state.on_0 = state.on_0->next)
	{
		if (state.on_0->_0 != state._0)
		{
			continue;
		}

		state._1 = state.on_0->_1;

		for (state.goal_on_table_1 = tuple_list::head<goal_on_table_tuple>(world.goal_on_table); state.goal_on_table_1 != 0; state.goal_on_table_1 = state.goal_on_table_1->next)
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

	for (state.on_0 = tuple_list::head<on_tuple>(world.on); state.on_0 != 0; state.on_0 = state.on_0->next)
	{
		if (state.on_0->_0 != state._0)
		{
			continue;
		}

		state._1 = state.on_0->_1;

		for (state.goal_clear_1 = tuple_list::head<goal_clear_tuple>(world.goal_clear); state.goal_clear_1 != 0; state.goal_clear_1 = state.goal_clear_1->next)
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

	for (state.on_0 = tuple_list::head<on_tuple>(world.on); state.on_0 != 0; state.on_0 = state.on_0->next)
	{
		if (state.on_0->_0 != state._0)
		{
			continue;
		}

		state._1 = state.on_0->_1;

		for (state.goal_on_1 = tuple_list::head<goal_on_tuple>(world.goal_on); state.goal_on_1 != 0; state.goal_on_1 = state.goal_on_1->next)
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

	for (state.on_0 = tuple_list::head<on_tuple>(world.on); state.on_0 != 0; state.on_0 = state.on_0->next)
	{
		if (state.on_0->_0 != state._0)
		{
			continue;
		}

		state._1 = state.on_0->_1;

		for (state.need_to_move_1 = tuple_list::head<need_to_move_tuple>(world.need_to_move); state.need_to_move_1 != 0; state.need_to_move_1 = state.need_to_move_1->next)
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

// method add-new-goals [94:10]
struct p13_state
{
	// x [94:12]
	int _0;
	// y [94:33]
	int _1;
	block_tuple* block_0;
	dont_move_tuple* dont_move_1;
	goal_on_table_tuple* goal_on_table_2;
	goal_on_tuple* goal_on_3;
	int stage;
};

bool next(p13_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.block_0 = tuple_list::head<block_tuple>(world.block); state.block_0 != 0; state.block_0 = state.block_0->next)
	{
		state._0 = state.block_0->_0;

		for (state.dont_move_1 = tuple_list::head<dont_move_tuple>(world.dont_move); state.dont_move_1 != 0; state.dont_move_1 = state.dont_move_1->next)
		{
			if (state.dont_move_1->_0 == state._0)
			{
				break;
			}
		}

		if (state.dont_move_1 == 0)
		{
			for (state.goal_on_table_2 = tuple_list::head<goal_on_table_tuple>(world.goal_on_table); state.goal_on_table_2 != 0; state.goal_on_table_2 = state.goal_on_table_2->next)
			{
				if (state.goal_on_table_2->_0 == state._0)
				{
					break;
				}
			}

			if (state.goal_on_table_2 == 0)
			{
				for (state.goal_on_3 = tuple_list::head<goal_on_tuple>(world.goal_on); state.goal_on_3 != 0; state.goal_on_3 = state.goal_on_3->next)
				{
					if (state.goal_on_3->_0 == state._0)
					{
						continue;
					}

					state._1 = state.goal_on_3->_1;

					PLNNR_COROUTINE_YIELD(state);
				}
			}
		}
	}

	PLNNR_COROUTINE_END();
}

// method add-new-goals [97:10]
struct p14_state
{
	int stage;
};

bool next(p14_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	PLNNR_COROUTINE_YIELD(state);

	PLNNR_COROUTINE_END();
}

// method find-movable [102:10]
struct p15_state
{
	// x [102:12]
	int _0;
	clear_tuple* clear_0;
	dont_move_tuple* dont_move_1;
	goal_on_table_tuple* goal_on_table_2;
	put_on_table_tuple* put_on_table_3;
	int stage;
};

bool next(p15_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.clear_0 = tuple_list::head<clear_tuple>(world.clear); state.clear_0 != 0; state.clear_0 = state.clear_0->next)
	{
		state._0 = state.clear_0->_0;

		for (state.dont_move_1 = tuple_list::head<dont_move_tuple>(world.dont_move); state.dont_move_1 != 0; state.dont_move_1 = state.dont_move_1->next)
		{
			if (state.dont_move_1->_0 == state._0)
			{
				break;
			}
		}

		if (state.dont_move_1 == 0)
		{
			for (state.goal_on_table_2 = tuple_list::head<goal_on_table_tuple>(world.goal_on_table); state.goal_on_table_2 != 0; state.goal_on_table_2 = state.goal_on_table_2->next)
			{
				if (state.goal_on_table_2->_0 != state._0)
				{
					continue;
				}

				for (state.put_on_table_3 = tuple_list::head<put_on_table_tuple>(world.put_on_table); state.put_on_table_3 != 0; state.put_on_table_3 = state.put_on_table_3->next)
				{
					if (state.put_on_table_3->_0 == state._0)
					{
						break;
					}
				}

				if (state.put_on_table_3 == 0)
				{
					PLNNR_COROUTINE_YIELD(state);
				}
			}
		}
	}

	PLNNR_COROUTINE_END();
}

// method find-movable [105:10]
struct p16_state
{
	// x [105:12]
	int _0;
	// y [105:24]
	int _1;
	clear_tuple* clear_0;
	dont_move_tuple* dont_move_1;
	goal_on_tuple* goal_on_2;
	stack_on_block_tuple* stack_on_block_3;
	dont_move_tuple* dont_move_4;
	clear_tuple* clear_5;
	int stage;
};

bool next(p16_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.clear_0 = tuple_list::head<clear_tuple>(world.clear); state.clear_0 != 0; state.clear_0 = state.clear_0->next)
	{
		state._0 = state.clear_0->_0;

		for (state.dont_move_1 = tuple_list::head<dont_move_tuple>(world.dont_move); state.dont_move_1 != 0; state.dont_move_1 = state.dont_move_1->next)
		{
			if (state.dont_move_1->_0 == state._0)
			{
				break;
			}
		}

		if (state.dont_move_1 == 0)
		{
			for (state.goal_on_2 = tuple_list::head<goal_on_tuple>(world.goal_on); state.goal_on_2 != 0; state.goal_on_2 = state.goal_on_2->next)
			{
				if (state.goal_on_2->_0 != state._0)
				{
					continue;
				}

				state._1 = state.goal_on_2->_1;

				for (state.stack_on_block_3 = tuple_list::head<stack_on_block_tuple>(world.stack_on_block); state.stack_on_block_3 != 0; state.stack_on_block_3 = state.stack_on_block_3->next)
				{
					if (state.stack_on_block_3->_0 == state._0)
					{
						break;
					}

					if (state.stack_on_block_3->_1 == state._1)
					{
						break;
					}
				}

				if (state.stack_on_block_3 == 0)
				{
					for (state.dont_move_4 = tuple_list::head<dont_move_tuple>(world.dont_move); state.dont_move_4 != 0; state.dont_move_4 = state.dont_move_4->next)
					{
						if (state.dont_move_4->_0 != state._1)
						{
							continue;
						}

						for (state.clear_5 = tuple_list::head<clear_tuple>(world.clear); state.clear_5 != 0; state.clear_5 = state.clear_5->next)
						{
							if (state.clear_5->_0 != state._1)
							{
								continue;
							}

							PLNNR_COROUTINE_YIELD(state);
						}
					}
				}
			}
		}
	}

	PLNNR_COROUTINE_END();
}

// method find-movable [108:10]
struct p17_state
{
	int stage;
};

bool next(p17_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	PLNNR_COROUTINE_YIELD(state);

	PLNNR_COROUTINE_END();
}

// method move-block [113:10]
struct p18_state
{
	// x [113:12]
	int _0;
	// y [113:13]
	int _1;
	stack_on_block_tuple* stack_on_block_0;
	int stage;
};

bool next(p18_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.stack_on_block_0 = tuple_list::head<stack_on_block_tuple>(world.stack_on_block); state.stack_on_block_0 != 0; state.stack_on_block_0 = state.stack_on_block_0->next)
	{
		state._0 = state.stack_on_block_0->_0;

		state._1 = state.stack_on_block_0->_1;

		PLNNR_COROUTINE_YIELD(state);
	}

	PLNNR_COROUTINE_END();
}

// method move-block [116:10]
struct p19_state
{
	// x [116:12]
	int _0;
	// y [116:17]
	int _1;
	put_on_table_tuple* put_on_table_0;
	on_tuple* on_1;
	int stage;
};

bool next(p19_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.put_on_table_0 = tuple_list::head<put_on_table_tuple>(world.put_on_table); state.put_on_table_0 != 0; state.put_on_table_0 = state.put_on_table_0->next)
	{
		state._0 = state.put_on_table_0->_0;

		for (state.on_1 = tuple_list::head<on_tuple>(world.on); state.on_1 != 0; state.on_1 = state.on_1->next)
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

// method move-block [119:10]
struct p20_state
{
	// x [119:12]
	int _0;
	// y [119:24]
	int _1;
	clear_tuple* clear_0;
	dont_move_tuple* dont_move_1;
	on_tuple* on_2;
	int stage;
};

bool next(p20_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.clear_0 = tuple_list::head<clear_tuple>(world.clear); state.clear_0 != 0; state.clear_0 = state.clear_0->next)
	{
		state._0 = state.clear_0->_0;

		for (state.dont_move_1 = tuple_list::head<dont_move_tuple>(world.dont_move); state.dont_move_1 != 0; state.dont_move_1 = state.dont_move_1->next)
		{
			if (state.dont_move_1->_0 == state._0)
			{
				break;
			}
		}

		if (state.dont_move_1 == 0)
		{
			for (state.on_2 = tuple_list::head<on_tuple>(world.on); state.on_2 != 0; state.on_2 = state.on_2->next)
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

// method move-block [122:10]
struct p21_state
{
	int stage;
};

bool next(p21_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	PLNNR_COROUTINE_YIELD(state);

	PLNNR_COROUTINE_END();
}

// method check [127:10]
struct p22_state
{
	// y [127:12]
	int _0;
	// x [127:13]
	int _1;
	goal_on_tuple* goal_on_0;
	clear_tuple* clear_1;
	int stage;
};

bool next(p22_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.goal_on_0 = tuple_list::head<goal_on_tuple>(world.goal_on); state.goal_on_0 != 0; state.goal_on_0 = state.goal_on_0->next)
	{
		if (state.goal_on_0->_1 != state._1)
		{
			continue;
		}

		state._0 = state.goal_on_0->_0;

		for (state.clear_1 = tuple_list::head<clear_tuple>(world.clear); state.clear_1 != 0; state.clear_1 = state.clear_1->next)
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

// method check [130:10]
struct p23_state
{
	int stage;
};

bool next(p23_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	PLNNR_COROUTINE_YIELD(state);

	PLNNR_COROUTINE_END();
}

// method check2 [135:10]
struct p24_state
{
	// x [135:12]
	int _0;
	// y [135:16]
	int _1;
	dont_move_tuple* dont_move_0;
	goal_on_tuple* goal_on_1;
	clear_tuple* clear_2;
	int stage;
};

bool next(p24_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.dont_move_0 = tuple_list::head<dont_move_tuple>(world.dont_move); state.dont_move_0 != 0; state.dont_move_0 = state.dont_move_0->next)
	{
		if (state.dont_move_0->_0 != state._0)
		{
			continue;
		}

		for (state.goal_on_1 = tuple_list::head<goal_on_tuple>(world.goal_on); state.goal_on_1 != 0; state.goal_on_1 = state.goal_on_1->next)
		{
			if (state.goal_on_1->_1 != state._0)
			{
				continue;
			}

			state._1 = state.goal_on_1->_0;

			for (state.clear_2 = tuple_list::head<clear_tuple>(world.clear); state.clear_2 != 0; state.clear_2 = state.clear_2->next)
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

// method check2 [138:10]
struct p25_state
{
	int stage;
};

bool next(p25_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	PLNNR_COROUTINE_YIELD(state);

	PLNNR_COROUTINE_END();
}

// method check3 [143:10]
struct p26_state
{
	// x [143:12]
	int _0;
	dont_move_tuple* dont_move_0;
	int stage;
};

bool next(p26_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.dont_move_0 = tuple_list::head<dont_move_tuple>(world.dont_move); state.dont_move_0 != 0; state.dont_move_0 = state.dont_move_0->next)
	{
		if (state.dont_move_0->_0 != state._0)
		{
			continue;
		}

		PLNNR_COROUTINE_YIELD(state);
	}

	PLNNR_COROUTINE_END();
}

// method check3 [146:10]
struct p27_state
{
	// x [146:12]
	int _0;
	// y [146:13]
	int _1;
	goal_on_tuple* goal_on_0;
	clear_tuple* clear_1;
	dont_move_tuple* dont_move_2;
	int stage;
};

bool next(p27_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.goal_on_0 = tuple_list::head<goal_on_tuple>(world.goal_on); state.goal_on_0 != 0; state.goal_on_0 = state.goal_on_0->next)
	{
		if (state.goal_on_0->_0 != state._0)
		{
			continue;
		}

		state._1 = state.goal_on_0->_1;

		for (state.clear_1 = tuple_list::head<clear_tuple>(world.clear); state.clear_1 != 0; state.clear_1 = state.clear_1->next)
		{
			if (state.clear_1->_0 != state._1)
			{
				continue;
			}

			for (state.dont_move_2 = tuple_list::head<dont_move_tuple>(world.dont_move); state.dont_move_2 != 0; state.dont_move_2 = state.dont_move_2->next)
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

// method check3 [149:10]
struct p28_state
{
	// x [149:12]
	int _0;
	goal_on_table_tuple* goal_on_table_0;
	int stage;
};

bool next(p28_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.goal_on_table_0 = tuple_list::head<goal_on_table_tuple>(world.goal_on_table); state.goal_on_table_0 != 0; state.goal_on_table_0 = state.goal_on_table_0->next)
	{
		if (state.goal_on_table_0->_0 != state._0)
		{
			continue;
		}

		PLNNR_COROUTINE_YIELD(state);
	}

	PLNNR_COROUTINE_END();
}

// method check3 [152:10]
struct p29_state
{
	int stage;
};

bool next(p29_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	PLNNR_COROUTINE_YIELD(state);

	PLNNR_COROUTINE_END();
}

// method move-block1 [157:10]
struct p30_state
{
	// x [157:12]
	int _0;
	// y [157:13]
	int _1;
	on_tuple* on_0;
	int stage;
};

bool next(p30_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.on_0 = tuple_list::head<on_tuple>(world.on); state.on_0 != 0; state.on_0 = state.on_0->next)
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

// method move-block1 [160:10]
struct p31_state
{
	int stage;
};

bool next(p31_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	PLNNR_COROUTINE_YIELD(state);

	PLNNR_COROUTINE_END();
}

bool solve_branch_0_expand(planner_state& pstate, void* world)
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
			method_instance* t = push_method(pstate, mark_all_blocks_branch_0_expand);
		}

		PLNNR_COROUTINE_YIELD(*method);

		{
			method_instance* t = push_method(pstate, add_new_goals_branch_0_expand);
		}

		PLNNR_COROUTINE_YIELD(*method);

		{
			method_instance* t = push_method(pstate, find_movable_branch_0_expand);
		}

		PLNNR_COROUTINE_YIELD(*method);

		{
			method_instance* t = push_method(pstate, move_block_branch_0_expand);
		}

		method->expanded = true;
		PLNNR_COROUTINE_YIELD(*method);
	}

	PLNNR_COROUTINE_END();
}

bool mark_all_blocks_branch_0_expand(planner_state& pstate, void* world)
{
	method_instance* method = pstate.top_method;
	p1_state* precondition = static_cast<p1_state*>(method->precondition);
	worldstate* wstate = static_cast<worldstate*>(world);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push<p1_state>(pstate.mstack);
	precondition->stage = 0;

	method->precondition = precondition;
	method->mrewind = pstate.mstack->top();
	method->trewind = pstate.tstack->top();
	method->jrewind = pstate.journal->top();

	while (next(*precondition, *wstate))
	{
		{
			method_instance* t = push_method(pstate, mark_block_branch_0_expand);
			mark_block_args* a = push<mark_block_args>(pstate.mstack);
			a->_0 = precondition->_0;
			t->args = a;
		}

		PLNNR_COROUTINE_YIELD(*method);
	}

	if (precondition->stage > 0)
	{
		method->expanded = true;
		PLNNR_COROUTINE_YIELD(*method);
	}

	PLNNR_COROUTINE_END();
}

bool mark_block_branch_0_expand(planner_state& pstate, void* world)
{
	method_instance* method = pstate.top_method;
	p2_state* precondition = static_cast<p2_state*>(method->precondition);
	worldstate* wstate = static_cast<worldstate*>(world);
	mark_block_args* method_args = static_cast<mark_block_args*>(method->args);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push<p2_state>(pstate.mstack);
	precondition->stage = 0;
	precondition->_0 = method_args->_0;

	method->precondition = precondition;
	method->mrewind = pstate.mstack->top();
	method->trewind = pstate.tstack->top();
	method->jrewind = pstate.journal->top();

	while (next(*precondition, *wstate))
	{
		{
			method_instance* t = push_method(pstate, mark_block_recursive_branch_0_expand);
			mark_block_recursive_args* a = push<mark_block_recursive_args>(pstate.mstack);
			a->_0 = method_args->_0;
			t->args = a;
		}

		method->expanded = true;
		PLNNR_COROUTINE_YIELD(*method);
	}

	return next_branch(pstate, mark_block_branch_1_expand, world);
	PLNNR_COROUTINE_END();
}

bool mark_block_branch_1_expand(planner_state& pstate, void* world)
{
	method_instance* method = pstate.top_method;
	p3_state* precondition = static_cast<p3_state*>(method->precondition);
	worldstate* wstate = static_cast<worldstate*>(world);
	mark_block_args* method_args = static_cast<mark_block_args*>(method->args);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push<p3_state>(pstate.mstack);
	precondition->stage = 0;

	method->precondition = precondition;
	method->mrewind = pstate.mstack->top();
	method->trewind = pstate.tstack->top();
	method->jrewind = pstate.journal->top();

	while (next(*precondition, *wstate))
	{
		method->expanded = true;
		PLNNR_COROUTINE_YIELD(*method);
	}

	PLNNR_COROUTINE_END();
}

bool mark_block_recursive_branch_0_expand(planner_state& pstate, void* world)
{
	method_instance* method = pstate.top_method;
	p4_state* precondition = static_cast<p4_state*>(method->precondition);
	worldstate* wstate = static_cast<worldstate*>(world);
	mark_block_recursive_args* method_args = static_cast<mark_block_recursive_args*>(method->args);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push<p4_state>(pstate.mstack);
	precondition->stage = 0;
	precondition->_0 = method_args->_0;

	method->precondition = precondition;
	method->mrewind = pstate.mstack->top();
	method->trewind = pstate.tstack->top();
	method->jrewind = pstate.journal->top();

	while (next(*precondition, *wstate))
	{
		{
			method_instance* t = push_method(pstate, mark_block_recursive_branch_0_expand);
			mark_block_recursive_args* a = push<mark_block_recursive_args>(pstate.mstack);
			a->_0 = precondition->_1;
			t->args = a;
		}

		PLNNR_COROUTINE_YIELD(*method);

		{
			method_instance* t = push_method(pstate, mark_block_term_branch_0_expand);
			mark_block_term_args* a = push<mark_block_term_args>(pstate.mstack);
			a->_0 = method_args->_0;
			t->args = a;
		}

		method->expanded = true;
		PLNNR_COROUTINE_YIELD(*method);
	}

	return next_branch(pstate, mark_block_recursive_branch_1_expand, world);
	PLNNR_COROUTINE_END();
}

bool mark_block_recursive_branch_1_expand(planner_state& pstate, void* world)
{
	method_instance* method = pstate.top_method;
	p5_state* precondition = static_cast<p5_state*>(method->precondition);
	worldstate* wstate = static_cast<worldstate*>(world);
	mark_block_recursive_args* method_args = static_cast<mark_block_recursive_args*>(method->args);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push<p5_state>(pstate.mstack);
	precondition->stage = 0;

	method->precondition = precondition;
	method->mrewind = pstate.mstack->top();
	method->trewind = pstate.tstack->top();
	method->jrewind = pstate.journal->top();

	while (next(*precondition, *wstate))
	{
		{
			method_instance* t = push_method(pstate, mark_block_term_branch_0_expand);
			mark_block_term_args* a = push<mark_block_term_args>(pstate.mstack);
			a->_0 = method_args->_0;
			t->args = a;
		}

		method->expanded = true;
		PLNNR_COROUTINE_YIELD(*method);
	}

	PLNNR_COROUTINE_END();
}

bool mark_block_term_branch_0_expand(planner_state& pstate, void* world)
{
	method_instance* method = pstate.top_method;
	p6_state* precondition = static_cast<p6_state*>(method->precondition);
	worldstate* wstate = static_cast<worldstate*>(world);
	mark_block_term_args* method_args = static_cast<mark_block_term_args*>(method->args);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push<p6_state>(pstate.mstack);
	precondition->stage = 0;
	precondition->_0 = method_args->_0;

	method->precondition = precondition;
	method->mrewind = pstate.mstack->top();
	method->trewind = pstate.tstack->top();
	method->jrewind = pstate.journal->top();

	while (next(*precondition, *wstate))
	{
		{
			{
				tuple_list::handle* list = wstate->need_to_move;
				need_to_move_tuple* tuple = tuple_list::append<need_to_move_tuple>(list);
				tuple->_0 = method_args->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		method->expanded = true;
		PLNNR_COROUTINE_YIELD(*method);
	}

	return next_branch(pstate, mark_block_term_branch_1_expand, world);
	PLNNR_COROUTINE_END();
}

bool mark_block_term_branch_1_expand(planner_state& pstate, void* world)
{
	method_instance* method = pstate.top_method;
	p7_state* precondition = static_cast<p7_state*>(method->precondition);
	worldstate* wstate = static_cast<worldstate*>(world);
	mark_block_term_args* method_args = static_cast<mark_block_term_args*>(method->args);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push<p7_state>(pstate.mstack);
	precondition->stage = 0;
	precondition->_0 = method_args->_0;

	method->precondition = precondition;
	method->mrewind = pstate.mstack->top();
	method->trewind = pstate.tstack->top();
	method->jrewind = pstate.journal->top();

	while (next(*precondition, *wstate))
	{
		{
			{
				tuple_list::handle* list = wstate->need_to_move;
				need_to_move_tuple* tuple = tuple_list::append<need_to_move_tuple>(list);
				tuple->_0 = method_args->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		method->expanded = true;
		PLNNR_COROUTINE_YIELD(*method);
	}

	return next_branch(pstate, mark_block_term_branch_2_expand, world);
	PLNNR_COROUTINE_END();
}

bool mark_block_term_branch_2_expand(planner_state& pstate, void* world)
{
	method_instance* method = pstate.top_method;
	p8_state* precondition = static_cast<p8_state*>(method->precondition);
	worldstate* wstate = static_cast<worldstate*>(world);
	mark_block_term_args* method_args = static_cast<mark_block_term_args*>(method->args);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push<p8_state>(pstate.mstack);
	precondition->stage = 0;
	precondition->_0 = method_args->_0;

	method->precondition = precondition;
	method->mrewind = pstate.mstack->top();
	method->trewind = pstate.tstack->top();
	method->jrewind = pstate.journal->top();

	while (next(*precondition, *wstate))
	{
		{
			{
				tuple_list::handle* list = wstate->need_to_move;
				need_to_move_tuple* tuple = tuple_list::append<need_to_move_tuple>(list);
				tuple->_0 = method_args->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		method->expanded = true;
		PLNNR_COROUTINE_YIELD(*method);
	}

	return next_branch(pstate, mark_block_term_branch_3_expand, world);
	PLNNR_COROUTINE_END();
}

bool mark_block_term_branch_3_expand(planner_state& pstate, void* world)
{
	method_instance* method = pstate.top_method;
	p9_state* precondition = static_cast<p9_state*>(method->precondition);
	worldstate* wstate = static_cast<worldstate*>(world);
	mark_block_term_args* method_args = static_cast<mark_block_term_args*>(method->args);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push<p9_state>(pstate.mstack);
	precondition->stage = 0;
	precondition->_0 = method_args->_0;

	method->precondition = precondition;
	method->mrewind = pstate.mstack->top();
	method->trewind = pstate.tstack->top();
	method->jrewind = pstate.journal->top();

	while (next(*precondition, *wstate))
	{
		{
			{
				tuple_list::handle* list = wstate->need_to_move;
				need_to_move_tuple* tuple = tuple_list::append<need_to_move_tuple>(list);
				tuple->_0 = method_args->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		method->expanded = true;
		PLNNR_COROUTINE_YIELD(*method);
	}

	return next_branch(pstate, mark_block_term_branch_4_expand, world);
	PLNNR_COROUTINE_END();
}

bool mark_block_term_branch_4_expand(planner_state& pstate, void* world)
{
	method_instance* method = pstate.top_method;
	p10_state* precondition = static_cast<p10_state*>(method->precondition);
	worldstate* wstate = static_cast<worldstate*>(world);
	mark_block_term_args* method_args = static_cast<mark_block_term_args*>(method->args);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push<p10_state>(pstate.mstack);
	precondition->stage = 0;
	precondition->_0 = method_args->_0;

	method->precondition = precondition;
	method->mrewind = pstate.mstack->top();
	method->trewind = pstate.tstack->top();
	method->jrewind = pstate.journal->top();

	while (next(*precondition, *wstate))
	{
		{
			{
				tuple_list::handle* list = wstate->need_to_move;
				need_to_move_tuple* tuple = tuple_list::append<need_to_move_tuple>(list);
				tuple->_0 = method_args->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		method->expanded = true;
		PLNNR_COROUTINE_YIELD(*method);
	}

	return next_branch(pstate, mark_block_term_branch_5_expand, world);
	PLNNR_COROUTINE_END();
}

bool mark_block_term_branch_5_expand(planner_state& pstate, void* world)
{
	method_instance* method = pstate.top_method;
	p11_state* precondition = static_cast<p11_state*>(method->precondition);
	worldstate* wstate = static_cast<worldstate*>(world);
	mark_block_term_args* method_args = static_cast<mark_block_term_args*>(method->args);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push<p11_state>(pstate.mstack);
	precondition->stage = 0;
	precondition->_0 = method_args->_0;

	method->precondition = precondition;
	method->mrewind = pstate.mstack->top();
	method->trewind = pstate.tstack->top();
	method->jrewind = pstate.journal->top();

	while (next(*precondition, *wstate))
	{
		{
			{
				tuple_list::handle* list = wstate->need_to_move;
				need_to_move_tuple* tuple = tuple_list::append<need_to_move_tuple>(list);
				tuple->_0 = method_args->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		method->expanded = true;
		PLNNR_COROUTINE_YIELD(*method);
	}

	return next_branch(pstate, mark_block_term_branch_6_expand, world);
	PLNNR_COROUTINE_END();
}

bool mark_block_term_branch_6_expand(planner_state& pstate, void* world)
{
	method_instance* method = pstate.top_method;
	p12_state* precondition = static_cast<p12_state*>(method->precondition);
	worldstate* wstate = static_cast<worldstate*>(world);
	mark_block_term_args* method_args = static_cast<mark_block_term_args*>(method->args);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push<p12_state>(pstate.mstack);
	precondition->stage = 0;

	method->precondition = precondition;
	method->mrewind = pstate.mstack->top();
	method->trewind = pstate.tstack->top();
	method->jrewind = pstate.journal->top();

	while (next(*precondition, *wstate))
	{
		{
			{
				tuple_list::handle* list = wstate->dont_move;
				dont_move_tuple* tuple = tuple_list::append<dont_move_tuple>(list);
				tuple->_0 = method_args->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		method->expanded = true;
		PLNNR_COROUTINE_YIELD(*method);
	}

	PLNNR_COROUTINE_END();
}

bool add_new_goals_branch_0_expand(planner_state& pstate, void* world)
{
	method_instance* method = pstate.top_method;
	p13_state* precondition = static_cast<p13_state*>(method->precondition);
	worldstate* wstate = static_cast<worldstate*>(world);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push<p13_state>(pstate.mstack);
	precondition->stage = 0;

	method->precondition = precondition;
	method->mrewind = pstate.mstack->top();
	method->trewind = pstate.tstack->top();
	method->jrewind = pstate.journal->top();

	while (next(*precondition, *wstate))
	{
		{
			{
				tuple_list::handle* list = wstate->goal_on_table;
				goal_on_table_tuple* tuple = tuple_list::append<goal_on_table_tuple>(list);
				tuple->_0 = precondition->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		{
			method_instance* t = push_method(pstate, add_new_goals_branch_0_expand);
		}

		method->expanded = true;
		PLNNR_COROUTINE_YIELD(*method);
	}

	return next_branch(pstate, add_new_goals_branch_1_expand, world);
	PLNNR_COROUTINE_END();
}

bool add_new_goals_branch_1_expand(planner_state& pstate, void* world)
{
	method_instance* method = pstate.top_method;
	p14_state* precondition = static_cast<p14_state*>(method->precondition);
	worldstate* wstate = static_cast<worldstate*>(world);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push<p14_state>(pstate.mstack);
	precondition->stage = 0;

	method->precondition = precondition;
	method->mrewind = pstate.mstack->top();
	method->trewind = pstate.tstack->top();
	method->jrewind = pstate.journal->top();

	while (next(*precondition, *wstate))
	{
		method->expanded = true;
		PLNNR_COROUTINE_YIELD(*method);
	}

	PLNNR_COROUTINE_END();
}

bool find_movable_branch_0_expand(planner_state& pstate, void* world)
{
	method_instance* method = pstate.top_method;
	p15_state* precondition = static_cast<p15_state*>(method->precondition);
	worldstate* wstate = static_cast<worldstate*>(world);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push<p15_state>(pstate.mstack);
	precondition->stage = 0;

	method->precondition = precondition;
	method->mrewind = pstate.mstack->top();
	method->trewind = pstate.tstack->top();
	method->jrewind = pstate.journal->top();

	while (next(*precondition, *wstate))
	{
		{
			{
				tuple_list::handle* list = wstate->put_on_table;
				put_on_table_tuple* tuple = tuple_list::append<put_on_table_tuple>(list);
				tuple->_0 = precondition->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		{
			method_instance* t = push_method(pstate, find_movable_branch_0_expand);
		}

		method->expanded = true;
		PLNNR_COROUTINE_YIELD(*method);
	}

	return next_branch(pstate, find_movable_branch_1_expand, world);
	PLNNR_COROUTINE_END();
}

bool find_movable_branch_1_expand(planner_state& pstate, void* world)
{
	method_instance* method = pstate.top_method;
	p16_state* precondition = static_cast<p16_state*>(method->precondition);
	worldstate* wstate = static_cast<worldstate*>(world);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push<p16_state>(pstate.mstack);
	precondition->stage = 0;

	method->precondition = precondition;
	method->mrewind = pstate.mstack->top();
	method->trewind = pstate.tstack->top();
	method->jrewind = pstate.journal->top();

	while (next(*precondition, *wstate))
	{
		{
			{
				tuple_list::handle* list = wstate->stack_on_block;
				stack_on_block_tuple* tuple = tuple_list::append<stack_on_block_tuple>(list);
				tuple->_0 = precondition->_0;
				tuple->_1 = precondition->_1;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		{
			method_instance* t = push_method(pstate, find_movable_branch_0_expand);
		}

		method->expanded = true;
		PLNNR_COROUTINE_YIELD(*method);
	}

	return next_branch(pstate, find_movable_branch_2_expand, world);
	PLNNR_COROUTINE_END();
}

bool find_movable_branch_2_expand(planner_state& pstate, void* world)
{
	method_instance* method = pstate.top_method;
	p17_state* precondition = static_cast<p17_state*>(method->precondition);
	worldstate* wstate = static_cast<worldstate*>(world);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push<p17_state>(pstate.mstack);
	precondition->stage = 0;

	method->precondition = precondition;
	method->mrewind = pstate.mstack->top();
	method->trewind = pstate.tstack->top();
	method->jrewind = pstate.journal->top();

	while (next(*precondition, *wstate))
	{
		method->expanded = true;
		PLNNR_COROUTINE_YIELD(*method);
	}

	PLNNR_COROUTINE_END();
}

bool move_block_branch_0_expand(planner_state& pstate, void* world)
{
	method_instance* method = pstate.top_method;
	p18_state* precondition = static_cast<p18_state*>(method->precondition);
	worldstate* wstate = static_cast<worldstate*>(world);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push<p18_state>(pstate.mstack);
	precondition->stage = 0;

	method->precondition = precondition;
	method->mrewind = pstate.mstack->top();
	method->trewind = pstate.tstack->top();
	method->jrewind = pstate.journal->top();

	while (next(*precondition, *wstate))
	{
		{
			method_instance* t = push_method(pstate, move_block1_branch_0_expand);
			move_block1_args* a = push<move_block1_args>(pstate.mstack);
			a->_0 = precondition->_0;
			a->_1 = precondition->_1;
			t->args = a;
		}

		PLNNR_COROUTINE_YIELD(*method);

		{
			method_instance* t = push_method(pstate, move_block_branch_0_expand);
		}

		method->expanded = true;
		PLNNR_COROUTINE_YIELD(*method);
	}

	return next_branch(pstate, move_block_branch_1_expand, world);
	PLNNR_COROUTINE_END();
}

bool move_block_branch_1_expand(planner_state& pstate, void* world)
{
	method_instance* method = pstate.top_method;
	p19_state* precondition = static_cast<p19_state*>(method->precondition);
	worldstate* wstate = static_cast<worldstate*>(world);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push<p19_state>(pstate.mstack);
	precondition->stage = 0;

	method->precondition = precondition;
	method->mrewind = pstate.mstack->top();
	method->trewind = pstate.tstack->top();
	method->jrewind = pstate.journal->top();

	while (next(*precondition, *wstate))
	{
		{
			task_instance* t = push_task(pstate, task_unstack);
			unstack_args* a = push<unstack_args>(pstate.tstack);
			a->_0 = precondition->_0;
			a->_1 = precondition->_1;
			t->args = a;

			for (clear_tuple* tuple = tuple_list::head<clear_tuple>(wstate->clear); tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != a->_0)
				{
					continue;
				}

				tuple_list::handle* list = wstate->clear;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
				tuple_list::detach(list, tuple);

				break;
			}
			for (on_tuple* tuple = tuple_list::head<on_tuple>(wstate->on); tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != a->_0)
				{
					continue;
				}

				if (tuple->_1 != a->_1)
				{
					continue;
				}

				tuple_list::handle* list = wstate->on;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
				tuple_list::detach(list, tuple);

				break;
			}

			{
				tuple_list::handle* list = wstate->holding;
				holding_tuple* tuple = tuple_list::append<holding_tuple>(list);
				tuple->_0 = a->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}

			{
				tuple_list::handle* list = wstate->clear;
				clear_tuple* tuple = tuple_list::append<clear_tuple>(list);
				tuple->_0 = a->_1;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		{
			task_instance* t = push_task(pstate, task_putdown);
			putdown_args* a = push<putdown_args>(pstate.tstack);
			a->_0 = precondition->_0;
			t->args = a;

			for (holding_tuple* tuple = tuple_list::head<holding_tuple>(wstate->holding); tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != a->_0)
				{
					continue;
				}

				tuple_list::handle* list = wstate->holding;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
				tuple_list::detach(list, tuple);

				break;
			}

			{
				tuple_list::handle* list = wstate->on_table;
				on_table_tuple* tuple = tuple_list::append<on_table_tuple>(list);
				tuple->_0 = a->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}

			{
				tuple_list::handle* list = wstate->clear;
				clear_tuple* tuple = tuple_list::append<clear_tuple>(list);
				tuple->_0 = a->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		{
			{
				tuple_list::handle* list = wstate->dont_move;
				dont_move_tuple* tuple = tuple_list::append<dont_move_tuple>(list);
				tuple->_0 = precondition->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		{
			for (put_on_table_tuple* tuple = tuple_list::head<put_on_table_tuple>(wstate->put_on_table); tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != precondition->_0)
				{
					continue;
				}

				tuple_list::handle* list = wstate->put_on_table;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
				tuple_list::detach(list, tuple);

				break;
			}
		}

		{
			method_instance* t = push_method(pstate, check_branch_0_expand);
			check_args* a = push<check_args>(pstate.mstack);
			a->_0 = precondition->_0;
			t->args = a;
		}

		PLNNR_COROUTINE_YIELD(*method);

		{
			method_instance* t = push_method(pstate, check2_branch_0_expand);
			check2_args* a = push<check2_args>(pstate.mstack);
			a->_0 = precondition->_1;
			t->args = a;
		}

		PLNNR_COROUTINE_YIELD(*method);

		{
			method_instance* t = push_method(pstate, check3_branch_0_expand);
			check3_args* a = push<check3_args>(pstate.mstack);
			a->_0 = precondition->_1;
			t->args = a;
		}

		PLNNR_COROUTINE_YIELD(*method);

		{
			method_instance* t = push_method(pstate, move_block_branch_0_expand);
		}

		method->expanded = true;
		PLNNR_COROUTINE_YIELD(*method);
	}

	return next_branch(pstate, move_block_branch_2_expand, world);
	PLNNR_COROUTINE_END();
}

bool move_block_branch_2_expand(planner_state& pstate, void* world)
{
	method_instance* method = pstate.top_method;
	p20_state* precondition = static_cast<p20_state*>(method->precondition);
	worldstate* wstate = static_cast<worldstate*>(world);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push<p20_state>(pstate.mstack);
	precondition->stage = 0;

	method->precondition = precondition;
	method->mrewind = pstate.mstack->top();
	method->trewind = pstate.tstack->top();
	method->jrewind = pstate.journal->top();

	while (next(*precondition, *wstate))
	{
		{
			task_instance* t = push_task(pstate, task_unstack);
			unstack_args* a = push<unstack_args>(pstate.tstack);
			a->_0 = precondition->_0;
			a->_1 = precondition->_1;
			t->args = a;

			for (clear_tuple* tuple = tuple_list::head<clear_tuple>(wstate->clear); tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != a->_0)
				{
					continue;
				}

				tuple_list::handle* list = wstate->clear;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
				tuple_list::detach(list, tuple);

				break;
			}
			for (on_tuple* tuple = tuple_list::head<on_tuple>(wstate->on); tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != a->_0)
				{
					continue;
				}

				if (tuple->_1 != a->_1)
				{
					continue;
				}

				tuple_list::handle* list = wstate->on;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
				tuple_list::detach(list, tuple);

				break;
			}

			{
				tuple_list::handle* list = wstate->holding;
				holding_tuple* tuple = tuple_list::append<holding_tuple>(list);
				tuple->_0 = a->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}

			{
				tuple_list::handle* list = wstate->clear;
				clear_tuple* tuple = tuple_list::append<clear_tuple>(list);
				tuple->_0 = a->_1;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		{
			task_instance* t = push_task(pstate, task_putdown);
			putdown_args* a = push<putdown_args>(pstate.tstack);
			a->_0 = precondition->_0;
			t->args = a;

			for (holding_tuple* tuple = tuple_list::head<holding_tuple>(wstate->holding); tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != a->_0)
				{
					continue;
				}

				tuple_list::handle* list = wstate->holding;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
				tuple_list::detach(list, tuple);

				break;
			}

			{
				tuple_list::handle* list = wstate->on_table;
				on_table_tuple* tuple = tuple_list::append<on_table_tuple>(list);
				tuple->_0 = a->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}

			{
				tuple_list::handle* list = wstate->clear;
				clear_tuple* tuple = tuple_list::append<clear_tuple>(list);
				tuple->_0 = a->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		{
			method_instance* t = push_method(pstate, check2_branch_0_expand);
			check2_args* a = push<check2_args>(pstate.mstack);
			a->_0 = precondition->_1;
			t->args = a;
		}

		PLNNR_COROUTINE_YIELD(*method);

		{
			method_instance* t = push_method(pstate, check3_branch_0_expand);
			check3_args* a = push<check3_args>(pstate.mstack);
			a->_0 = precondition->_1;
			t->args = a;
		}

		PLNNR_COROUTINE_YIELD(*method);

		{
			method_instance* t = push_method(pstate, move_block_branch_0_expand);
		}

		method->expanded = true;
		PLNNR_COROUTINE_YIELD(*method);
	}

	return next_branch(pstate, move_block_branch_3_expand, world);
	PLNNR_COROUTINE_END();
}

bool move_block_branch_3_expand(planner_state& pstate, void* world)
{
	method_instance* method = pstate.top_method;
	p21_state* precondition = static_cast<p21_state*>(method->precondition);
	worldstate* wstate = static_cast<worldstate*>(world);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push<p21_state>(pstate.mstack);
	precondition->stage = 0;

	method->precondition = precondition;
	method->mrewind = pstate.mstack->top();
	method->trewind = pstate.tstack->top();
	method->jrewind = pstate.journal->top();

	while (next(*precondition, *wstate))
	{
		method->expanded = true;
		PLNNR_COROUTINE_YIELD(*method);
	}

	PLNNR_COROUTINE_END();
}

bool check_branch_0_expand(planner_state& pstate, void* world)
{
	method_instance* method = pstate.top_method;
	p22_state* precondition = static_cast<p22_state*>(method->precondition);
	worldstate* wstate = static_cast<worldstate*>(world);
	check_args* method_args = static_cast<check_args*>(method->args);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push<p22_state>(pstate.mstack);
	precondition->stage = 0;
	precondition->_1 = method_args->_0;

	method->precondition = precondition;
	method->mrewind = pstate.mstack->top();
	method->trewind = pstate.tstack->top();
	method->jrewind = pstate.journal->top();

	while (next(*precondition, *wstate))
	{
		{
			{
				tuple_list::handle* list = wstate->stack_on_block;
				stack_on_block_tuple* tuple = tuple_list::append<stack_on_block_tuple>(list);
				tuple->_0 = precondition->_0;
				tuple->_1 = method_args->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		method->expanded = true;
		PLNNR_COROUTINE_YIELD(*method);
	}

	return next_branch(pstate, check_branch_1_expand, world);
	PLNNR_COROUTINE_END();
}

bool check_branch_1_expand(planner_state& pstate, void* world)
{
	method_instance* method = pstate.top_method;
	p23_state* precondition = static_cast<p23_state*>(method->precondition);
	worldstate* wstate = static_cast<worldstate*>(world);
	check_args* method_args = static_cast<check_args*>(method->args);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push<p23_state>(pstate.mstack);
	precondition->stage = 0;

	method->precondition = precondition;
	method->mrewind = pstate.mstack->top();
	method->trewind = pstate.tstack->top();
	method->jrewind = pstate.journal->top();

	while (next(*precondition, *wstate))
	{
		method->expanded = true;
		PLNNR_COROUTINE_YIELD(*method);
	}

	PLNNR_COROUTINE_END();
}

bool check2_branch_0_expand(planner_state& pstate, void* world)
{
	method_instance* method = pstate.top_method;
	p24_state* precondition = static_cast<p24_state*>(method->precondition);
	worldstate* wstate = static_cast<worldstate*>(world);
	check2_args* method_args = static_cast<check2_args*>(method->args);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push<p24_state>(pstate.mstack);
	precondition->stage = 0;
	precondition->_0 = method_args->_0;

	method->precondition = precondition;
	method->mrewind = pstate.mstack->top();
	method->trewind = pstate.tstack->top();
	method->jrewind = pstate.journal->top();

	while (next(*precondition, *wstate))
	{
		{
			{
				tuple_list::handle* list = wstate->stack_on_block;
				stack_on_block_tuple* tuple = tuple_list::append<stack_on_block_tuple>(list);
				tuple->_0 = precondition->_1;
				tuple->_1 = method_args->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		method->expanded = true;
		PLNNR_COROUTINE_YIELD(*method);
	}

	return next_branch(pstate, check2_branch_1_expand, world);
	PLNNR_COROUTINE_END();
}

bool check2_branch_1_expand(planner_state& pstate, void* world)
{
	method_instance* method = pstate.top_method;
	p25_state* precondition = static_cast<p25_state*>(method->precondition);
	worldstate* wstate = static_cast<worldstate*>(world);
	check2_args* method_args = static_cast<check2_args*>(method->args);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push<p25_state>(pstate.mstack);
	precondition->stage = 0;

	method->precondition = precondition;
	method->mrewind = pstate.mstack->top();
	method->trewind = pstate.tstack->top();
	method->jrewind = pstate.journal->top();

	while (next(*precondition, *wstate))
	{
		method->expanded = true;
		PLNNR_COROUTINE_YIELD(*method);
	}

	PLNNR_COROUTINE_END();
}

bool check3_branch_0_expand(planner_state& pstate, void* world)
{
	method_instance* method = pstate.top_method;
	p26_state* precondition = static_cast<p26_state*>(method->precondition);
	worldstate* wstate = static_cast<worldstate*>(world);
	check3_args* method_args = static_cast<check3_args*>(method->args);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push<p26_state>(pstate.mstack);
	precondition->stage = 0;
	precondition->_0 = method_args->_0;

	method->precondition = precondition;
	method->mrewind = pstate.mstack->top();
	method->trewind = pstate.tstack->top();
	method->jrewind = pstate.journal->top();

	while (next(*precondition, *wstate))
	{
		method->expanded = true;
		PLNNR_COROUTINE_YIELD(*method);
	}

	return next_branch(pstate, check3_branch_1_expand, world);
	PLNNR_COROUTINE_END();
}

bool check3_branch_1_expand(planner_state& pstate, void* world)
{
	method_instance* method = pstate.top_method;
	p27_state* precondition = static_cast<p27_state*>(method->precondition);
	worldstate* wstate = static_cast<worldstate*>(world);
	check3_args* method_args = static_cast<check3_args*>(method->args);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push<p27_state>(pstate.mstack);
	precondition->stage = 0;
	precondition->_0 = method_args->_0;

	method->precondition = precondition;
	method->mrewind = pstate.mstack->top();
	method->trewind = pstate.tstack->top();
	method->jrewind = pstate.journal->top();

	while (next(*precondition, *wstate))
	{
		{
			{
				tuple_list::handle* list = wstate->stack_on_block;
				stack_on_block_tuple* tuple = tuple_list::append<stack_on_block_tuple>(list);
				tuple->_0 = method_args->_0;
				tuple->_1 = precondition->_1;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		method->expanded = true;
		PLNNR_COROUTINE_YIELD(*method);
	}

	return next_branch(pstate, check3_branch_2_expand, world);
	PLNNR_COROUTINE_END();
}

bool check3_branch_2_expand(planner_state& pstate, void* world)
{
	method_instance* method = pstate.top_method;
	p28_state* precondition = static_cast<p28_state*>(method->precondition);
	worldstate* wstate = static_cast<worldstate*>(world);
	check3_args* method_args = static_cast<check3_args*>(method->args);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push<p28_state>(pstate.mstack);
	precondition->stage = 0;
	precondition->_0 = method_args->_0;

	method->precondition = precondition;
	method->mrewind = pstate.mstack->top();
	method->trewind = pstate.tstack->top();
	method->jrewind = pstate.journal->top();

	while (next(*precondition, *wstate))
	{
		{
			{
				tuple_list::handle* list = wstate->put_on_table;
				put_on_table_tuple* tuple = tuple_list::append<put_on_table_tuple>(list);
				tuple->_0 = method_args->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		method->expanded = true;
		PLNNR_COROUTINE_YIELD(*method);
	}

	return next_branch(pstate, check3_branch_3_expand, world);
	PLNNR_COROUTINE_END();
}

bool check3_branch_3_expand(planner_state& pstate, void* world)
{
	method_instance* method = pstate.top_method;
	p29_state* precondition = static_cast<p29_state*>(method->precondition);
	worldstate* wstate = static_cast<worldstate*>(world);
	check3_args* method_args = static_cast<check3_args*>(method->args);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push<p29_state>(pstate.mstack);
	precondition->stage = 0;

	method->precondition = precondition;
	method->mrewind = pstate.mstack->top();
	method->trewind = pstate.tstack->top();
	method->jrewind = pstate.journal->top();

	while (next(*precondition, *wstate))
	{
		method->expanded = true;
		PLNNR_COROUTINE_YIELD(*method);
	}

	PLNNR_COROUTINE_END();
}

bool move_block1_branch_0_expand(planner_state& pstate, void* world)
{
	method_instance* method = pstate.top_method;
	p30_state* precondition = static_cast<p30_state*>(method->precondition);
	worldstate* wstate = static_cast<worldstate*>(world);
	move_block1_args* method_args = static_cast<move_block1_args*>(method->args);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push<p30_state>(pstate.mstack);
	precondition->stage = 0;
	precondition->_0 = method_args->_0;

	method->precondition = precondition;
	method->mrewind = pstate.mstack->top();
	method->trewind = pstate.tstack->top();
	method->jrewind = pstate.journal->top();

	while (next(*precondition, *wstate))
	{
		{
			task_instance* t = push_task(pstate, task_unstack);
			unstack_args* a = push<unstack_args>(pstate.tstack);
			a->_0 = method_args->_0;
			a->_1 = precondition->_1;
			t->args = a;

			for (clear_tuple* tuple = tuple_list::head<clear_tuple>(wstate->clear); tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != a->_0)
				{
					continue;
				}

				tuple_list::handle* list = wstate->clear;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
				tuple_list::detach(list, tuple);

				break;
			}
			for (on_tuple* tuple = tuple_list::head<on_tuple>(wstate->on); tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != a->_0)
				{
					continue;
				}

				if (tuple->_1 != a->_1)
				{
					continue;
				}

				tuple_list::handle* list = wstate->on;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
				tuple_list::detach(list, tuple);

				break;
			}

			{
				tuple_list::handle* list = wstate->holding;
				holding_tuple* tuple = tuple_list::append<holding_tuple>(list);
				tuple->_0 = a->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}

			{
				tuple_list::handle* list = wstate->clear;
				clear_tuple* tuple = tuple_list::append<clear_tuple>(list);
				tuple->_0 = a->_1;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		{
			task_instance* t = push_task(pstate, task_stack);
			stack_args* a = push<stack_args>(pstate.tstack);
			a->_0 = method_args->_0;
			a->_1 = method_args->_1;
			t->args = a;

			for (holding_tuple* tuple = tuple_list::head<holding_tuple>(wstate->holding); tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != a->_0)
				{
					continue;
				}

				tuple_list::handle* list = wstate->holding;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
				tuple_list::detach(list, tuple);

				break;
			}
			for (clear_tuple* tuple = tuple_list::head<clear_tuple>(wstate->clear); tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != a->_1)
				{
					continue;
				}

				tuple_list::handle* list = wstate->clear;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
				tuple_list::detach(list, tuple);

				break;
			}

			{
				tuple_list::handle* list = wstate->on;
				on_tuple* tuple = tuple_list::append<on_tuple>(list);
				tuple->_0 = a->_0;
				tuple->_1 = a->_1;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}

			{
				tuple_list::handle* list = wstate->clear;
				clear_tuple* tuple = tuple_list::append<clear_tuple>(list);
				tuple->_0 = a->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		{
			{
				tuple_list::handle* list = wstate->dont_move;
				dont_move_tuple* tuple = tuple_list::append<dont_move_tuple>(list);
				tuple->_0 = method_args->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		{
			for (stack_on_block_tuple* tuple = tuple_list::head<stack_on_block_tuple>(wstate->stack_on_block); tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != method_args->_0)
				{
					continue;
				}

				if (tuple->_1 != method_args->_1)
				{
					continue;
				}

				tuple_list::handle* list = wstate->stack_on_block;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
				tuple_list::detach(list, tuple);

				break;
			}
		}

		{
			method_instance* t = push_method(pstate, check_branch_0_expand);
			check_args* a = push<check_args>(pstate.mstack);
			a->_0 = method_args->_0;
			t->args = a;
		}

		PLNNR_COROUTINE_YIELD(*method);

		{
			method_instance* t = push_method(pstate, check2_branch_0_expand);
			check2_args* a = push<check2_args>(pstate.mstack);
			a->_0 = precondition->_1;
			t->args = a;
		}

		PLNNR_COROUTINE_YIELD(*method);

		{
			method_instance* t = push_method(pstate, check3_branch_0_expand);
			check3_args* a = push<check3_args>(pstate.mstack);
			a->_0 = precondition->_1;
			t->args = a;
		}

		method->expanded = true;
		PLNNR_COROUTINE_YIELD(*method);
	}

	return next_branch(pstate, move_block1_branch_1_expand, world);
	PLNNR_COROUTINE_END();
}

bool move_block1_branch_1_expand(planner_state& pstate, void* world)
{
	method_instance* method = pstate.top_method;
	p31_state* precondition = static_cast<p31_state*>(method->precondition);
	worldstate* wstate = static_cast<worldstate*>(world);
	move_block1_args* method_args = static_cast<move_block1_args*>(method->args);

	PLNNR_COROUTINE_BEGIN(*method);

	precondition = push<p31_state>(pstate.mstack);
	precondition->stage = 0;

	method->precondition = precondition;
	method->mrewind = pstate.mstack->top();
	method->trewind = pstate.tstack->top();
	method->jrewind = pstate.journal->top();

	while (next(*precondition, *wstate))
	{
		{
			task_instance* t = push_task(pstate, task_pickup);
			pickup_args* a = push<pickup_args>(pstate.tstack);
			a->_0 = method_args->_0;
			t->args = a;

			for (clear_tuple* tuple = tuple_list::head<clear_tuple>(wstate->clear); tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != a->_0)
				{
					continue;
				}

				tuple_list::handle* list = wstate->clear;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
				tuple_list::detach(list, tuple);

				break;
			}
			for (on_table_tuple* tuple = tuple_list::head<on_table_tuple>(wstate->on_table); tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != a->_0)
				{
					continue;
				}

				tuple_list::handle* list = wstate->on_table;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
				tuple_list::detach(list, tuple);

				break;
			}

			{
				tuple_list::handle* list = wstate->holding;
				holding_tuple* tuple = tuple_list::append<holding_tuple>(list);
				tuple->_0 = a->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		{
			task_instance* t = push_task(pstate, task_stack);
			stack_args* a = push<stack_args>(pstate.tstack);
			a->_0 = method_args->_0;
			a->_1 = method_args->_1;
			t->args = a;

			for (holding_tuple* tuple = tuple_list::head<holding_tuple>(wstate->holding); tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != a->_0)
				{
					continue;
				}

				tuple_list::handle* list = wstate->holding;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
				tuple_list::detach(list, tuple);

				break;
			}
			for (clear_tuple* tuple = tuple_list::head<clear_tuple>(wstate->clear); tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != a->_1)
				{
					continue;
				}

				tuple_list::handle* list = wstate->clear;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
				tuple_list::detach(list, tuple);

				break;
			}

			{
				tuple_list::handle* list = wstate->on;
				on_tuple* tuple = tuple_list::append<on_tuple>(list);
				tuple->_0 = a->_0;
				tuple->_1 = a->_1;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}

			{
				tuple_list::handle* list = wstate->clear;
				clear_tuple* tuple = tuple_list::append<clear_tuple>(list);
				tuple->_0 = a->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		{
			{
				tuple_list::handle* list = wstate->dont_move;
				dont_move_tuple* tuple = tuple_list::append<dont_move_tuple>(list);
				tuple->_0 = method_args->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		{
			for (stack_on_block_tuple* tuple = tuple_list::head<stack_on_block_tuple>(wstate->stack_on_block); tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != method_args->_0)
				{
					continue;
				}

				if (tuple->_1 != method_args->_1)
				{
					continue;
				}

				tuple_list::handle* list = wstate->stack_on_block;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
				tuple_list::detach(list, tuple);

				break;
			}
		}

		{
			method_instance* t = push_method(pstate, check_branch_0_expand);
			check_args* a = push<check_args>(pstate.mstack);
			a->_0 = method_args->_0;
			t->args = a;
		}

		method->expanded = true;
		PLNNR_COROUTINE_YIELD(*method);
	}

	PLNNR_COROUTINE_END();
}

