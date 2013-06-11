#include <stddef.h> // size_t

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
*/

struct stack
{
    stack(size_t size)
    {
        data_ = new char[size];
        top_ = data_;
    }

    ~stack()
    {
        delete [] data_;
    }

    void* top() const
    {
        return top_;
    }

    template<typename T>
    T* push()
    {
        top_ += sizeof(T);
    }

    void rewind(void* p)
    {
        top_ = static_cast<char*>(p);
    }

    char* data_;
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

enum task_type_t
{
    task_none = 0,
    task_root,
    task_travel,
    task_travel_by_air,
    task_ride_taxi,
    task_fly,
};

struct ride_taxi_args_t
{
    int x;
    int y;
};

struct fly_args_t
{
    int x;
    int y;
};

struct travel_args_t
{
    int x;
    int y;
};

struct travel_by_air_args_t
{
    int x;
    int y;
};

struct branch_t;

typedef bool (*expand_f)(branch_t*, stack&, stack&, const worldstate_t&);

struct method_t;

struct branch_t
{
    void*           rewind;       // stack top before pushing this branch.
    void*           args;         // argument struct.
    void*           precondition; // branch precondition state.
    expand_f        expand;       // function which expands the branch task list.
    method_t*       method;       // next method in the branch task list to expand.
    void*           mrewind;      // stack top before expanding this branch.
    void*           prewind;      // primitive task stack top before expanding this branch.
    branch_t*       previous;     // parent branch on a stack.
};

struct method_t
{
    void*     args;        // struct with arguments.
    expand_f  tail;        // expand primitive tasks which follow after this method.
    expand_f  expand;      // method expansion.
    method_t* next;        // next method in task list.
};

struct task_t
{
    task_type_t type;
    void*       args;
};

// forwards
bool root_expand(branch_t*, stack&, stack&, const worldstate_t&);
bool root_branch_0_expand(branch_t*, stack&, stack&, const worldstate_t&);
bool travel_expand(branch_t*, stack&, stack&, const worldstate_t&);
bool travel_branch_0_expand(branch_t*, stack&, stack&, const worldstate_t&);
bool travel_branch_1_expand(branch_t*, stack&, stack&, const worldstate_t&);
bool travel_by_air_expand(branch_t*, stack&, stack&, const worldstate_t&);
bool travel_by_air_branch_0_expand(branch_t*, stack&, stack&, const worldstate_t&);
bool travel_by_air_branch_0_0_tail(branch_t*, stack&, stack&, const worldstate_t&);

/*
    (:method (root)
        ((start ?s) (finish ?f))
        ((travel ?s ?f))
    )
*/
bool root_expand(branch_t* branch, stack& mstack, stack& pstack, const worldstate_t& world)
{
    branch->mrewind = mstack.top();
    branch->prewind = pstack.top();
    // init precondition state.
    p1_state_t* state = mstack.push<p1_state_t>();
    init(*state, world);
    // init branch
    branch->precondition = state;
    branch->expand = root_branch_0_expand;
    // expand branch.
    return root_branch_0_expand(branch, mstack, pstack, world);
}

bool root_branch_0_expand(branch_t* branch, stack& mstack, stack& pstack, const worldstate_t& world)
{
    p1_state_t* state = reinterpret_cast<p1_state_t*>(branch->precondition);
    // produce next binding
    if (next(*state))
    {
        method_t*      m0 = mstack.push<method_t>();
        travel_args_t* a0 = mstack.push<travel_args_t>();
        m0->args = a0;
        m0->tail = 0;
        m0->expand = travel_expand;
        m0->next = 0;
        a0->x = state->out_s;
        a0->y = state->out_f;
        branch->method = m0;
        return true;
    }
    // when there're no more bindings, set next branch for expansion
    branch->expand = 0;
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
bool travel_expand(branch_t* branch, stack& mstack, stack& pstack, const worldstate_t& world)
{
    branch->mrewind = mstack.top();
    branch->prewind = pstack.top();
    p2_state_t* state = mstack.push<p2_state_t>();
    travel_args_t* args = reinterpret_cast<travel_args_t*>(branch->args);
    state->in_x = args->x;
    state->in_y = args->y;
    init(*state, world);
    branch->precondition = state;
    branch->expand = travel_branch_0_expand;
    return travel_branch_0_expand(branch, mstack, pstack, world);
}

bool travel_branch_0_expand(branch_t* branch, stack& mstack, stack& pstack, const worldstate_t& world)
{
    p2_state_t* state = reinterpret_cast<p2_state_t*>(branch->precondition);
    travel_args_t* args = reinterpret_cast<travel_args_t*>(branch->args);

    if (next(*state))
    {
        task_t*           t0 = pstack.push<task_t>();
        ride_taxi_args_t* a0 = pstack.push<ride_taxi_args_t>();
        t0->type = task_ride_taxi;
        t0->args = a0;
        a0->x = args->x;
        a0->y = args->y;
        branch->method = 0;
        return true;
    }

    // initialize next branch precondition
    {
        p3_state_t* state = mstack.push<p3_state_t>();
        state->in_x = args->x;
        state->in_y = args->y;
        init(*state, world);
        branch->precondition = state;
        branch->expand = travel_branch_1_expand;
    }

    return travel_branch_1_expand(branch, mstack, pstack, world);
}

bool travel_branch_1_expand(branch_t* branch, stack& mstack, stack& pstack, const worldstate_t& world)
{
    p3_state_t* state = reinterpret_cast<p3_state_t*>(branch->precondition);
    travel_args_t* args = reinterpret_cast<travel_args_t*>(branch->args);

    if (next(*state))
    {
        method_t*             m0 = mstack.push<method_t>();
        travel_by_air_args_t* a0 = mstack.push<travel_by_air_args_t>();
        m0->args = a0;
        m0->tail = 0;
        m0->expand = travel_by_air_expand;
        m0->next = 0;
        a0->x = args->x;
        a0->y = args->y;
        branch->method = m0;
        return true;
    }

    branch->expand = 0;
    return false;
}

/*
    (:method (travel_by_air ?x ?y)
        ((airport ?x ?ax) (airport ?y ?ay))
        ((travel ?x ?ax) (!fly ?ax ?ay) (travel ?y ?ay))
    )
*/
bool travel_by_air_expand(branch_t* branch, stack& mstack, stack& pstack, const worldstate_t& world)
{
    branch->mrewind = mstack.top();
    branch->prewind = pstack.top();
    p4_state_t* state = mstack.push<p4_state_t>();
    travel_by_air_args_t* args = reinterpret_cast<travel_by_air_args_t*>(branch->args);
    state->in_x = args->x;
    state->in_y = args->y;
    init(*state, world);
    branch->precondition = state;
    branch->expand = travel_by_air_branch_0_expand;
    return travel_by_air_branch_0_expand(branch, mstack, pstack, world);
}

bool travel_by_air_branch_0_expand(branch_t* branch, stack& mstack, stack& pstack, const worldstate_t& world)
{
    p4_state_t* state = reinterpret_cast<p4_state_t*>(branch->precondition);
    travel_by_air_args_t* args = reinterpret_cast<travel_by_air_args_t*>(branch->args);

    if (next(*state))
    {
        method_t*      m0 = mstack.push<method_t>();
        travel_args_t* a0 = mstack.push<travel_args_t>();

        method_t*      m1 = mstack.push<method_t>();
        travel_args_t* a1 = mstack.push<travel_args_t>();

        m0->args = a0;
        m0->tail = travel_by_air_branch_0_0_tail;
        m0->expand = travel_expand;
        m0->next = m1;
        a0->x = args->x;
        a0->y = state->out_ax;

        m1->args = a1;
        m1->tail = 0;
        m1->expand = travel_expand;
        m1->next = 0;
        a1->x = args->y;
        a1->y = state->out_ay;

        branch->method = m0;
        return true;
    }

    branch->expand = 0;
    return false;
}

bool travel_by_air_branch_0_0_tail(branch_t* branch, stack& mstack, stack& pstack, const worldstate_t& world)
{
    p4_state_t* state = reinterpret_cast<p4_state_t*>(branch->precondition);
    travel_by_air_args_t* args = reinterpret_cast<travel_by_air_args_t*>(branch->args);
    (void)args;

    task_t*     t0 = pstack.push<task_t>();
    fly_args_t* a0 = pstack.push<fly_args_t>();
    t0->type = task_fly;
    t0->args = a0;
    a0->x = state->out_ax;
    a0->y = state->out_ay;
    return true;
}

// runtime

void push_method(branch_t* branch, stack& mstack)
{
    void* t = mstack.top();
    branch_t* new_branch = mstack.push<branch_t>();
    new_branch->rewind = t;
    new_branch->args = branch->method->args;
    new_branch->precondition = 0;
    new_branch->expand = branch->method->expand;
    new_branch->method = 0;
    new_branch->mrewind = 0;
    new_branch->prewind = 0;
    new_branch->previous = branch;
}

void pop(branch_t*& branch, stack& mstack)
{
    branch_t* previous = branch->previous;
    mstack.rewind(branch->rewind);
    branch = previous;
}

bool find_plan(stack& mstack, stack& pstack, const worldstate_t& world)
{
    branch_t* root = mstack.push<branch_t>();
    root->args = 0;
    root->precondition = 0;
    root->expand = root_expand;
    root->method = 0;
    root->mrewind = mstack.top();
    root->prewind = pstack.top();
    root->previous = 0;

    branch_t* branch = root;

    while (branch)
    {
        if (branch->expand(branch, mstack, pstack, world))
        {
            // only primitive tasks are in the task list
            if (!branch->method)
            {
                pop(branch, mstack);

                while (branch && !branch->method->next)
                {
                    // produce tail tasks
                    // tail tasks are primitive tasks situated in the tasklist
                    // after expanded method and before the next method.
                    if (branch->method->tail)
                    {
                        branch->method->tail(branch, mstack, pstack, world);
                    }

                    pop(branch, mstack);
                }

                // top method has been successfully expanded
                if (!branch)
                {
                    return true;
                }

                // otherwise move on to the next method in the tasklist.
                branch->method = branch->method->next;
                push_method(branch, mstack);
            }
            // if there are methods to expand in the task list
            else
            {
                // push method's branch on the stack
                push_method(branch, mstack);
            }
        }
        else
        {
            // failed to expand method => backtrack
            pop(branch, mstack);

            // clear parent expansion
            if (branch)
            {
                mstack.rewind(branch->mrewind);
                pstack.rewind(branch->prewind);
            }
        }
    }

    return false;
}

int main()
{
    return 0;
}
