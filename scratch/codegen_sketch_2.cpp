#include <stdio.h>
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
        start_ = 0;
    }

    ~stack()
    {
        delete [] data_;
    }

    void* top() const
    {
        return top_;
    }

    void* object() const
    {
        return start_;
    }

    template<typename T>
    T* push(bool start_object=false)
    {
        T* addr = reinterpret_cast<T*>(top_);
        top_ += sizeof(T);

        if (start_object)
        {
            start_ = addr;
        }

        return addr;
    }

    void rewind(void* p)
    {
        top_ = static_cast<char*>(p);
    }

    char* data_;
    void* start_;
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

const char* name(task_type_t task)
{
    switch (task)
    {
    case task_none:
        return "none";
    case task_root:
        return "root";
    case task_travel:
        return "travel";
    case task_travel_by_air:
        return "travel_by_air";
    case task_ride_taxi:
        return "!ride_taxi";
    case task_fly:
        return "!fly";
    }

    return "error";
}

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
    task_t*     next;
    task_t*     previous;
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
    p1_state_t* state = static_cast<p1_state_t*>(branch->precondition);
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
    travel_args_t* args = static_cast<travel_args_t*>(branch->args);
    state->in_x = args->x;
    state->in_y = args->y;
    init(*state, world);
    branch->precondition = state;
    branch->expand = travel_branch_0_expand;
    return travel_branch_0_expand(branch, mstack, pstack, world);
}

bool travel_branch_0_expand(branch_t* branch, stack& mstack, stack& pstack, const worldstate_t& world)
{
    p2_state_t* state = static_cast<p2_state_t*>(branch->precondition);
    travel_args_t* args = static_cast<travel_args_t*>(branch->args);

    if (next(*state))
    {
        task_t* previous = static_cast<task_t*>(pstack.object());
        task_t*           t0 = pstack.push<task_t>(true);
        ride_taxi_args_t* a0 = pstack.push<ride_taxi_args_t>();
        t0->type = task_ride_taxi;
        t0->args = a0;
        a0->x = args->x;
        a0->y = args->y;
        t0->previous = previous;
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
    p3_state_t* state = static_cast<p3_state_t*>(branch->precondition);
    travel_args_t* args = static_cast<travel_args_t*>(branch->args);

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
    travel_by_air_args_t* args = static_cast<travel_by_air_args_t*>(branch->args);
    state->in_x = args->x;
    state->in_y = args->y;
    init(*state, world);
    branch->precondition = state;
    branch->expand = travel_by_air_branch_0_expand;
    return travel_by_air_branch_0_expand(branch, mstack, pstack, world);
}

bool travel_by_air_branch_0_expand(branch_t* branch, stack& mstack, stack& pstack, const worldstate_t& world)
{
    p4_state_t* state = static_cast<p4_state_t*>(branch->precondition);
    travel_by_air_args_t* args = static_cast<travel_by_air_args_t*>(branch->args);

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
        a1->x = state->out_ay;
        a1->y = args->y;

        branch->method = m0;
        return true;
    }

    branch->expand = 0;
    return false;
}

bool travel_by_air_branch_0_0_tail(branch_t* branch, stack& mstack, stack& pstack, const worldstate_t& world)
{
    p4_state_t* state = static_cast<p4_state_t*>(branch->precondition);
    travel_by_air_args_t* args = static_cast<travel_by_air_args_t*>(branch->args);
    (void)args;

    task_t*     previous = static_cast<task_t*>(pstack.object());
    task_t*     t0 = pstack.push<task_t>(true);
    fly_args_t* a0 = pstack.push<fly_args_t>();
    t0->type = task_fly;
    t0->args = a0;
    a0->x = state->out_ax;
    a0->y = state->out_ay;
    t0->previous = previous;
    return true;
}

// runtime

branch_t* push_method(branch_t* branch, stack& mstack)
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
    return new_branch;
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

                while (branch)
                {
                    // produce tail tasks
                    // tail tasks are primitive tasks situated in the tasklist
                    // after expanded method and before the next method.
                    if (branch->method->tail)
                    {
                        branch->method->tail(branch, mstack, pstack, world);
                    }

                    if (branch->method->next)
                    {
                        break;
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
                branch = push_method(branch, mstack);
            }
            // if there are methods to expand in the task list
            else
            {
                // push method's branch on the stack
                branch = push_method(branch, mstack);
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

void print_task_ride_taxi(const task_t* task)
{
    ride_taxi_args_t* args = static_cast<ride_taxi_args_t*>(task->args);
    printf("!ride_taxi(%d, %d)\n", args->x, args->y);
}

void print_task_fly(const task_t* task)
{
    fly_args_t* args = static_cast<fly_args_t*>(task->args);
    printf("!fly(%d, %d)\n", args->x, args->y);
}

void print_task(const task_t* task)
{
    switch (task->type)
    {
    case task_ride_taxi:
        print_task_ride_taxi(task);
        break;
    case task_fly:
        print_task_fly(task);
        break;
    default:
        printf("<error>\n");
        break;
    }
}

int main()
{
    const int spb = 0;
    const int led = 1;
    const int svo = 2;
    const int msc = 3;
/*
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
*/
    worldstate_t world;

    world.start =  new start_tuple_t[1];
    world.finish = new finish_tuple_t[1];
    world.short_distance = new short_distance_tuple_t[4];
    world.long_distance = new long_distance_tuple_t[8];
    world.airport = new airport_tuple_t[2];

    world.start->_0 = spb;
    world.start->next = 0;

    world.finish->_0 = msc;
    world.finish->next = 0;

    short_distance_tuple_t* s = world.short_distance;
    s->_0 = spb;
    s->_1 = led;
    s->next = s+1;
    s++;

    s->_0 = led;
    s->_1 = spb;
    s->next = s+1;
    s++;

    s->_0 = msc;
    s->_1 = svo;
    s->next = s+1;
    s++;

    s->_0 = svo;
    s->_1 = msc;
    s->next = 0;

    long_distance_tuple_t* l = world.long_distance;
    l->_0 = spb;
    l->_1 = msc;
    l->next = l+1;
    l++;

    l->_0 = msc;
    l->_1 = spb;
    l->next = l+1;
    l++;

    l->_0 = led;
    l->_1 = svo;
    l->next = l+1;
    l++;

    l->_0 = svo;
    l->_1 = led;
    l->next = l+1;
    l++;

    l->_0 = spb;
    l->_1 = svo;
    l->next = l+1;
    l++;

    l->_0 = svo;
    l->_1 = spb;
    l->next = l+1;
    l++;

    l->_0 = msc;
    l->_1 = led;
    l->next = l+1;
    l++;

    l->_0 = led;
    l->_1 = msc;
    l->next = 0;

    airport_tuple_t* a = world.airport;
    a->_0 = spb;
    a->_1 = led;
    a->next = a+1;
    a++;

    a->_0 = msc;
    a->_1 = svo;
    a->next = 0;

    stack mstack(32768);
    stack pstack(32768);

    bool result = find_plan(mstack, pstack, world);

    printf("result=%d\n", result);

    // compute forward task order
    task_t* task = static_cast<task_t*>(pstack.object());

    while (task->previous)
    {
        task->previous->next = task;
        task = task->previous;
    }

    // print plan
    for (task_t* t = task; t != 0; t = t->next)
    {
        print_task(t);
    }

    delete [] world.start;
    delete [] world.finish;
    delete [] world.short_distance;
    delete [] world.long_distance;
    delete [] world.airport;

    return 0;
}
