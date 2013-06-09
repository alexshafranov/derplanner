/*
(:domain
    (:method (root)
        ((start ?s) (finish ?f))
        ((travel ?s ?f))
    )

    (:method (travel ?x ?y)
        ((short_distance ?x ?y))
        ((!ride_taxi ?x ?y))

        ((long_distance ?x ?y))
        ((travel_by_air ?x ?y))
    )

    (:method (travel_by_air ?x ?y)
        ((airport ?x ?ax) (airport ?y ?ay))
        ((travel ?x ?ax) (!fly ?ax ?ay) (travel ?y ?ay))
    )
)

(start  spb)
(finish msc)

(short_distance spb led)
(short_distance led spb)
(short_distance msc svo)
(short_distance svo msc)

(long_distance spb msc)
(long_distance msc spb)
(long_distance led svo)
(long_distance svo led)
(long_distance spb svo)
(long_distance svo spb)
(long_distance msc led)
(long_distance led msc)

(airport spb led)
(airport msc svo)

(!ride_taxi spb led) (!fly led svo) (!ride_taxi svo msc)

*/

typedef char* stack_ptr;

struct stack
{
    stack(int max_size)
    {
        data = new char[max_size];
        top_ = data;
    }

    ~stack()
    {
        delete [] data;
    }

    void push(unsigned id)
    {

    }

    void push(const void* const data, unsigned size)
    {
    }

    stack_ptr top_ptr()
    {
        return top_;
    }

    void rewind(stack_ptr ptr)
    {
        top_ = ptr;
    }

    char* data;
    char* top_;
};

// world state

struct start_tuple_t
{
    int _0;
    start_tuple_t* next;
};

struct finish_tuple_t
{
    int _0;
    finish_tuple_t* next;
};

struct short_distance_tuple_t
{
    int _0;
    int _1;
    short_distance_tuple_t* next;
};

struct long_distance_tuple_t
{
    int _0;
    int _1;
    long_distance_tuple_t* next;
};

struct airport_tuple_t
{
    int _0;
    int _1;
    airport_tuple_t* next;
};

struct worldstate_t
{
    start_tuple_t*          start;
    finish_tuple_t*         finish;
    short_distance_tuple_t* short_distance;
    long_distance_tuple_t*  long_distance;
    airport_tuple_t*        airport;
};

// domain

// preconditions

// ((start ?s) (finish ?f))
struct p1_state_t
{
    int out_s;
    int out_f;
    start_tuple_t*  state_start;
    finish_tuple_t* state_finish;
};

void init(p1_state_t& state, const worldstate_t& world)
{
    state.state_start  = world.start;
    state.state_finish = world.finish;
}

bool next(p1_state_t& state)
{
    for (; state.state_start != 0; state.state_start = state.state_start->next)
    {
        for (; state.state_finish != 0; state.state_finish = state.state_finish->next)
        {
            state.out_s = state.state_start->_0;
            state.out_f = state.state_finish->_0;
            return true;
        }
    }

    return false;
}

// ((short_distance ?x ?y))
struct p2_state_t
{
    int in_x;
    int in_y;
    short_distance_tuple_t* state_short_distance;
};

void init(p2_state_t& state, const worldstate_t& world)
{
    state.state_short_distance = world.short_distance;
}

bool next(p2_state_t& state)
{
    for (; state.state_short_distance != 0; state.state_short_distance = state.state_short_distance->next)
    {
        if (state.state_short_distance->_0 == state.in_x && state.state_short_distance->_1 == state.in_y)
        {
            return true;
        }
    }

    return false;
}

struct p3_state_t
{
    int in_x;
    int in_y;
    long_distance_tuple_t* state_long_distance;
};

void init(p3_state_t& state, const worldstate_t& world)
{
    state.state_long_distance = world.long_distance;
}

bool next(p3_state_t& state)
{
    for (; state.state_long_distance != 0; state.state_long_distance = state.state_long_distance->next)
    {
        if (state.state_long_distance->_0 == state.in_x && state.state_long_distance->_1 == state.in_y)
        {
            return true;
        }
    }

    return false;
}

// ((airport ?x ?ax) (airport ?y ?ay))
struct p4_state_t
{
    int in_x;
    int in_y;

    int out_ax;
    int out_ay;

    airport_tuple_t* state_airport_1;
    airport_tuple_t* state_airport_2;
};

void init(p4_state_t& state, const worldstate_t& world)
{
    state.state_airport_1 = world.airport;
    state.state_airport_2 = world.airport;
}

