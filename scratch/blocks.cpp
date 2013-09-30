#include <derplanner/runtime/runtime.h>

using namespace plnnr;

struct block_tuple
{
	int _0;
	block_tuple* next;
	block_tuple* prev;
};

struct on_table_tuple
{
	int _0;
	on_table_tuple* next;
	on_table_tuple* prev;
};

struct on_tuple
{
	int _0;
	int _1;
	on_tuple* next;
	on_tuple* prev;
};

struct clear_tuple
{
	int _0;
	clear_tuple* next;
	clear_tuple* prev;
};

struct goal_on_table_tuple
{
	int _0;
	goal_on_table_tuple* next;
	goal_on_table_tuple* prev;
};

struct goal_on_tuple
{
	int _0;
	int _1;
	goal_on_tuple* next;
	goal_on_tuple* prev;
};

struct goal_clear_tuple
{
	int _0;
	goal_clear_tuple* next;
	goal_clear_tuple* prev;
};

struct holding_tuple
{
	int _0;
	holding_tuple* next;
	holding_tuple* prev;
};

struct dont_move_tuple
{
	int _0;
	dont_move_tuple* next;
	dont_move_tuple* prev;
};

struct need_to_move_tuple
{
	int _0;
	need_to_move_tuple* next;
	need_to_move_tuple* prev;
};

struct put_on_table_tuple
{
	int _0;
	put_on_table_tuple* next;
	put_on_table_tuple* prev;
};

struct stack_on_block_tuple
{
	int _0;
	int _1;
	stack_on_block_tuple* next;
	stack_on_block_tuple* prev;
};

struct worldstate
{
	block_tuple* block;
	on_table_tuple* on_table;
	on_tuple* on;
	clear_tuple* clear;
	goal_on_table_tuple* goal_on_table;
	goal_on_tuple* goal_on;
	goal_clear_tuple* goal_clear;
	holding_tuple* holding;
	dont_move_tuple* dont_move;
	need_to_move_tuple* need_to_move;
	put_on_table_tuple* put_on_table;
	stack_on_block_tuple* stack_on_block;
};

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

struct p1_state
{
	int _0;
	block_tuple* block_0;
	int stage;
};

bool next(p1_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.block_0 = world.block; state.block_0 != 0; state.block_0 = state.block_0->next)
	{
		state._0 = state.block_0->_0;

		PLNNR_COROUTINE_YIELD(state);
	}

	PLNNR_COROUTINE_END();
}

struct p2_state
{
	int _0;
	dont_move_tuple* dont_move_0;
	need_to_move_tuple* need_to_move_1;
	int stage;
};

bool next(p2_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	if (!world.dont_move)
	{
		if (!world.need_to_move)
		{
			PLNNR_COROUTINE_YIELD(state);
		}

		for (state.need_to_move_1 = world.need_to_move; state.need_to_move_1 != 0; state.need_to_move_1 = state.need_to_move_1->next)
		{
			if (state.need_to_move_1->_0 == state._0)
			{
				continue;
			}

			PLNNR_COROUTINE_YIELD(state);
		}
	}

	for (state.dont_move_0 = world.dont_move; state.dont_move_0 != 0; state.dont_move_0 = state.dont_move_0->next)
	{
		if (state.dont_move_0->_0 == state._0)
		{
			continue;
		}

		if (!world.need_to_move)
		{
			PLNNR_COROUTINE_YIELD(state);
		}

		for (state.need_to_move_1 = world.need_to_move; state.need_to_move_1 != 0; state.need_to_move_1 = state.need_to_move_1->next)
		{
			if (state.need_to_move_1->_0 == state._0)
			{
				continue;
			}

			PLNNR_COROUTINE_YIELD(state);
		}
	}

	PLNNR_COROUTINE_END();
}

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

struct p4_state
{
	int _0;
	int _1;
	on_tuple* on_0;
	int stage;
};

bool next(p4_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.on_0 = world.on; state.on_0 != 0; state.on_0 = state.on_0->next)
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

struct p6_state
{
	int _0;
	int _1;
	int _2;
	on_tuple* on_0;
	goal_on_tuple* goal_on_1;
	int stage;
};

