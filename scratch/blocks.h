#ifndef blocks_H_
#define blocks_H_

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

namespace plnnr
{
	namespace tuple_list
	{
		struct handle;
	}
}

struct worldstate
{
	plnnr::tuple_list::handle* block;
	plnnr::tuple_list::handle* on_table;
	plnnr::tuple_list::handle* on;
	plnnr::tuple_list::handle* clear;
	plnnr::tuple_list::handle* goal_on_table;
	plnnr::tuple_list::handle* goal_on;
	plnnr::tuple_list::handle* goal_clear;
	plnnr::tuple_list::handle* holding;
	plnnr::tuple_list::handle* dont_move;
	plnnr::tuple_list::handle* need_to_move;
	plnnr::tuple_list::handle* put_on_table;
	plnnr::tuple_list::handle* stack_on_block;
};

enum task_type
{
	task_none=0,
	task_solve,
	task_mark_all_blocks,
	task_mark_block,
	task_mark_block_recursive,
	task_mark_block_term,
	task_find_all_movable,
	task_mark_move_type,
	task_move_block,
	task_check,
	task_check2,
	task_check3,
	task_move_block1,
	task_putdown,
	task_unstack,
	task_pickup,
	task_stack,
};

const char* task_name(task_type type);

struct putdown_args
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

struct stack_args
{
	int _0;
	int _1;
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

struct mark_move_type_args
{
	int _0;
};

struct check_args
{
	int _0;
};

struct check2_args
{
	int _0;
};

struct check3_args
{
	int _0;
};

struct move_block1_args
{
	int _0;
	int _1;
};

namespace plnnr
{
	struct planner_state;
}

bool solve_branch_0_expand(plnnr::planner_state& pstate, void* world);
bool mark_all_blocks_branch_0_expand(plnnr::planner_state& pstate, void* world);
bool mark_block_branch_0_expand(plnnr::planner_state& pstate, void* world);
bool mark_block_branch_1_expand(plnnr::planner_state& pstate, void* world);
bool mark_block_recursive_branch_0_expand(plnnr::planner_state& pstate, void* world);
bool mark_block_recursive_branch_1_expand(plnnr::planner_state& pstate, void* world);
bool mark_block_term_branch_0_expand(plnnr::planner_state& pstate, void* world);
bool mark_block_term_branch_1_expand(plnnr::planner_state& pstate, void* world);
bool mark_block_term_branch_2_expand(plnnr::planner_state& pstate, void* world);
bool mark_block_term_branch_3_expand(plnnr::planner_state& pstate, void* world);
bool mark_block_term_branch_4_expand(plnnr::planner_state& pstate, void* world);
bool mark_block_term_branch_5_expand(plnnr::planner_state& pstate, void* world);
bool mark_block_term_branch_6_expand(plnnr::planner_state& pstate, void* world);
bool find_all_movable_branch_0_expand(plnnr::planner_state& pstate, void* world);
bool mark_move_type_branch_0_expand(plnnr::planner_state& pstate, void* world);
bool mark_move_type_branch_1_expand(plnnr::planner_state& pstate, void* world);
bool mark_move_type_branch_2_expand(plnnr::planner_state& pstate, void* world);
bool move_block_branch_0_expand(plnnr::planner_state& pstate, void* world);
bool move_block_branch_1_expand(plnnr::planner_state& pstate, void* world);
bool move_block_branch_2_expand(plnnr::planner_state& pstate, void* world);
bool move_block_branch_3_expand(plnnr::planner_state& pstate, void* world);
bool check_branch_0_expand(plnnr::planner_state& pstate, void* world);
bool check_branch_1_expand(plnnr::planner_state& pstate, void* world);
bool check2_branch_0_expand(plnnr::planner_state& pstate, void* world);
bool check2_branch_1_expand(plnnr::planner_state& pstate, void* world);
bool check3_branch_0_expand(plnnr::planner_state& pstate, void* world);
bool check3_branch_1_expand(plnnr::planner_state& pstate, void* world);
bool check3_branch_2_expand(plnnr::planner_state& pstate, void* world);
bool check3_branch_3_expand(plnnr::planner_state& pstate, void* world);
bool move_block1_branch_0_expand(plnnr::planner_state& pstate, void* world);
bool move_block1_branch_1_expand(plnnr::planner_state& pstate, void* world);

#endif
