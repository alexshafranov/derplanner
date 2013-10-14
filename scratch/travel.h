#ifndef travel_H_
#define travel_H_

namespace plnnr
{
	namespace tuple_list
	{
		struct handle;
	}
}

struct worldstate
{
	plnnr::tuple_list::handle* travel;
	plnnr::tuple_list::handle* start;
	plnnr::tuple_list::handle* finish;
	plnnr::tuple_list::handle* short_distance;
	plnnr::tuple_list::handle* long_distance;
	plnnr::tuple_list::handle* airport;
};

struct travel_tuple
{
	travel_tuple* next;
	travel_tuple* prev;
	enum { worldstate_offset = offsetof(worldstate, travel) };
};

struct start_tuple
{
	int _0;
	start_tuple* next;
	start_tuple* prev;
	enum { worldstate_offset = offsetof(worldstate, start) };
};

struct finish_tuple
{
	int _0;
	finish_tuple* next;
	finish_tuple* prev;
	enum { worldstate_offset = offsetof(worldstate, finish) };
};

struct short_distance_tuple
{
	int _0;
	int _1;
	short_distance_tuple* next;
	short_distance_tuple* prev;
	enum { worldstate_offset = offsetof(worldstate, short_distance) };
};

struct long_distance_tuple
{
	int _0;
	int _1;
	long_distance_tuple* next;
	long_distance_tuple* prev;
	enum { worldstate_offset = offsetof(worldstate, long_distance) };
};

struct airport_tuple
{
	int _0;
	int _1;
	airport_tuple* next;
	airport_tuple* prev;
	enum { worldstate_offset = offsetof(worldstate, airport) };
};

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

namespace plnnr
{
	struct planner_state;
}

bool root_branch_0_expand(plnnr::planner_state& pstate, void* world);
bool travel_branch_0_expand(plnnr::planner_state& pstate, void* world);
bool travel_branch_1_expand(plnnr::planner_state& pstate, void* world);
bool travel_by_air_branch_0_expand(plnnr::planner_state& pstate, void* world);

#endif