bool next(p6_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.on_0 = world.on; state.on_0 != 0; state.on_0 = state.on_0->next)
	{
		if (state.on_0->_0 != state._0)
		{
			continue;
		}

		state._1 = state.on_0->_1;

		for (state.goal_on_1 = world.goal_on; state.goal_on_1 != 0; state.goal_on_1 = state.goal_on_1->next)
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

struct p7_state
{
	int _0;
	int _1;
	on_table_tuple* on_table_0;
	goal_on_tuple* goal_on_1;
	int stage;
};

bool next(p7_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.on_table_0 = world.on_table; state.on_table_0 != 0; state.on_table_0 = state.on_table_0->next)
	{
		if (state.on_table_0->_0 != state._0)
		{
			continue;
		}

		for (state.goal_on_1 = world.goal_on; state.goal_on_1 != 0; state.goal_on_1 = state.goal_on_1->next)
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

struct p8_state
{
	int _0;
	int _1;
	on_tuple* on_0;
	goal_on_table_tuple* goal_on_table_1;
	int stage;
};

bool next(p8_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.on_0 = world.on; state.on_0 != 0; state.on_0 = state.on_0->next)
	{
		if (state.on_0->_0 != state._0)
		{
			continue;
		}

		state._1 = state.on_0->_1;

		for (state.goal_on_table_1 = world.goal_on_table; state.goal_on_table_1 != 0; state.goal_on_table_1 = state.goal_on_table_1->next)
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

struct p9_state
{
	int _0;
	int _1;
	on_tuple* on_0;
	goal_clear_tuple* goal_clear_1;
	int stage;
};

bool next(p9_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.on_0 = world.on; state.on_0 != 0; state.on_0 = state.on_0->next)
	{
		if (state.on_0->_0 != state._0)
		{
			continue;
		}

		state._1 = state.on_0->_1;

		for (state.goal_clear_1 = world.goal_clear; state.goal_clear_1 != 0; state.goal_clear_1 = state.goal_clear_1->next)
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

struct p10_state
{
	int _0;
	int _1;
	int _2;
	on_tuple* on_0;
	goal_on_tuple* goal_on_1;
	int stage;
};

bool next(p10_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.on_0 = world.on; state.on_0 != 0; state.on_0 = state.on_0->next)
	{
		if (state.on_0->_0 != state._0)
		{
			continue;
		}

		state._1 = state.on_0->_1;

		for (state.goal_on_1 = world.goal_on; state.goal_on_1 != 0; state.goal_on_1 = state.goal_on_1->next)
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

struct p11_state
{
	int _0;
	int _1;
	on_tuple* on_0;
	need_to_move_tuple* need_to_move_1;
	int stage;
};

bool next(p11_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.on_0 = world.on; state.on_0 != 0; state.on_0 = state.on_0->next)
	{
		if (state.on_0->_0 != state._0)
		{
			continue;
		}

		state._1 = state.on_0->_1;

		for (state.need_to_move_1 = world.need_to_move; state.need_to_move_1 != 0; state.need_to_move_1 = state.need_to_move_1->next)
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

struct p13_state
{
	int _0;
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

	for (state.block_0 = world.block; state.block_0 != 0; state.block_0 = state.block_0->next)
	{
		state._0 = state.block_0->_0;

		if (!world.dont_move)
		{
			if (!world.goal_on_table)
			{
				for (state.goal_on_3 = world.goal_on; state.goal_on_3 != 0; state.goal_on_3 = state.goal_on_3->next)
				{
					if (state.goal_on_3->_0 == state._0)
					{
						continue;
					}

					state._1 = state.goal_on_3->_1;

					PLNNR_COROUTINE_YIELD(state);
				}
			}

			for (state.goal_on_table_2 = world.goal_on_table; state.goal_on_table_2 != 0; state.goal_on_table_2 = state.goal_on_table_2->next)
			{
				if (state.goal_on_table_2->_0 == state._0)
				{
					continue;
				}

				for (state.goal_on_3 = world.goal_on; state.goal_on_3 != 0; state.goal_on_3 = state.goal_on_3->next)
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

		for (state.dont_move_1 = world.dont_move; state.dont_move_1 != 0; state.dont_move_1 = state.dont_move_1->next)
		{
			if (state.dont_move_1->_0 == state._0)
			{
				continue;
			}

			if (!world.goal_on_table)
			{
				for (state.goal_on_3 = world.goal_on; state.goal_on_3 != 0; state.goal_on_3 = state.goal_on_3->next)
				{
					if (state.goal_on_3->_0 == state._0)
					{
						continue;
					}

					state._1 = state.goal_on_3->_1;

					PLNNR_COROUTINE_YIELD(state);
				}
			}

			for (state.goal_on_table_2 = world.goal_on_table; state.goal_on_table_2 != 0; state.goal_on_table_2 = state.goal_on_table_2->next)
			{
				if (state.goal_on_table_2->_0 == state._0)
				{
					continue;
				}

				for (state.goal_on_3 = world.goal_on; state.goal_on_3 != 0; state.goal_on_3 = state.goal_on_3->next)
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

struct p15_state
{
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

	for (state.clear_0 = world.clear; state.clear_0 != 0; state.clear_0 = state.clear_0->next)
	{
		state._0 = state.clear_0->_0;

		if (!world.dont_move)
		{
			for (state.goal_on_table_2 = world.goal_on_table; state.goal_on_table_2 != 0; state.goal_on_table_2 = state.goal_on_table_2->next)
			{
				if (state.goal_on_table_2->_0 != state._0)
				{
					continue;
				}

				if (!world.put_on_table)
				{
					PLNNR_COROUTINE_YIELD(state);
				}

				for (state.put_on_table_3 = world.put_on_table; state.put_on_table_3 != 0; state.put_on_table_3 = state.put_on_table_3->next)
				{
					if (state.put_on_table_3->_0 == state._0)
					{
						continue;
					}

					PLNNR_COROUTINE_YIELD(state);
				}
			}
		}

		for (state.dont_move_1 = world.dont_move; state.dont_move_1 != 0; state.dont_move_1 = state.dont_move_1->next)
		{
			if (state.dont_move_1->_0 == state._0)
			{
				continue;
			}

			for (state.goal_on_table_2 = world.goal_on_table; state.goal_on_table_2 != 0; state.goal_on_table_2 = state.goal_on_table_2->next)
			{
				if (state.goal_on_table_2->_0 != state._0)
				{
					continue;
				}

				if (!world.put_on_table)
				{
					PLNNR_COROUTINE_YIELD(state);
				}

				for (state.put_on_table_3 = world.put_on_table; state.put_on_table_3 != 0; state.put_on_table_3 = state.put_on_table_3->next)
				{
					if (state.put_on_table_3->_0 == state._0)
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

struct p16_state
{
	int _0;
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

	for (state.clear_0 = world.clear; state.clear_0 != 0; state.clear_0 = state.clear_0->next)
	{
		state._0 = state.clear_0->_0;

		if (!world.dont_move)
		{
			for (state.goal_on_2 = world.goal_on; state.goal_on_2 != 0; state.goal_on_2 = state.goal_on_2->next)
			{
				if (state.goal_on_2->_0 != state._0)
				{
					continue;
				}

				state._1 = state.goal_on_2->_1;

				if (!world.stack_on_block)
				{
					for (state.dont_move_4 = world.dont_move; state.dont_move_4 != 0; state.dont_move_4 = state.dont_move_4->next)
					{
						if (state.dont_move_4->_0 != state._1)
						{
							continue;
						}

						for (state.clear_5 = world.clear; state.clear_5 != 0; state.clear_5 = state.clear_5->next)
						{
							if (state.clear_5->_0 != state._1)
							{
								continue;
							}

							PLNNR_COROUTINE_YIELD(state);
						}
					}
				}

				for (state.stack_on_block_3 = world.stack_on_block; state.stack_on_block_3 != 0; state.stack_on_block_3 = state.stack_on_block_3->next)
				{
					if (state.stack_on_block_3->_0 == state._0)
					{
						continue;
					}

					if (state.stack_on_block_3->_1 == state._1)
					{
						continue;
					}

					for (state.dont_move_4 = world.dont_move; state.dont_move_4 != 0; state.dont_move_4 = state.dont_move_4->next)
					{
						if (state.dont_move_4->_0 != state._1)
						{
							continue;
						}

						for (state.clear_5 = world.clear; state.clear_5 != 0; state.clear_5 = state.clear_5->next)
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

		for (state.dont_move_1 = world.dont_move; state.dont_move_1 != 0; state.dont_move_1 = state.dont_move_1->next)
		{
			if (state.dont_move_1->_0 == state._0)
			{
				continue;
			}

			for (state.goal_on_2 = world.goal_on; state.goal_on_2 != 0; state.goal_on_2 = state.goal_on_2->next)
			{
				if (state.goal_on_2->_0 != state._0)
				{
					continue;
				}

				state._1 = state.goal_on_2->_1;

				if (!world.stack_on_block)
				{
					for (state.dont_move_4 = world.dont_move; state.dont_move_4 != 0; state.dont_move_4 = state.dont_move_4->next)
					{
						if (state.dont_move_4->_0 != state._1)
						{
							continue;
						}

						for (state.clear_5 = world.clear; state.clear_5 != 0; state.clear_5 = state.clear_5->next)
						{
							if (state.clear_5->_0 != state._1)
							{
								continue;
							}

							PLNNR_COROUTINE_YIELD(state);
						}
					}
				}

				for (state.stack_on_block_3 = world.stack_on_block; state.stack_on_block_3 != 0; state.stack_on_block_3 = state.stack_on_block_3->next)
				{
					if (state.stack_on_block_3->_0 == state._0)
					{
						continue;
					}

					if (state.stack_on_block_3->_1 == state._1)
					{
						continue;
					}

					for (state.dont_move_4 = world.dont_move; state.dont_move_4 != 0; state.dont_move_4 = state.dont_move_4->next)
					{
						if (state.dont_move_4->_0 != state._1)
						{
							continue;
						}

						for (state.clear_5 = world.clear; state.clear_5 != 0; state.clear_5 = state.clear_5->next)
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

struct p18_state
{
	int _0;
	int _1;
	stack_on_block_tuple* stack_on_block_0;
	int stage;
};

bool next(p18_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.stack_on_block_0 = world.stack_on_block; state.stack_on_block_0 != 0; state.stack_on_block_0 = state.stack_on_block_0->next)
	{
		state._0 = state.stack_on_block_0->_0;

		state._1 = state.stack_on_block_0->_1;

		PLNNR_COROUTINE_YIELD(state);
	}

	PLNNR_COROUTINE_END();
}

struct p19_state
{
	int _0;
	int _1;
	put_on_table_tuple* put_on_table_0;
	on_tuple* on_1;
	int stage;
};

bool next(p19_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.put_on_table_0 = world.put_on_table; state.put_on_table_0 != 0; state.put_on_table_0 = state.put_on_table_0->next)
	{
		state._0 = state.put_on_table_0->_0;

		for (state.on_1 = world.on; state.on_1 != 0; state.on_1 = state.on_1->next)
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

struct p20_state
{
	int _0;
	int _1;
	clear_tuple* clear_0;
	dont_move_tuple* dont_move_1;
	on_tuple* on_2;
	int stage;
};

bool next(p20_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.clear_0 = world.clear; state.clear_0 != 0; state.clear_0 = state.clear_0->next)
	{
		state._0 = state.clear_0->_0;

		if (!world.dont_move)
		{
			for (state.on_2 = world.on; state.on_2 != 0; state.on_2 = state.on_2->next)
			{
				if (state.on_2->_0 != state._0)
				{
					continue;
				}

				state._1 = state.on_2->_1;

				PLNNR_COROUTINE_YIELD(state);
			}
		}

		for (state.dont_move_1 = world.dont_move; state.dont_move_1 != 0; state.dont_move_1 = state.dont_move_1->next)
		{
			if (state.dont_move_1->_0 == state._0)
			{
				continue;
			}

			for (state.on_2 = world.on; state.on_2 != 0; state.on_2 = state.on_2->next)
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

struct p22_state
{
	int _0;
	int _1;
	goal_on_tuple* goal_on_0;
	clear_tuple* clear_1;
	int stage;
};

bool next(p22_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.goal_on_0 = world.goal_on; state.goal_on_0 != 0; state.goal_on_0 = state.goal_on_0->next)
	{
		if (state.goal_on_0->_1 != state._1)
		{
			continue;
		}

		state._0 = state.goal_on_0->_0;

		for (state.clear_1 = world.clear; state.clear_1 != 0; state.clear_1 = state.clear_1->next)
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

struct p24_state
{
	int _0;
	int _1;
	dont_move_tuple* dont_move_0;
	goal_on_tuple* goal_on_1;
	clear_tuple* clear_2;
	int stage;
};

bool next(p24_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.dont_move_0 = world.dont_move; state.dont_move_0 != 0; state.dont_move_0 = state.dont_move_0->next)
	{
		if (state.dont_move_0->_0 != state._0)
		{
			continue;
		}

		for (state.goal_on_1 = world.goal_on; state.goal_on_1 != 0; state.goal_on_1 = state.goal_on_1->next)
		{
			if (state.goal_on_1->_1 != state._0)
			{
				continue;
			}

			state._1 = state.goal_on_1->_0;

			for (state.clear_2 = world.clear; state.clear_2 != 0; state.clear_2 = state.clear_2->next)
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

struct p26_state
{
	int _0;
	dont_move_tuple* dont_move_0;
	int stage;
};

bool next(p26_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.dont_move_0 = world.dont_move; state.dont_move_0 != 0; state.dont_move_0 = state.dont_move_0->next)
	{
		if (state.dont_move_0->_0 != state._0)
		{
			continue;
		}

		PLNNR_COROUTINE_YIELD(state);
	}

	PLNNR_COROUTINE_END();
}

struct p27_state
{
	int _0;
	int _1;
	goal_on_tuple* goal_on_0;
	clear_tuple* clear_1;
	dont_move_tuple* dont_move_2;
	int stage;
};

bool next(p27_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.goal_on_0 = world.goal_on; state.goal_on_0 != 0; state.goal_on_0 = state.goal_on_0->next)
	{
		if (state.goal_on_0->_0 != state._0)
		{
			continue;
		}

		state._1 = state.goal_on_0->_1;

		for (state.clear_1 = world.clear; state.clear_1 != 0; state.clear_1 = state.clear_1->next)
		{
			if (state.clear_1->_0 != state._1)
			{
				continue;
			}

			for (state.dont_move_2 = world.dont_move; state.dont_move_2 != 0; state.dont_move_2 = state.dont_move_2->next)
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

struct p28_state
{
	int _0;
	goal_on_table_tuple* goal_on_table_0;
	int stage;
};

bool next(p28_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.goal_on_table_0 = world.goal_on_table; state.goal_on_table_0 != 0; state.goal_on_table_0 = state.goal_on_table_0->next)
	{
		if (state.goal_on_table_0->_0 != state._0)
		{
			continue;
		}

		PLNNR_COROUTINE_YIELD(state);
	}

	PLNNR_COROUTINE_END();
}

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

struct p30_state
{
	int _0;
	int _1;
	on_tuple* on_0;
	int stage;
};

bool next(p30_state& state, worldstate& world)
{
	PLNNR_COROUTINE_BEGIN(state);

	for (state.on_0 = world.on; state.on_0 != 0; state.on_0 = state.on_0->next)
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

enum task_type
{
	task_none=0,
	task_add_stack_on_block,
	task_putdown,
	task_mark_dont_move,
	task_remove_put_on_table,
	task_remove_stack_on_block,
	task_stack,
	task_mark_need_to_move,
	task_add_goal_on_table,
	task_mark_no_move,
	task_unstack,
	task_pickup,
	task_add_put_on_table,
};

struct add_stack_on_block_args
{
	int _0;
	int _1;
};

struct putdown_args
{
	int _0;
};

struct mark_dont_move_args
{
	int _0;
};

struct remove_put_on_table_args
{
	int _0;
};

struct remove_stack_on_block_args
{
	int _0;
	int _1;
};

struct stack_args
{
	int _0;
	int _1;
};

struct mark_need_to_move_args
{
	int _0;
};

struct add_goal_on_table_args
{
	int _0;
};

struct mark_no_move_args
{
	int _0;
};

struct unstack_args
{
	int _0;
	int _1;
};

struct pickup_args
{
	int _0;
};

struct add_put_on_table_args
{
	int _0;
};

struct check2_args
{
	int _0;
};

struct move_block1_args
{
	int _0;
	int _1;
};

struct check3_args
{
	int _0;
};

struct mark_block_args
{
	int _0;
};

struct mark_block_recursive_args
{
	int _0;
};

struct mark_block_term_args
{
	int _0;
};

struct check_args
{
	int _0;
};

bool solve_branch_0_expand(planner_state& pstate, void* world);
bool mark_all_blocks_branch_0_expand(planner_state& pstate, void* world);
bool mark_block_branch_0_expand(planner_state& pstate, void* world);
bool mark_block_branch_1_expand(planner_state& pstate, void* world);
bool mark_block_recursive_branch_0_expand(planner_state& pstate, void* world);
bool mark_block_recursive_branch_1_expand(planner_state& pstate, void* world);
bool mark_block_term_branch_0_expand(planner_state& pstate, void* world);
bool mark_block_term_branch_1_expand(planner_state& pstate, void* world);
bool mark_block_term_branch_2_expand(planner_state& pstate, void* world);
bool mark_block_term_branch_3_expand(planner_state& pstate, void* world);
bool mark_block_term_branch_4_expand(planner_state& pstate, void* world);
bool mark_block_term_branch_5_expand(planner_state& pstate, void* world);
bool mark_block_term_branch_6_expand(planner_state& pstate, void* world);
bool add_new_goals_branch_0_expand(planner_state& pstate, void* world);
bool add_new_goals_branch_1_expand(planner_state& pstate, void* world);
bool find_movable_branch_0_expand(planner_state& pstate, void* world);
bool find_movable_branch_1_expand(planner_state& pstate, void* world);
bool find_movable_branch_2_expand(planner_state& pstate, void* world);
bool move_block_branch_0_expand(planner_state& pstate, void* world);
bool move_block_branch_1_expand(planner_state& pstate, void* world);
bool move_block_branch_2_expand(planner_state& pstate, void* world);
bool move_block_branch_3_expand(planner_state& pstate, void* world);
bool check_branch_0_expand(planner_state& pstate, void* world);
bool check_branch_1_expand(planner_state& pstate, void* world);
bool check2_branch_0_expand(planner_state& pstate, void* world);
bool check2_branch_1_expand(planner_state& pstate, void* world);
bool check3_branch_0_expand(planner_state& pstate, void* world);
bool check3_branch_1_expand(planner_state& pstate, void* world);
bool check3_branch_2_expand(planner_state& pstate, void* world);
bool check3_branch_3_expand(planner_state& pstate, void* world);
bool move_block1_branch_0_expand(planner_state& pstate, void* world);
bool move_block1_branch_1_expand(planner_state& pstate, void* world);

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
			task_instance* t = push_task(pstate, task_mark_need_to_move);
			mark_need_to_move_args* a = push<mark_need_to_move_args>(pstate.tstack);
			a->_0 = method_args->_0;
			t->args = a;

			{
				tuple_list::handle* list = tuple_list::head_to_handle<need_to_move_tuple>(wstate->need_to_move);
				need_to_move_tuple* tuple = tuple_list::append<need_to_move_tuple>(list);
				tuple->_0 = a->_0;
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
			task_instance* t = push_task(pstate, task_mark_need_to_move);
			mark_need_to_move_args* a = push<mark_need_to_move_args>(pstate.tstack);
			a->_0 = method_args->_0;
			t->args = a;

			{
				tuple_list::handle* list = tuple_list::head_to_handle<need_to_move_tuple>(wstate->need_to_move);
				need_to_move_tuple* tuple = tuple_list::append<need_to_move_tuple>(list);
				tuple->_0 = a->_0;
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
			task_instance* t = push_task(pstate, task_mark_need_to_move);
			mark_need_to_move_args* a = push<mark_need_to_move_args>(pstate.tstack);
			a->_0 = method_args->_0;
			t->args = a;

			{
				tuple_list::handle* list = tuple_list::head_to_handle<need_to_move_tuple>(wstate->need_to_move);
				need_to_move_tuple* tuple = tuple_list::append<need_to_move_tuple>(list);
				tuple->_0 = a->_0;
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
			task_instance* t = push_task(pstate, task_mark_need_to_move);
			mark_need_to_move_args* a = push<mark_need_to_move_args>(pstate.tstack);
			a->_0 = method_args->_0;
			t->args = a;

			{
				tuple_list::handle* list = tuple_list::head_to_handle<need_to_move_tuple>(wstate->need_to_move);
				need_to_move_tuple* tuple = tuple_list::append<need_to_move_tuple>(list);
				tuple->_0 = a->_0;
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
			task_instance* t = push_task(pstate, task_mark_need_to_move);
			mark_need_to_move_args* a = push<mark_need_to_move_args>(pstate.tstack);
			a->_0 = method_args->_0;
			t->args = a;

			{
				tuple_list::handle* list = tuple_list::head_to_handle<need_to_move_tuple>(wstate->need_to_move);
				need_to_move_tuple* tuple = tuple_list::append<need_to_move_tuple>(list);
				tuple->_0 = a->_0;
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
			task_instance* t = push_task(pstate, task_mark_need_to_move);
			mark_need_to_move_args* a = push<mark_need_to_move_args>(pstate.tstack);
			a->_0 = method_args->_0;
			t->args = a;

			{
				tuple_list::handle* list = tuple_list::head_to_handle<need_to_move_tuple>(wstate->need_to_move);
				need_to_move_tuple* tuple = tuple_list::append<need_to_move_tuple>(list);
				tuple->_0 = a->_0;
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
			task_instance* t = push_task(pstate, task_mark_no_move);
			mark_no_move_args* a = push<mark_no_move_args>(pstate.tstack);
			a->_0 = method_args->_0;
			t->args = a;

			{
				tuple_list::handle* list = tuple_list::head_to_handle<dont_move_tuple>(wstate->dont_move);
				dont_move_tuple* tuple = tuple_list::append<dont_move_tuple>(list);
				tuple->_0 = a->_0;
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
			task_instance* t = push_task(pstate, task_add_goal_on_table);
			add_goal_on_table_args* a = push<add_goal_on_table_args>(pstate.tstack);
			a->_0 = precondition->_0;
			t->args = a;

			{
				tuple_list::handle* list = tuple_list::head_to_handle<goal_on_table_tuple>(wstate->goal_on_table);
				goal_on_table_tuple* tuple = tuple_list::append<goal_on_table_tuple>(list);
				tuple->_0 = a->_0;
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
			task_instance* t = push_task(pstate, task_add_put_on_table);
			add_put_on_table_args* a = push<add_put_on_table_args>(pstate.tstack);
			a->_0 = precondition->_0;
			t->args = a;

			{
				tuple_list::handle* list = tuple_list::head_to_handle<put_on_table_tuple>(wstate->put_on_table);
				put_on_table_tuple* tuple = tuple_list::append<put_on_table_tuple>(list);
				tuple->_0 = a->_0;
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
			task_instance* t = push_task(pstate, task_add_stack_on_block);
			add_stack_on_block_args* a = push<add_stack_on_block_args>(pstate.tstack);
			a->_0 = precondition->_0;
			a->_1 = precondition->_1;
			t->args = a;

			{
				tuple_list::handle* list = tuple_list::head_to_handle<stack_on_block_tuple>(wstate->stack_on_block);
				stack_on_block_tuple* tuple = tuple_list::append<stack_on_block_tuple>(list);
				tuple->_0 = a->_0;
				tuple->_1 = a->_1;
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

			for (clear_tuple* tuple = wstate->clear; tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != a->_0)
				{
					continue;
				}

				tuple_list::handle* list = tuple_list::head_to_handle<clear_tuple>(wstate->clear);
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
				tuple_list::detach(list, tuple);

				break;
			}

			for (on_tuple* tuple = wstate->on; tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != a->_0)
				{
					continue;
				}

				if (tuple->_1 != a->_1)
				{
					continue;
				}

				tuple_list::handle* list = tuple_list::head_to_handle<on_tuple>(wstate->on);
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
				tuple_list::detach(list, tuple);

				break;
			}

			{
				tuple_list::handle* list = tuple_list::head_to_handle<holding_tuple>(wstate->holding);
				holding_tuple* tuple = tuple_list::append<holding_tuple>(list);
				tuple->_0 = a->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}

			{
				tuple_list::handle* list = tuple_list::head_to_handle<clear_tuple>(wstate->clear);
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

			for (holding_tuple* tuple = wstate->holding; tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != a->_0)
				{
					continue;
				}

				tuple_list::handle* list = tuple_list::head_to_handle<holding_tuple>(wstate->holding);
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
				tuple_list::detach(list, tuple);

				break;
			}

			{
				tuple_list::handle* list = tuple_list::head_to_handle<on_table_tuple>(wstate->on_table);
				on_table_tuple* tuple = tuple_list::append<on_table_tuple>(list);
				tuple->_0 = a->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}

			{
				tuple_list::handle* list = tuple_list::head_to_handle<clear_tuple>(wstate->clear);
				clear_tuple* tuple = tuple_list::append<clear_tuple>(list);
				tuple->_0 = a->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		{
			task_instance* t = push_task(pstate, task_mark_dont_move);
			mark_dont_move_args* a = push<mark_dont_move_args>(pstate.tstack);
			a->_0 = precondition->_0;
			t->args = a;
		}

		{
			task_instance* t = push_task(pstate, task_remove_put_on_table);
			remove_put_on_table_args* a = push<remove_put_on_table_args>(pstate.tstack);
			a->_0 = precondition->_0;
			t->args = a;

			for (put_on_table_tuple* tuple = wstate->put_on_table; tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != a->_0)
				{
					continue;
				}

				tuple_list::handle* list = tuple_list::head_to_handle<put_on_table_tuple>(wstate->put_on_table);
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

			for (clear_tuple* tuple = wstate->clear; tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != a->_0)
				{
					continue;
				}

				tuple_list::handle* list = tuple_list::head_to_handle<clear_tuple>(wstate->clear);
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
				tuple_list::detach(list, tuple);

				break;
			}

			for (on_tuple* tuple = wstate->on; tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != a->_0)
				{
					continue;
				}

				if (tuple->_1 != a->_1)
				{
					continue;
				}

				tuple_list::handle* list = tuple_list::head_to_handle<on_tuple>(wstate->on);
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
				tuple_list::detach(list, tuple);

				break;
			}

			{
				tuple_list::handle* list = tuple_list::head_to_handle<holding_tuple>(wstate->holding);
				holding_tuple* tuple = tuple_list::append<holding_tuple>(list);
				tuple->_0 = a->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}

			{
				tuple_list::handle* list = tuple_list::head_to_handle<clear_tuple>(wstate->clear);
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

			for (holding_tuple* tuple = wstate->holding; tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != a->_0)
				{
					continue;
				}

				tuple_list::handle* list = tuple_list::head_to_handle<holding_tuple>(wstate->holding);
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
				tuple_list::detach(list, tuple);

				break;
			}

			{
				tuple_list::handle* list = tuple_list::head_to_handle<on_table_tuple>(wstate->on_table);
				on_table_tuple* tuple = tuple_list::append<on_table_tuple>(list);
				tuple->_0 = a->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}

			{
				tuple_list::handle* list = tuple_list::head_to_handle<clear_tuple>(wstate->clear);
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
			task_instance* t = push_task(pstate, task_add_stack_on_block);
			add_stack_on_block_args* a = push<add_stack_on_block_args>(pstate.tstack);
			a->_0 = precondition->_0;
			a->_1 = method_args->_0;
			t->args = a;

			{
				tuple_list::handle* list = tuple_list::head_to_handle<stack_on_block_tuple>(wstate->stack_on_block);
				stack_on_block_tuple* tuple = tuple_list::append<stack_on_block_tuple>(list);
				tuple->_0 = a->_0;
				tuple->_1 = a->_1;
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
			task_instance* t = push_task(pstate, task_add_stack_on_block);
			add_stack_on_block_args* a = push<add_stack_on_block_args>(pstate.tstack);
			a->_0 = precondition->_1;
			a->_1 = method_args->_0;
			t->args = a;

			{
				tuple_list::handle* list = tuple_list::head_to_handle<stack_on_block_tuple>(wstate->stack_on_block);
				stack_on_block_tuple* tuple = tuple_list::append<stack_on_block_tuple>(list);
				tuple->_0 = a->_0;
				tuple->_1 = a->_1;
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
			task_instance* t = push_task(pstate, task_add_stack_on_block);
			add_stack_on_block_args* a = push<add_stack_on_block_args>(pstate.tstack);
			a->_0 = method_args->_0;
			a->_1 = precondition->_1;
			t->args = a;

			{
				tuple_list::handle* list = tuple_list::head_to_handle<stack_on_block_tuple>(wstate->stack_on_block);
				stack_on_block_tuple* tuple = tuple_list::append<stack_on_block_tuple>(list);
				tuple->_0 = a->_0;
				tuple->_1 = a->_1;
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
			task_instance* t = push_task(pstate, task_add_put_on_table);
			add_put_on_table_args* a = push<add_put_on_table_args>(pstate.tstack);
			a->_0 = method_args->_0;
			t->args = a;

			{
				tuple_list::handle* list = tuple_list::head_to_handle<put_on_table_tuple>(wstate->put_on_table);
				put_on_table_tuple* tuple = tuple_list::append<put_on_table_tuple>(list);
				tuple->_0 = a->_0;
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

			for (clear_tuple* tuple = wstate->clear; tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != a->_0)
				{
					continue;
				}

				tuple_list::handle* list = tuple_list::head_to_handle<clear_tuple>(wstate->clear);
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
				tuple_list::detach(list, tuple);

				break;
			}

			for (on_tuple* tuple = wstate->on; tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != a->_0)
				{
					continue;
				}

				if (tuple->_1 != a->_1)
				{
					continue;
				}

				tuple_list::handle* list = tuple_list::head_to_handle<on_tuple>(wstate->on);
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
				tuple_list::detach(list, tuple);

				break;
			}

			{
				tuple_list::handle* list = tuple_list::head_to_handle<holding_tuple>(wstate->holding);
				holding_tuple* tuple = tuple_list::append<holding_tuple>(list);
				tuple->_0 = a->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}

			{
				tuple_list::handle* list = tuple_list::head_to_handle<clear_tuple>(wstate->clear);
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

			for (holding_tuple* tuple = wstate->holding; tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != a->_0)
				{
					continue;
				}

				tuple_list::handle* list = tuple_list::head_to_handle<holding_tuple>(wstate->holding);
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
				tuple_list::detach(list, tuple);

				break;
			}

			for (clear_tuple* tuple = wstate->clear; tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != a->_1)
				{
					continue;
				}

				tuple_list::handle* list = tuple_list::head_to_handle<clear_tuple>(wstate->clear);
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
				tuple_list::detach(list, tuple);

				break;
			}

			{
				tuple_list::handle* list = tuple_list::head_to_handle<on_tuple>(wstate->on);
				on_tuple* tuple = tuple_list::append<on_tuple>(list);
				tuple->_0 = a->_0;
				tuple->_1 = a->_1;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}

			{
				tuple_list::handle* list = tuple_list::head_to_handle<clear_tuple>(wstate->clear);
				clear_tuple* tuple = tuple_list::append<clear_tuple>(list);
				tuple->_0 = a->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		{
			task_instance* t = push_task(pstate, task_mark_dont_move);
			mark_dont_move_args* a = push<mark_dont_move_args>(pstate.tstack);
			a->_0 = method_args->_0;
			t->args = a;
		}

		{
			task_instance* t = push_task(pstate, task_remove_stack_on_block);
			remove_stack_on_block_args* a = push<remove_stack_on_block_args>(pstate.tstack);
			a->_0 = method_args->_0;
			a->_1 = method_args->_1;
			t->args = a;

			for (stack_on_block_tuple* tuple = wstate->stack_on_block; tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != a->_0)
				{
					continue;
				}

				if (tuple->_1 != a->_1)
				{
					continue;
				}

				tuple_list::handle* list = tuple_list::head_to_handle<stack_on_block_tuple>(wstate->stack_on_block);
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

			for (clear_tuple* tuple = wstate->clear; tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != a->_0)
				{
					continue;
				}

				tuple_list::handle* list = tuple_list::head_to_handle<clear_tuple>(wstate->clear);
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
				tuple_list::detach(list, tuple);

				break;
			}

			for (on_table_tuple* tuple = wstate->on_table; tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != a->_0)
				{
					continue;
				}

				tuple_list::handle* list = tuple_list::head_to_handle<on_table_tuple>(wstate->on_table);
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
				tuple_list::detach(list, tuple);

				break;
			}

			{
				tuple_list::handle* list = tuple_list::head_to_handle<holding_tuple>(wstate->holding);
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

			for (holding_tuple* tuple = wstate->holding; tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != a->_0)
				{
					continue;
				}

				tuple_list::handle* list = tuple_list::head_to_handle<holding_tuple>(wstate->holding);
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
				tuple_list::detach(list, tuple);

				break;
			}

			for (clear_tuple* tuple = wstate->clear; tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != a->_1)
				{
					continue;
				}

				tuple_list::handle* list = tuple_list::head_to_handle<clear_tuple>(wstate->clear);
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
				tuple_list::detach(list, tuple);

				break;
			}

			{
				tuple_list::handle* list = tuple_list::head_to_handle<on_tuple>(wstate->on);
				on_tuple* tuple = tuple_list::append<on_tuple>(list);
				tuple->_0 = a->_0;
				tuple->_1 = a->_1;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}

			{
				tuple_list::handle* list = tuple_list::head_to_handle<clear_tuple>(wstate->clear);
				clear_tuple* tuple = tuple_list::append<clear_tuple>(list);
				tuple->_0 = a->_0;
				operator_effect* effect = push<operator_effect>(pstate.journal);
				effect->tuple = tuple;
				effect->list = list;
			}
		}

		{
			task_instance* t = push_task(pstate, task_mark_dont_move);
			mark_dont_move_args* a = push<mark_dont_move_args>(pstate.tstack);
			a->_0 = method_args->_0;
			t->args = a;
		}

		{
			task_instance* t = push_task(pstate, task_remove_stack_on_block);
			remove_stack_on_block_args* a = push<remove_stack_on_block_args>(pstate.tstack);
			a->_0 = method_args->_0;
			a->_1 = method_args->_1;
			t->args = a;

			for (stack_on_block_tuple* tuple = wstate->stack_on_block; tuple != 0; tuple = tuple->next)
			{
				if (tuple->_0 != a->_0)
				{
					continue;
				}

				if (tuple->_1 != a->_1)
				{
					continue;
				}

				tuple_list::handle* list = tuple_list::head_to_handle<stack_on_block_tuple>(wstate->stack_on_block);
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

