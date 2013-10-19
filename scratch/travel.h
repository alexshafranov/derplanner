#ifndef travel_H_
#define travel_H_

#include <derplanner/runtime/interface.h>

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

namespace travel {

enum atom_type
{
	atom_start,
	atom_finish,
	atom_short_distance,
	atom_long_distance,
	atom_airport,
	atom_count,
};

const char* atom_name(atom_type type);

struct worldstate
{
	plnnr::tuple_list::handle* atoms[atom_count];
};

struct start_tuple
{
	int _0;
	start_tuple* next;
	start_tuple* prev;
	enum { id = atom_start };
};

struct finish_tuple
{
	int _0;
	finish_tuple* next;
	finish_tuple* prev;
	enum { id = atom_finish };
};

struct short_distance_tuple
{
	int _0;
	int _1;
	short_distance_tuple* next;
	short_distance_tuple* prev;
	enum { id = atom_short_distance };
};

struct long_distance_tuple
{
	int _0;
	int _1;
	long_distance_tuple* next;
	long_distance_tuple* prev;
	enum { id = atom_long_distance };
};

struct airport_tuple
{
	int _0;
	int _1;
	airport_tuple* next;
	airport_tuple* prev;
	enum { id = atom_airport };
};

}

namespace travel {

enum task_type
{
	task_none=0,
	task_root,
	task_travel,
	task_travel_by_air,
	task_ride_taxi,
	task_fly,
};

const char* task_name(task_type type);

struct ride_taxi_args
{
	int _0;
	int _1;
};

struct fly_args
{
	int _0;
	int _1;
};

struct travel_args
{
	int _0;
	int _1;
};

struct travel_by_air_args
{
	int _0;
	int _1;
};

bool root_branch_0_expand(plnnr::planner_state& pstate, void* world);
bool travel_branch_0_expand(plnnr::planner_state& pstate, void* world);
bool travel_branch_1_expand(plnnr::planner_state& pstate, void* world);
bool travel_by_air_branch_0_expand(plnnr::planner_state& pstate, void* world);

}

namespace plnnr {

template <typename V>
struct generated_type_reflector<travel::worldstate, V>
{
	void operator()(const travel::worldstate& world, V& visitor)
	{
		reflect_atom_list<travel::start_tuple, V>(travel::atom_start, travel::atom_name(travel::atom_start), world.atoms[travel::atom_start], visitor);
		reflect_atom_list<travel::finish_tuple, V>(travel::atom_finish, travel::atom_name(travel::atom_finish), world.atoms[travel::atom_finish], visitor);
		reflect_atom_list<travel::short_distance_tuple, V>(travel::atom_short_distance, travel::atom_name(travel::atom_short_distance), world.atoms[travel::atom_short_distance], visitor);
		reflect_atom_list<travel::long_distance_tuple, V>(travel::atom_long_distance, travel::atom_name(travel::atom_long_distance), world.atoms[travel::atom_long_distance], visitor);
		reflect_atom_list<travel::airport_tuple, V>(travel::atom_airport, travel::atom_name(travel::atom_airport), world.atoms[travel::atom_airport], visitor);
	}
};


template <typename V>
struct generated_type_reflector<travel::start_tuple, V>
{
    void operator()(const travel::start_tuple& tuple, V& visitor)
    {
		visitor.tuple_element(tuple._0);
    }
};

template <typename V>
struct generated_type_reflector<travel::finish_tuple, V>
{
    void operator()(const travel::finish_tuple& tuple, V& visitor)
    {
		visitor.tuple_element(tuple._0);
    }
};


template <typename V>
struct generated_type_reflector<travel::short_distance_tuple, V>
{
    void operator()(const travel::short_distance_tuple& tuple, V& visitor)
    {
		visitor.tuple_element(tuple._0);
		visitor.tuple_element(tuple._1);
    }
};

template <typename V>
struct generated_type_reflector<travel::long_distance_tuple, V>
{
    void operator()(const travel::long_distance_tuple& tuple, V& visitor)
    {
		visitor.tuple_element(tuple._0);
		visitor.tuple_element(tuple._1);
    }
};


template <typename V>
struct generated_type_reflector<travel::airport_tuple, V>
{
    void operator()(const travel::airport_tuple& tuple, V& visitor)
    {
		visitor.tuple_element(tuple._0);
		visitor.tuple_element(tuple._1);
    }
};

}

#endif