bool next(p4_state_t& state)
{
    for (; state.state_airport_1 != 0; state.state_airport_1 = state.state_airport_1->next)
    {
        if (state.state_airport_1->_0 == state.in_x)
        {
            state.out_ax = state.state_airport_1->_1;

            for (; state.state_airport_2 != 0; state.state_airport_2 = state.state_airport_2->next)
            {
                if (state.state_airport_2->_0 == state.in_y)
                {
                    state.out_ay = state.state_airport_2->_1;
                    state.state_airport_2 = state.state_airport_2->next;
                    return true;
                }
            }
        }
    }

    return false;
}

// tasks

enum task_t
{
    task_none = 0,
    task_root,
    task_travel,
    task_travel_by_air,
    task_ride_taxi,
    task_fly,
};

struct task_ride_taxi_args_t
{
    int x;
    int y;
};

struct task_fly_args_t
{
    int x;
    int y;
};

inline bool is_composite(const task_t& task)
{
    return task >= task_root && task <= task_travel_by_air;
}

struct root_state_t
{
    p1_state_t p1;
};

struct travel_state_t
{
    int x;
    int y;
    p2_state_t p2;
    p3_state_t p3;
};

struct travel_by_air_state_t
{
    int x;
    int y;
    p4_state_t p4;
};

bool root(root_state_t& state, const worldstate_t& world, stack& plan);
bool travel(travel_state_t& state, const worldstate_t& world, stack& plan);
bool travel_by_air(travel_by_air_state_t& state, const worldstate_t& world, stack& plan);

/*
    (:method (root)
        ((start ?s) (finish ?f))
        ((travel ?s ?f))
    )
*/

bool root(root_state_t& state, const worldstate_t& world, stack& plan)
{
    init(state.p1, world);

    while (next(state.p1))
    {
        travel_state_t travel_state;
        travel_state.x = state.p1.out_s;
        travel_state.y = state.p1.out_f;

        stack_ptr rwnd_pos = plan.top_ptr();

        if (travel(travel_state, world, plan))
        {
            return true;
        }

        plan.rewind(rwnd_pos);
    }

    return false;
}

/*
    (:method (travel ?x ?y)
        ((short_distance ?x ?y))
        ((!ride_taxi ?x ?y))

        ((long_distance ?x ?y))
        ((travel_by_air ?x ?y))
    )
*/

bool travel(travel_state_t& state, const worldstate_t& world, stack& plan)
{
    state.p2.in_x = state.x;
    state.p2.in_y = state.y;
    init(state.p2, world);

    while (next(state.p2))
    {
        task_ride_taxi_args_t args;
        args.x = state.x;
        args.y = state.y;

        plan.push(task_ride_taxi);
        plan.push(&args, sizeof(args));

        return true;
    }

    state.p3.in_x = state.x;
    state.p3.in_y = state.y;
    init(state.p3, world);

    while (next(state.p3))
    {
        travel_by_air_state_t travel_by_air_state;
        travel_by_air_state.x = state.x;
        travel_by_air_state.y = state.y;

        stack_ptr rwnd_pos = plan.top_ptr();

        if (travel_by_air(travel_by_air_state, world, plan))
        {
            return true;
        }

        plan.rewind(rwnd_pos);
    }

    return false;
}

/*
    (:method (travel_by_air ?x ?y)
        ((airport ?x ?ax) (airport ?y ?ay))
        ((travel ?x ?ax) (!fly ?ax ?ay) (travel ?y ?ay))
    )
*/

bool travel_by_air(travel_by_air_state_t& state, const worldstate_t& world, stack& plan)
{
    state.p4.in_x = state.x;
    state.p4.in_y = state.y;
    init(state.p4, world);

    while (next(state.p4))
    {
        travel_state_t travel_state;

        travel_state.x = state.x;
        travel_state.y = state.p4.out_ax;

        stack_ptr rwnd_pos = plan.top_ptr();

        if (travel(travel_state, world, plan))         // yield travel(x, y)
        {
            task_fly_args_t args;
            args.x = state.p4.out_ax;
            args.y = state.p4.out_ay;

            plan.push(task_fly);
            plan.push(&args, sizeof(args));

            travel_state.x = state.y;
            travel_state.y = state.p4.out_ay;

            if (travel(travel_state, world, plan))     // yield travel(x, y)
            {
                return true;
            }
        }

        plan.rewind(rwnd_pos);
    }
}

int main()
{
    return 0;
}
