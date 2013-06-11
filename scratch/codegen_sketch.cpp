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

enum task_type
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

struct travel_args_t
{
    int s;
    int f;
};

struct travel_by_air_args_t
{
    int x;
    int y;
};

struct frame_t;

typedef bool (*expand_t)(frame_t*, stack* stack);

struct method_t
{
    expand_t  expand;
    void*     args;
    expand_t  tail;
    method_t* next;
};

struct task_t
{
    task_type type;
    void*     args;
    task_t*   next;
};

struct frame_t
{
    method_t*       method;
    void*           precondition;
    expand_t        expand;
    stack_ptr       rewind;
};

void push(frame_t* frame);

/*
    (:method (root)
        ((start ?s) (finish ?f))
        ((travel ?s ?f))
    )
*/
void root_expand(frame_t* frame, stack* mstack, stack* pstack)
{
}

bool root_branch_0_expand(frame_t* frame, stack* mstack, stack* pstack)
{
    p1_state_t* state = reinterpret_cast<p1_state_t*>(frame.precondition);

    if (next(*state))
    {
        method_t*      m0 = reinterpret_cast<method_t*>(mstack->push_bytes(sizeof(m0)));
        travel_args_t* a0 = reinterpret_cast<method_t*>(mstack->push_bytes(sizeof(a0)));

        a0->s = state->out_s;
        a0->f = state->out_f;

        m0->expand  = travel_banch_0_expand;
        m0->args    = a0;
        m0->tail    = 0;
        m0->next    = 0;

        frame->method = m0;

        return true;
    }

    frame->expand = 0;

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
bool travel_banch_0_expand(frame_t* frame, stack* mstack, stack* pstack)
{
    p2_state_t* state = reinterpret_cast<p2_state_t*>(frame.precondition);

    if (next(*state))
    {
        frame->method = 0;

        task_t*                t0 = reinterpret_cast<task_t*>(pstack->push_bytes(sizeof(t0)));
        task_ride_taxi_args_t* a0 = reinterpret_cast<task_ride_taxi_args_t*>(pstack->push_bytes(sizeof(a0)));

        a0.x = state->in_x;
        a0.y = state->in_y;

        t0->type = task_ride_taxi;
        t0->args = a0;

        return true;
    }

    frame->expand = travel_banch_1_expand;
}

bool travel_banch_1_expand(frame_t* frame, stack* mstack, stack* pstack)
{
    p3_state_t* state = reinterpret_cast<p3_state_t*>(frame.precondition);

    if (next(*state))
    {
        method_t*             m0 = reinterpret_cast<method_t*>(mstack->push_bytes(sizeof(m0)));
        travel_by_air_args_t* a0 = reinterpret_cast<method_t*>(mstack->push_bytes(sizeof(a0)));

        a0->x = state->in_x;
        a0->y = state->in_y;

        m0->expand  = travel_by_air_branch_0_expand;
        m0->args    = a0;
        m0->tail    = 0;
        m0->next    = 0;

        frame->method = m0;

        return true;
    }

    frame->expand = 0;
}

/*
    (:method (travel_by_air ?x ?y)
        ((airport ?x ?ax) (airport ?y ?ay))
        ((travel ?x ?ax) (!fly ?ax ?ay) (travel ?y ?ay))
    )
*/
bool travel_by_air_branch_0_expand(frame_t* frame, stack* mstack, stack* pstack)
{
    p4_state_t* state = reinterpret_cast<p4_state_t*>(frame.precondition);

    if (next(*state))
    {
        method_t*      m0 = reinterpret_cast<method_t*>(mstack->push_bytes(sizeof(m0)));
        travel_args_t* a0 = reinterpret_cast<method_t*>(mstack->push_bytes(sizeof(a0)));

        method_t*      m1 = reinterpret_cast<method_t*>(mstack->push_bytes(sizeof(m1)));
        travel_args_t* a1 = reinterpret_cast<method_t*>(mstack->push_bytes(sizeof(a1)));

        a0->s = state->in_x;
        a0->f = state->out_ax;

        a1->s = state->in_y;
        a1->f = state->out_ay;

        m0->expand  = travel_banch_0_expand;
        m0->args    = a0;
        m0->tail    = travel_by_air_branch_0_tail_0_expand;
        m0->next    = m1;

        m1->expand  = travel_banch_0_expand;
        m1->args    = a1;
        m1->tail    = 0;
        m1->next    = 0;

        return true;
    }

    frame->expand = 0;

    return false;
}

bool travel_by_air_branch_0_tail_0_expand(frame_t* frame, stack* mstack, stack* pstack)
{
    p4_state_t* state = reinterpret_cast<p4_state_t*>(frame.precondition);

    task_t*          t0 = reinterpret_cast<task_t*>(pstack->push_bytes(sizeof(t0)));
    task_fly_args_t* a0 = reinterpret_cast<task_fly_args_t*>(sizeof(a0));

    a0.x = state->out_ax;
    a0.y = state->out_ay;

    t0->type = task_fly;
    t0->args = a0;
}

void expand_method(stack* mstack, frame_t* frame)
{
    frame_t* new_frame = reinterpret_cast<frame_t*>(mstack.push_bytes(sizeof(new_frame)));
}

bool find_plan(stack& mstack, stack& pstack)
{
    while (top(mstack))
    {
        frame_t* frame = top(mstack);

        // no more branches to try => method expansion failed
        // - parent method shall try another binding
        if (!frame->expand)
        {
            pstack.rewind(frame->rewind);
            pop(mstack);
            continue;
        }

        // ask current branch' precondition for a new binding
        // - expands task list if the satisfying binding is found.
        if (frame->expand(frame))
        {
            // this method expansion has another methods
            // - expand them one by one.
            if (frame->method)
            {
                expand_method(mstack, frame);
            }
            else
            {
                // this method has expanded to primitive tasks
                // - move to the next method in a task list of the parent.

                if (frame->method->tail)
                {
                    frame->method->tail(frame);
                }

                pop(mstack);

                frame_t* parent = top(mstack);

                while (parent && !parent->method->next)
                {
                    if (parent->method->tail)
                    {
                        parent->method->tail(parent);
                    }

                    pop(mstack);
                    parent = top(mstack);
                }

                // top method successfully expanded
                // - the plan is found.
                if (!parent)
                {
                    return true;
                }

                if (parent->method->tail)
                {
                    parent->method->tail(parent);
                }

                parent->method = parent->method->next;

                expand_method(mstack, parent);
            }
        }
    }

    return false;
}

int main()
{
    return 0;
}
