#ifndef blocks_H_
#define blocks_H_

namespace plnnr
{
	namespace tuple_list
	{
		struct handle;
	}
}

namespace plnnr
{
	struct planner_state;
}

namespace blocks {

enum atom_type
{
	atom_block,
	atom_on_table,
	atom_on,
	atom_clear,
	atom_goal_on_table,
	atom_goal_on,
	atom_goal_clear,
	atom_holding,
	atom_dont_move,
	atom_need_to_move,
	atom_put_on_table,
	atom_stack_on_block,
	atom_count,
};

struct worldstate
{
	plnnr::tuple_list::handle* atoms[atom_count];
};

struct block_tuple
{
	int _0;
	block_tuple* next;
	block_tuple* prev;
	enum { id = atom_block };
};

struct on_table_tuple
{
	int _0;
	on_table_tuple* next;
	on_table_tuple* prev;
	enum { id = atom_on_table };
};

struct on_tuple
{
	int _0;
	int _1;
	on_tuple* next;
	on_tuple* prev;
	enum { id = atom_on };
};

struct clear_tuple
{
	int _0;
	clear_tuple* next;
	clear_tuple* prev;
	enum { id = atom_clear };
};

struct goal_on_table_tuple
{
	int _0;
	goal_on_table_tuple* next;
	goal_on_table_tuple* prev;
	enum { id = atom_goal_on_table };
};

struct goal_on_tuple
{
	int _0;
	int _1;
	goal_on_tuple* next;
	goal_on_tuple* prev;
	enum { id = atom_goal_on };
};

struct goal_clear_tuple
{
	int _0;
	goal_clear_tuple* next;
	goal_clear_tuple* prev;
	enum { id = atom_goal_clear };
};

struct holding_tuple
{
	int _0;
	holding_tuple* next;
	holding_tuple* prev;
	enum { id = atom_holding };
};

struct dont_move_tuple
{
	int _0;
	dont_move_tuple* next;
	dont_move_tuple* prev;
	enum { id = atom_dont_move };
};

struct need_to_move_tuple
{
	int _0;
	need_to_move_tuple* next;
	need_to_move_tuple* prev;
	enum { id = atom_need_to_move };
};

struct put_on_table_tuple
{
	int _0;
	put_on_table_tuple* next;
	put_on_table_tuple* prev;
	enum { id = atom_put_on_table };
};

struct stack_on_block_tuple
{
	int _0;
	int _1;
	stack_on_block_tuple* next;
	stack_on_block_tuple* prev;
	enum { id = atom_stack_on_block };
};

}

namespace blocks {

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

}

#endif
