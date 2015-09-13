#ifndef blocks_H_
#define blocks_H_

#include <derplanner/runtime/interface.h>

namespace plnnr
{
	namespace tuple_list
	{
		struct Handle;
	}
}

namespace plnnr
{
	struct Planner_State;
	struct Method_Instance;
}

namespace blocks {

enum Atom_Type
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

const char* atom_name(Atom_Type type);

struct Worldstate
{
	plnnr::tuple_list::Handle* atoms[atom_count];
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

enum Task_Type
{
	task_putdown,
	task_unstack,
	task_pickup,
	task_stack,
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
	task_count,
};

static const int operator_count = 4;
static const int method_count = 12;

const char* task_name(Task_Type type);

struct putdown_args
{
	int _0;
};

inline bool operator==(const putdown_args& a, const putdown_args& b)
{
	return \
		a._0 == b._0 ;
}

struct unstack_args
{
	int _0;
	int _1;
};

inline bool operator==(const unstack_args& a, const unstack_args& b)
{
	return \
		a._0 == b._0 &&
		a._1 == b._1 ;
}

struct pickup_args
{
	int _0;
};

inline bool operator==(const pickup_args& a, const pickup_args& b)
{
	return \
		a._0 == b._0 ;
}

struct stack_args
{
	int _0;
	int _1;
};

inline bool operator==(const stack_args& a, const stack_args& b)
{
	return \
		a._0 == b._0 &&
		a._1 == b._1 ;
}

struct mark_block_args
{
	int _0;
};

inline bool operator==(const mark_block_args& a, const mark_block_args& b)
{
	return \
		a._0 == b._0 ;
}

struct mark_block_recursive_args
{
	int _0;
};

inline bool operator==(const mark_block_recursive_args& a, const mark_block_recursive_args& b)
{
	return \
		a._0 == b._0 ;
}

struct mark_block_term_args
{
	int _0;
};

inline bool operator==(const mark_block_term_args& a, const mark_block_term_args& b)
{
	return \
		a._0 == b._0 ;
}

struct mark_move_type_args
{
	int _0;
};

inline bool operator==(const mark_move_type_args& a, const mark_move_type_args& b)
{
	return \
		a._0 == b._0 ;
}

struct check_args
{
	int _0;
};

inline bool operator==(const check_args& a, const check_args& b)
{
	return \
		a._0 == b._0 ;
}

struct check2_args
{
	int _0;
};

inline bool operator==(const check2_args& a, const check2_args& b)
{
	return \
		a._0 == b._0 ;
}

struct check3_args
{
	int _0;
};

inline bool operator==(const check3_args& a, const check3_args& b)
{
	return \
		a._0 == b._0 ;
}

struct move_block1_args
{
	int _0;
	int _1;
};

inline bool operator==(const move_block1_args& a, const move_block1_args& b)
{
	return \
		a._0 == b._0 &&
		a._1 == b._1 ;
}

bool solve_branch_0_expand(plnnr::Method_Instance*, plnnr::Planner_State&, void*);
bool mark_all_blocks_branch_0_expand(plnnr::Method_Instance*, plnnr::Planner_State&, void*);
bool mark_block_branch_0_expand(plnnr::Method_Instance*, plnnr::Planner_State&, void*);
bool mark_block_branch_1_expand(plnnr::Method_Instance*, plnnr::Planner_State&, void*);
bool mark_block_recursive_branch_0_expand(plnnr::Method_Instance*, plnnr::Planner_State&, void*);
bool mark_block_recursive_branch_1_expand(plnnr::Method_Instance*, plnnr::Planner_State&, void*);
bool mark_block_term_branch_0_expand(plnnr::Method_Instance*, plnnr::Planner_State&, void*);
bool mark_block_term_branch_1_expand(plnnr::Method_Instance*, plnnr::Planner_State&, void*);
bool mark_block_term_branch_2_expand(plnnr::Method_Instance*, plnnr::Planner_State&, void*);
bool mark_block_term_branch_3_expand(plnnr::Method_Instance*, plnnr::Planner_State&, void*);
bool mark_block_term_branch_4_expand(plnnr::Method_Instance*, plnnr::Planner_State&, void*);
bool mark_block_term_branch_5_expand(plnnr::Method_Instance*, plnnr::Planner_State&, void*);
bool mark_block_term_branch_6_expand(plnnr::Method_Instance*, plnnr::Planner_State&, void*);
bool find_all_movable_branch_0_expand(plnnr::Method_Instance*, plnnr::Planner_State&, void*);
bool mark_move_type_branch_0_expand(plnnr::Method_Instance*, plnnr::Planner_State&, void*);
bool mark_move_type_branch_1_expand(plnnr::Method_Instance*, plnnr::Planner_State&, void*);
bool mark_move_type_branch_2_expand(plnnr::Method_Instance*, plnnr::Planner_State&, void*);
bool move_block_branch_0_expand(plnnr::Method_Instance*, plnnr::Planner_State&, void*);
bool move_block_branch_1_expand(plnnr::Method_Instance*, plnnr::Planner_State&, void*);
bool move_block_branch_2_expand(plnnr::Method_Instance*, plnnr::Planner_State&, void*);
bool move_block_branch_3_expand(plnnr::Method_Instance*, plnnr::Planner_State&, void*);
bool check_branch_0_expand(plnnr::Method_Instance*, plnnr::Planner_State&, void*);
bool check_branch_1_expand(plnnr::Method_Instance*, plnnr::Planner_State&, void*);
bool check2_branch_0_expand(plnnr::Method_Instance*, plnnr::Planner_State&, void*);
bool check2_branch_1_expand(plnnr::Method_Instance*, plnnr::Planner_State&, void*);
bool check3_branch_0_expand(plnnr::Method_Instance*, plnnr::Planner_State&, void*);
bool check3_branch_1_expand(plnnr::Method_Instance*, plnnr::Planner_State&, void*);
bool check3_branch_2_expand(plnnr::Method_Instance*, plnnr::Planner_State&, void*);
bool check3_branch_3_expand(plnnr::Method_Instance*, plnnr::Planner_State&, void*);
bool move_block1_branch_0_expand(plnnr::Method_Instance*, plnnr::Planner_State&, void*);
bool move_block1_branch_1_expand(plnnr::Method_Instance*, plnnr::Planner_State&, void*);

}

namespace plnnr {

template <typename V>
struct Generated_Type_Reflector<blocks::Worldstate, V>
{
	void operator()(const blocks::Worldstate& world, V& visitor)
	{
		PLNNR_GENCODE_VISIT_ATOM_LIST(blocks, atom_block, block_tuple, visitor);
		PLNNR_GENCODE_VISIT_ATOM_LIST(blocks, atom_on_table, on_table_tuple, visitor);
		PLNNR_GENCODE_VISIT_ATOM_LIST(blocks, atom_on, on_tuple, visitor);
		PLNNR_GENCODE_VISIT_ATOM_LIST(blocks, atom_clear, clear_tuple, visitor);
		PLNNR_GENCODE_VISIT_ATOM_LIST(blocks, atom_goal_on_table, goal_on_table_tuple, visitor);
		PLNNR_GENCODE_VISIT_ATOM_LIST(blocks, atom_goal_on, goal_on_tuple, visitor);
		PLNNR_GENCODE_VISIT_ATOM_LIST(blocks, atom_goal_clear, goal_clear_tuple, visitor);
		PLNNR_GENCODE_VISIT_ATOM_LIST(blocks, atom_holding, holding_tuple, visitor);
		PLNNR_GENCODE_VISIT_ATOM_LIST(blocks, atom_dont_move, dont_move_tuple, visitor);
		PLNNR_GENCODE_VISIT_ATOM_LIST(blocks, atom_need_to_move, need_to_move_tuple, visitor);
		PLNNR_GENCODE_VISIT_ATOM_LIST(blocks, atom_put_on_table, put_on_table_tuple, visitor);
		PLNNR_GENCODE_VISIT_ATOM_LIST(blocks, atom_stack_on_block, stack_on_block_tuple, visitor);
	}
};

template <typename V>
struct Generated_Type_Reflector<blocks::block_tuple, V>
{
	void operator()(const blocks::block_tuple& tuple, V& visitor)
	{
		PLNNR_GENCODE_VISIT_TUPLE_BEGIN(visitor, blocks, atom_name, atom_block, 1);
		PLNNR_GENCODE_VISIT_TUPLE_ELEMENT(visitor, tuple, 0);
		PLNNR_GENCODE_VISIT_TUPLE_END(visitor, blocks, atom_name, atom_block, 1);
	}
};

template <typename V>
struct Generated_Type_Reflector<blocks::on_table_tuple, V>
{
	void operator()(const blocks::on_table_tuple& tuple, V& visitor)
	{
		PLNNR_GENCODE_VISIT_TUPLE_BEGIN(visitor, blocks, atom_name, atom_on_table, 1);
		PLNNR_GENCODE_VISIT_TUPLE_ELEMENT(visitor, tuple, 0);
		PLNNR_GENCODE_VISIT_TUPLE_END(visitor, blocks, atom_name, atom_on_table, 1);
	}
};

template <typename V>
struct Generated_Type_Reflector<blocks::on_tuple, V>
{
	void operator()(const blocks::on_tuple& tuple, V& visitor)
	{
		PLNNR_GENCODE_VISIT_TUPLE_BEGIN(visitor, blocks, atom_name, atom_on, 2);
		PLNNR_GENCODE_VISIT_TUPLE_ELEMENT(visitor, tuple, 0);
		PLNNR_GENCODE_VISIT_TUPLE_ELEMENT(visitor, tuple, 1);
		PLNNR_GENCODE_VISIT_TUPLE_END(visitor, blocks, atom_name, atom_on, 2);
	}
};

template <typename V>
struct Generated_Type_Reflector<blocks::clear_tuple, V>
{
	void operator()(const blocks::clear_tuple& tuple, V& visitor)
	{
		PLNNR_GENCODE_VISIT_TUPLE_BEGIN(visitor, blocks, atom_name, atom_clear, 1);
		PLNNR_GENCODE_VISIT_TUPLE_ELEMENT(visitor, tuple, 0);
		PLNNR_GENCODE_VISIT_TUPLE_END(visitor, blocks, atom_name, atom_clear, 1);
	}
};

template <typename V>
struct Generated_Type_Reflector<blocks::goal_on_table_tuple, V>
{
	void operator()(const blocks::goal_on_table_tuple& tuple, V& visitor)
	{
		PLNNR_GENCODE_VISIT_TUPLE_BEGIN(visitor, blocks, atom_name, atom_goal_on_table, 1);
		PLNNR_GENCODE_VISIT_TUPLE_ELEMENT(visitor, tuple, 0);
		PLNNR_GENCODE_VISIT_TUPLE_END(visitor, blocks, atom_name, atom_goal_on_table, 1);
	}
};

template <typename V>
struct Generated_Type_Reflector<blocks::goal_on_tuple, V>
{
	void operator()(const blocks::goal_on_tuple& tuple, V& visitor)
	{
		PLNNR_GENCODE_VISIT_TUPLE_BEGIN(visitor, blocks, atom_name, atom_goal_on, 2);
		PLNNR_GENCODE_VISIT_TUPLE_ELEMENT(visitor, tuple, 0);
		PLNNR_GENCODE_VISIT_TUPLE_ELEMENT(visitor, tuple, 1);
		PLNNR_GENCODE_VISIT_TUPLE_END(visitor, blocks, atom_name, atom_goal_on, 2);
	}
};

template <typename V>
struct Generated_Type_Reflector<blocks::goal_clear_tuple, V>
{
	void operator()(const blocks::goal_clear_tuple& tuple, V& visitor)
	{
		PLNNR_GENCODE_VISIT_TUPLE_BEGIN(visitor, blocks, atom_name, atom_goal_clear, 1);
		PLNNR_GENCODE_VISIT_TUPLE_ELEMENT(visitor, tuple, 0);
		PLNNR_GENCODE_VISIT_TUPLE_END(visitor, blocks, atom_name, atom_goal_clear, 1);
	}
};

template <typename V>
struct Generated_Type_Reflector<blocks::holding_tuple, V>
{
	void operator()(const blocks::holding_tuple& tuple, V& visitor)
	{
		PLNNR_GENCODE_VISIT_TUPLE_BEGIN(visitor, blocks, atom_name, atom_holding, 1);
		PLNNR_GENCODE_VISIT_TUPLE_ELEMENT(visitor, tuple, 0);
		PLNNR_GENCODE_VISIT_TUPLE_END(visitor, blocks, atom_name, atom_holding, 1);
	}
};

template <typename V>
struct Generated_Type_Reflector<blocks::dont_move_tuple, V>
{
	void operator()(const blocks::dont_move_tuple& tuple, V& visitor)
	{
		PLNNR_GENCODE_VISIT_TUPLE_BEGIN(visitor, blocks, atom_name, atom_dont_move, 1);
		PLNNR_GENCODE_VISIT_TUPLE_ELEMENT(visitor, tuple, 0);
		PLNNR_GENCODE_VISIT_TUPLE_END(visitor, blocks, atom_name, atom_dont_move, 1);
	}
};

template <typename V>
struct Generated_Type_Reflector<blocks::need_to_move_tuple, V>
{
	void operator()(const blocks::need_to_move_tuple& tuple, V& visitor)
	{
		PLNNR_GENCODE_VISIT_TUPLE_BEGIN(visitor, blocks, atom_name, atom_need_to_move, 1);
		PLNNR_GENCODE_VISIT_TUPLE_ELEMENT(visitor, tuple, 0);
		PLNNR_GENCODE_VISIT_TUPLE_END(visitor, blocks, atom_name, atom_need_to_move, 1);
	}
};

template <typename V>
struct Generated_Type_Reflector<blocks::put_on_table_tuple, V>
{
	void operator()(const blocks::put_on_table_tuple& tuple, V& visitor)
	{
		PLNNR_GENCODE_VISIT_TUPLE_BEGIN(visitor, blocks, atom_name, atom_put_on_table, 1);
		PLNNR_GENCODE_VISIT_TUPLE_ELEMENT(visitor, tuple, 0);
		PLNNR_GENCODE_VISIT_TUPLE_END(visitor, blocks, atom_name, atom_put_on_table, 1);
	}
};

template <typename V>
struct Generated_Type_Reflector<blocks::stack_on_block_tuple, V>
{
	void operator()(const blocks::stack_on_block_tuple& tuple, V& visitor)
	{
		PLNNR_GENCODE_VISIT_TUPLE_BEGIN(visitor, blocks, atom_name, atom_stack_on_block, 2);
		PLNNR_GENCODE_VISIT_TUPLE_ELEMENT(visitor, tuple, 0);
		PLNNR_GENCODE_VISIT_TUPLE_ELEMENT(visitor, tuple, 1);
		PLNNR_GENCODE_VISIT_TUPLE_END(visitor, blocks, atom_name, atom_stack_on_block, 2);
	}
};

template <typename V>
struct Generated_Type_Reflector<blocks::putdown_args, V>
{
	void operator()(const blocks::putdown_args& tuple, V& visitor)
	{
		PLNNR_GENCODE_VISIT_TUPLE_BEGIN(visitor, blocks, task_name, task_putdown, 1);
		PLNNR_GENCODE_VISIT_TUPLE_ELEMENT(visitor, tuple, 0);
		PLNNR_GENCODE_VISIT_TUPLE_END(visitor, blocks, task_name, task_putdown, 1);
	}
};

template <typename V>
struct Generated_Type_Reflector<blocks::unstack_args, V>
{
	void operator()(const blocks::unstack_args& tuple, V& visitor)
	{
		PLNNR_GENCODE_VISIT_TUPLE_BEGIN(visitor, blocks, task_name, task_unstack, 2);
		PLNNR_GENCODE_VISIT_TUPLE_ELEMENT(visitor, tuple, 0);
		PLNNR_GENCODE_VISIT_TUPLE_ELEMENT(visitor, tuple, 1);
		PLNNR_GENCODE_VISIT_TUPLE_END(visitor, blocks, task_name, task_unstack, 2);
	}
};

template <typename V>
struct Generated_Type_Reflector<blocks::pickup_args, V>
{
	void operator()(const blocks::pickup_args& tuple, V& visitor)
	{
		PLNNR_GENCODE_VISIT_TUPLE_BEGIN(visitor, blocks, task_name, task_pickup, 1);
		PLNNR_GENCODE_VISIT_TUPLE_ELEMENT(visitor, tuple, 0);
		PLNNR_GENCODE_VISIT_TUPLE_END(visitor, blocks, task_name, task_pickup, 1);
	}
};

template <typename V>
struct Generated_Type_Reflector<blocks::stack_args, V>
{
	void operator()(const blocks::stack_args& tuple, V& visitor)
	{
		PLNNR_GENCODE_VISIT_TUPLE_BEGIN(visitor, blocks, task_name, task_stack, 2);
		PLNNR_GENCODE_VISIT_TUPLE_ELEMENT(visitor, tuple, 0);
		PLNNR_GENCODE_VISIT_TUPLE_ELEMENT(visitor, tuple, 1);
		PLNNR_GENCODE_VISIT_TUPLE_END(visitor, blocks, task_name, task_stack, 2);
	}
};

template <typename V>
struct Generated_Type_Reflector<blocks::mark_block_args, V>
{
	void operator()(const blocks::mark_block_args& tuple, V& visitor)
	{
		PLNNR_GENCODE_VISIT_TUPLE_BEGIN(visitor, blocks, task_name, task_mark_block, 1);
		PLNNR_GENCODE_VISIT_TUPLE_ELEMENT(visitor, tuple, 0);
		PLNNR_GENCODE_VISIT_TUPLE_END(visitor, blocks, task_name, task_mark_block, 1);
	}
};

template <typename V>
struct Generated_Type_Reflector<blocks::mark_block_recursive_args, V>
{
	void operator()(const blocks::mark_block_recursive_args& tuple, V& visitor)
	{
		PLNNR_GENCODE_VISIT_TUPLE_BEGIN(visitor, blocks, task_name, task_mark_block_recursive, 1);
		PLNNR_GENCODE_VISIT_TUPLE_ELEMENT(visitor, tuple, 0);
		PLNNR_GENCODE_VISIT_TUPLE_END(visitor, blocks, task_name, task_mark_block_recursive, 1);
	}
};

template <typename V>
struct Generated_Type_Reflector<blocks::mark_block_term_args, V>
{
	void operator()(const blocks::mark_block_term_args& tuple, V& visitor)
	{
		PLNNR_GENCODE_VISIT_TUPLE_BEGIN(visitor, blocks, task_name, task_mark_block_term, 1);
		PLNNR_GENCODE_VISIT_TUPLE_ELEMENT(visitor, tuple, 0);
		PLNNR_GENCODE_VISIT_TUPLE_END(visitor, blocks, task_name, task_mark_block_term, 1);
	}
};

template <typename V>
struct Generated_Type_Reflector<blocks::mark_move_type_args, V>
{
	void operator()(const blocks::mark_move_type_args& tuple, V& visitor)
	{
		PLNNR_GENCODE_VISIT_TUPLE_BEGIN(visitor, blocks, task_name, task_mark_move_type, 1);
		PLNNR_GENCODE_VISIT_TUPLE_ELEMENT(visitor, tuple, 0);
		PLNNR_GENCODE_VISIT_TUPLE_END(visitor, blocks, task_name, task_mark_move_type, 1);
	}
};

template <typename V>
struct Generated_Type_Reflector<blocks::check_args, V>
{
	void operator()(const blocks::check_args& tuple, V& visitor)
	{
		PLNNR_GENCODE_VISIT_TUPLE_BEGIN(visitor, blocks, task_name, task_check, 1);
		PLNNR_GENCODE_VISIT_TUPLE_ELEMENT(visitor, tuple, 0);
		PLNNR_GENCODE_VISIT_TUPLE_END(visitor, blocks, task_name, task_check, 1);
	}
};

template <typename V>
struct Generated_Type_Reflector<blocks::check2_args, V>
{
	void operator()(const blocks::check2_args& tuple, V& visitor)
	{
		PLNNR_GENCODE_VISIT_TUPLE_BEGIN(visitor, blocks, task_name, task_check2, 1);
		PLNNR_GENCODE_VISIT_TUPLE_ELEMENT(visitor, tuple, 0);
		PLNNR_GENCODE_VISIT_TUPLE_END(visitor, blocks, task_name, task_check2, 1);
	}
};

template <typename V>
struct Generated_Type_Reflector<blocks::check3_args, V>
{
	void operator()(const blocks::check3_args& tuple, V& visitor)
	{
		PLNNR_GENCODE_VISIT_TUPLE_BEGIN(visitor, blocks, task_name, task_check3, 1);
		PLNNR_GENCODE_VISIT_TUPLE_ELEMENT(visitor, tuple, 0);
		PLNNR_GENCODE_VISIT_TUPLE_END(visitor, blocks, task_name, task_check3, 1);
	}
};

template <typename V>
struct Generated_Type_Reflector<blocks::move_block1_args, V>
{
	void operator()(const blocks::move_block1_args& tuple, V& visitor)
	{
		PLNNR_GENCODE_VISIT_TUPLE_BEGIN(visitor, blocks, task_name, task_move_block1, 2);
		PLNNR_GENCODE_VISIT_TUPLE_ELEMENT(visitor, tuple, 0);
		PLNNR_GENCODE_VISIT_TUPLE_ELEMENT(visitor, tuple, 1);
		PLNNR_GENCODE_VISIT_TUPLE_END(visitor, blocks, task_name, task_move_block1, 2);
	}
};

template <typename V>
struct Task_Type_Dispatcher<blocks::Task_Type, V>
{
	void operator()(const blocks::Task_Type& task_type, void* args, V& visitor)
	{
		switch (task_type)
		{
			case blocks::task_solve:
				PLNNR_GENCODE_VISIT_TASK_NO_ARGS(visitor, blocks, task_solve);
				break;
			case blocks::task_mark_all_blocks:
				PLNNR_GENCODE_VISIT_TASK_NO_ARGS(visitor, blocks, task_mark_all_blocks);
				break;
			case blocks::task_mark_block:
				PLNNR_GENCODE_VISIT_TASK_WITH_ARGS(visitor, blocks, task_mark_block, mark_block_args);
				break;
			case blocks::task_mark_block_recursive:
				PLNNR_GENCODE_VISIT_TASK_WITH_ARGS(visitor, blocks, task_mark_block_recursive, mark_block_recursive_args);
				break;
			case blocks::task_mark_block_term:
				PLNNR_GENCODE_VISIT_TASK_WITH_ARGS(visitor, blocks, task_mark_block_term, mark_block_term_args);
				break;
			case blocks::task_find_all_movable:
				PLNNR_GENCODE_VISIT_TASK_NO_ARGS(visitor, blocks, task_find_all_movable);
				break;
			case blocks::task_mark_move_type:
				PLNNR_GENCODE_VISIT_TASK_WITH_ARGS(visitor, blocks, task_mark_move_type, mark_move_type_args);
				break;
			case blocks::task_move_block:
				PLNNR_GENCODE_VISIT_TASK_NO_ARGS(visitor, blocks, task_move_block);
				break;
			case blocks::task_check:
				PLNNR_GENCODE_VISIT_TASK_WITH_ARGS(visitor, blocks, task_check, check_args);
				break;
			case blocks::task_check2:
				PLNNR_GENCODE_VISIT_TASK_WITH_ARGS(visitor, blocks, task_check2, check2_args);
				break;
			case blocks::task_check3:
				PLNNR_GENCODE_VISIT_TASK_WITH_ARGS(visitor, blocks, task_check3, check3_args);
				break;
			case blocks::task_move_block1:
				PLNNR_GENCODE_VISIT_TASK_WITH_ARGS(visitor, blocks, task_move_block1, move_block1_args);
				break;
			case blocks::task_putdown:
				PLNNR_GENCODE_VISIT_TASK_WITH_ARGS(visitor, blocks, task_putdown, putdown_args);
				break;
			case blocks::task_unstack:
				PLNNR_GENCODE_VISIT_TASK_WITH_ARGS(visitor, blocks, task_unstack, unstack_args);
				break;
			case blocks::task_pickup:
				PLNNR_GENCODE_VISIT_TASK_WITH_ARGS(visitor, blocks, task_pickup, pickup_args);
				break;
			case blocks::task_stack:
				PLNNR_GENCODE_VISIT_TASK_WITH_ARGS(visitor, blocks, task_stack, stack_args);
				break;
			default:
				plnnr_assert(false);
				break;
		}
	}
};

}

#endif
