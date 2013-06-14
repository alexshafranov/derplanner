#include <stdio.h>
#include <stddef.h> // size_t

// for test
#include <iostream>
#include <chrono>

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
        object_ = 0;
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
        return object_;
    }

    template<typename T>
    T* begin_object()
    {
        T* o = push<T>();
        object_ = o;
        return o;
    }

    template<typename T>
    T* push()
    {
        T* addr = reinterpret_cast<T*>(top_);
        top_ += sizeof(T);
        return addr;
    }

    void rewind(void* p)
    {
        top_ = static_cast<char*>(p);
    }

    char* data_;
    void* object_;
    char* top_;
};

// world state

struct start_tuple
{
    int _0;
    start_tuple* next;
};

struct finish_tuple
{
    int _0;
    finish_tuple* next;
};

struct short_distance_tuple
{
    int _0;
    int _1;
    short_distance_tuple* next;
};

struct long_distance_tuple
{
    int _0;
    int _1;
    long_distance_tuple* next;
};

struct airport_tuple
{
    int _0;
    int _1;
    airport_tuple* next;
};

struct worldstate
{
    start_tuple*          start;
    finish_tuple*         finish;
    short_distance_tuple* short_distance;
    long_distance_tuple*  long_distance;
    airport_tuple*        airport;
};

// preconditions

// ((start ?s) (finish ?f))
struct p1_state
{
    int out_s;
    int out_f;
    start_tuple*  state_start;
    finish_tuple* state_finish;
};

void init(p1_state& state, const worldstate& world)
{
    state.state_start  = world.start;
    state.state_finish = world.finish;
}

bool next(p1_state& state)
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
struct p2_state
{
    int in_x;
    int in_y;
    short_distance_tuple* state_short_distance;
};

void init(p2_state& state, const worldstate& world)
{
    state.state_short_distance = world.short_distance;
}

bool next(p2_state& state)
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

struct p3_state
{
    int in_x;
    int in_y;
    long_distance_tuple* state_long_distance;
};

void init(p3_state& state, const worldstate& world)
{
    state.state_long_distance = world.long_distance;
}

bool next(p3_state& state)
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
struct p4_state
{
    int in_x;
    int in_y;

    int out_ax;
    int out_ay;

    airport_tuple* state_airport_1;
    airport_tuple* state_airport_2;
};

void init(p4_state& state, const worldstate& world)
{
    state.state_airport_1 = world.airport;
    state.state_airport_2 = world.airport;
}

bool next(p4_state& state)
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

struct ride_taxi_args
{
    int x;
    int y;
};

struct fly_args
{
    int x;
    int y;
};

struct travel_args
{
    int x;
    int y;
};

struct travel_by_air_args
{
    int x;
    int y;
};

struct method_expansion;

typedef bool (*expand_func)(method_expansion*, stack&, stack&, const worldstate&);

struct method_instance;

struct method_expansion
{
    void*               rewind;       // stack top before pushing this branch.
    void*               args;         // argument struct.
    void*               precondition; // branch precondition state.
    expand_func         expand;       // function which expands the branch task list.
    method_instance*    method;       // next method in the branch task list to expand.
    void*               mrewind;      // stack top before expanding this branch.
    void*               prewind;      // primitive task stack top before expanding this branch.
    method_expansion*   previous;     // parent branch on a stack.
};

struct method_instance
{
    void*               args;     // struct with arguments.
    expand_func         tail;     // expand primitive tasks which follow after this method.
    expand_func         expand;   // method expansion.
    method_instance*    next;     // next method in task list.
};

struct task_instance
{
    task_type         type;    // type of the task.
    void*             args;    // pointer to the arguments struct.
    task_instance*    link;    // 'previous' task during find_plan, reversed when plan is found.
};

// forwards
bool root_expand(method_expansion*, stack&, stack&, const worldstate&);
bool root_branch_0_expand(method_expansion*, stack&, stack&, const worldstate&);
bool travel_expand(method_expansion*, stack&, stack&, const worldstate&);
bool travel_branch_0_expand(method_expansion*, stack&, stack&, const worldstate&);
bool travel_branch_1_expand(method_expansion*, stack&, stack&, const worldstate&);
bool travel_by_air_expand(method_expansion*, stack&, stack&, const worldstate&);
bool travel_by_air_branch_0_expand(method_expansion*, stack&, stack&, const worldstate&);
bool travel_by_air_branch_0_0_tail(method_expansion*, stack&, stack&, const worldstate&);

/*
    (:method (root)
        ((start ?s) (finish ?f))
        ((travel ?s ?f))
    )
*/
bool root_expand(method_expansion* expansion, stack& mstack, stack& pstack, const worldstate& world)
{
    expansion->mrewind = mstack.top();
    expansion->prewind = pstack.top();
    // init precondition state.
    p1_state* state = mstack.push<p1_state>();
    init(*state, world);
    // init branch
    expansion->precondition = state;
    expansion->expand = root_branch_0_expand;
    // expand branch.
    return root_branch_0_expand(expansion, mstack, pstack, world);
}

bool root_branch_0_expand(method_expansion* expansion, stack& mstack, stack& pstack, const worldstate& world)
{
    p1_state* state = static_cast<p1_state*>(expansion->precondition);
    // produce next binding
    if (next(*state))
    {
        method_instance* m0 = mstack.push<method_instance>();
        travel_args* a0 = mstack.push<travel_args>();
        m0->args = a0;
        m0->tail = 0;
        m0->expand = travel_expand;
        m0->next = 0;
        a0->x = state->out_s;
        a0->y = state->out_f;
        expansion->method = m0;
        return true;
    }

    expansion->expand = 0;
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
bool travel_expand(method_expansion* expansion, stack& mstack, stack& pstack, const worldstate& world)
{
    expansion->mrewind = mstack.top();
    expansion->prewind = pstack.top();
    p2_state* state = mstack.push<p2_state>();
    travel_args* args = static_cast<travel_args*>(expansion->args);
    state->in_x = args->x;
    state->in_y = args->y;
    init(*state, world);
    expansion->precondition = state;
    expansion->expand = travel_branch_0_expand;
    return travel_branch_0_expand(expansion, mstack, pstack, world);
}

bool travel_branch_0_expand(method_expansion* expansion, stack& mstack, stack& pstack, const worldstate& world)
{
    p2_state* state = static_cast<p2_state*>(expansion->precondition);
    travel_args* args = static_cast<travel_args*>(expansion->args);

    if (next(*state))
    {
        task_instance* previous = static_cast<task_instance*>(pstack.object());
        task_instance*           t0 = pstack.begin_object<task_instance>();
        ride_taxi_args* a0 = pstack.push<ride_taxi_args>();
        t0->type = task_ride_taxi;
        t0->args = a0;
        a0->x = args->x;
        a0->y = args->y;
        t0->link = previous;
        expansion->method = 0;
        return true;
    }

    // initialize next branch precondition
    {
        p3_state* state = mstack.push<p3_state>();
        state->in_x = args->x;
        state->in_y = args->y;
        init(*state, world);
        expansion->precondition = state;
        expansion->expand = travel_branch_1_expand;
    }

    return travel_branch_1_expand(expansion, mstack, pstack, world);
}

bool travel_branch_1_expand(method_expansion* expansion, stack& mstack, stack& pstack, const worldstate& world)
{
    p3_state* state = static_cast<p3_state*>(expansion->precondition);
    travel_args* args = static_cast<travel_args*>(expansion->args);

    if (next(*state))
    {
        method_instance* m0 = mstack.push<method_instance>();
        travel_by_air_args* a0 = mstack.push<travel_by_air_args>();
        m0->args = a0;
        m0->tail = 0;
        m0->expand = travel_by_air_expand;
        m0->next = 0;
        a0->x = args->x;
        a0->y = args->y;
        expansion->method = m0;
        return true;
    }

    expansion->expand = 0;
    return false;
}

/*
    (:method (travel_by_air ?x ?y)
        ((airport ?x ?ax) (airport ?y ?ay))
        ((travel ?x ?ax) (!fly ?ax ?ay) (travel ?y ?ay))
    )
*/
bool travel_by_air_expand(method_expansion* expansion, stack& mstack, stack& pstack, const worldstate& world)
{
    expansion->mrewind = mstack.top();
    expansion->prewind = pstack.top();
    p4_state* state = mstack.push<p4_state>();
    travel_by_air_args* args = static_cast<travel_by_air_args*>(expansion->args);
    state->in_x = args->x;
    state->in_y = args->y;
    init(*state, world);
    expansion->precondition = state;
    expansion->expand = travel_by_air_branch_0_expand;
    return travel_by_air_branch_0_expand(expansion, mstack, pstack, world);
}

bool travel_by_air_branch_0_expand(method_expansion* expansion, stack& mstack, stack& pstack, const worldstate& world)
{
    p4_state* state = static_cast<p4_state*>(expansion->precondition);
    travel_by_air_args* args = static_cast<travel_by_air_args*>(expansion->args);

    if (next(*state))
    {
        method_instance* m0 = mstack.push<method_instance>();
        travel_args* a0 = mstack.push<travel_args>();

        method_instance* m1 = mstack.push<method_instance>();
        travel_args* a1 = mstack.push<travel_args>();

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

        expansion->method = m0;
        return true;
    }

    expansion->expand = 0;
    return false;
}

bool travel_by_air_branch_0_0_tail(method_expansion* expansion, stack& mstack, stack& pstack, const worldstate& world)
{
    p4_state* state = static_cast<p4_state*>(expansion->precondition);
    travel_by_air_args* args = static_cast<travel_by_air_args*>(expansion->args);
    (void)args;

    task_instance*     previous = static_cast<task_instance*>(pstack.object());
    task_instance*     t0 = pstack.begin_object<task_instance>();
    fly_args* a0 = pstack.push<fly_args>();
    t0->type = task_fly;
    t0->args = a0;
    a0->x = state->out_ax;
    a0->y = state->out_ay;
    t0->link = previous;
    return true;
}

// runtime

method_expansion* push_method(method_expansion* parent, stack& mstack)
{
    void* t = mstack.top();
    method_expansion* new_branch = mstack.begin_object<method_expansion>();
    new_branch->rewind = t;
    new_branch->args = parent->method->args;
    new_branch->precondition = 0;
    new_branch->expand = parent->method->expand;
    new_branch->method = 0;
    new_branch->mrewind = 0;
    new_branch->prewind = 0;
    new_branch->previous = parent;
    return new_branch;
}

method_expansion* top(stack& mstack)
{
    return static_cast<method_expansion*>(mstack.object());
}

void pop(stack& mstack)
{
    method_expansion* b = top(mstack);
    method_expansion* p = b->previous;
    mstack.rewind(b->rewind);
    mstack.object_ = p;
}

bool find_plan(const worldstate& world, stack& mstack, stack& pstack)
{
    method_expansion* root = mstack.begin_object<method_expansion>();
    root->args = 0;
    root->precondition = 0;
    root->expand = root_expand;
    root->method = 0;
    root->mrewind = mstack.top();
    root->prewind = pstack.top();
    root->previous = 0;

    while (method_expansion* e = top(mstack))
    {
        if (e->expand(e, mstack, pstack, world))
        {
            if (!e->method)
            {
                // only primitive tasks are in the task list
                // go up, finalizing parents by producing tail primitive tasks,
                // until we find parent expansion with next method instance to expand

                pop(mstack);

                for (e=top(mstack); e != 0; pop(mstack), e=top(mstack))
                {
                    // produce tail tasks
                    // tail tasks are primitive tasks situated in the tasklist
                    // after the method instance and before the next method instance.
                    if (e->method->tail)
                    {
                        e->method->tail(e, mstack, pstack, world);
                    }

                    if (e->method->next)
                    {
                        break;
                    }
                }

                // top method has been successfully expanded
                // i.e. the plan is found
                // (and stored as a primitive task instance list in pstack)
                if (!e)
                {
                    return true;
                }

                // otherwise move on to the next method instance in the tasklist.
                e->method = e->method->next;
                e = push_method(e, mstack);
            }
            else
            {
                // there're method instances to expand.
                // push method's expansion on the stack
                e = push_method(e, mstack);
            }
        }
        else
        {
            // failed to expand method => backtrack
            pop(mstack);

            e = top(mstack);

            // clear parent expansion
            if (e)
            {
                mstack.rewind(e->mrewind);
                pstack.rewind(e->prewind);
            }
        }
    }

    return false;
}

task_instance* reverse_task_list(task_instance* head)
{
    task_instance* new_head = 0;

    while (head)
    {
        task_instance* link = head->link;
        head->link = new_head;
        new_head = head;
        head = link;
    }

    return new_head;
}

void print_task_ride_taxi(const task_instance* task)
{
    ride_taxi_args* args = static_cast<ride_taxi_args*>(task->args);
    printf("!ride_taxi(%d, %d)\n", args->x, args->y);
}

void print_task_fly(const task_instance* task)
{
    fly_args* args = static_cast<fly_args*>(task->args);
    printf("!fly(%d, %d)\n", args->x, args->y);
}

void print_task(const task_instance* task)
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
    worldstate world;

    world.start =  new start_tuple[1];
    world.finish = new finish_tuple[1];
    world.short_distance = new short_distance_tuple[4];
    world.long_distance = new long_distance_tuple[8];
    world.airport = new airport_tuple[2];

    world.start->_0 = spb;
    world.start->next = 0;

    world.finish->_0 = msc;
    world.finish->next = 0;

    short_distance_tuple* s = world.short_distance;
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

    long_distance_tuple* l = world.long_distance;
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

    airport_tuple* a = world.airport;
    a->_0 = spb;
    a->_1 = led;
    a->next = a+1;
    a++;

    a->_0 = msc;
    a->_1 = svo;
    a->next = 0;

    stack mstack(32768);
    stack pstack(32768);

    auto timestamp = std::chrono::high_resolution_clock::now();

    for (int i=0; i<1000000; ++i)
    {
        find_plan(world, mstack, pstack);

        mstack.top_ = mstack.data_;
        mstack.object_ = 0;

        pstack.top_ = pstack.data_;
        pstack.object_ = 0;
    }

    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - timestamp);
    std::cout << "elapsed: " << elapsed.count() / 1000.f << " s" << std::endl;

    bool result = find_plan(world, mstack, pstack);

    if (result)
    {
        task_instance* task = reverse_task_list(static_cast<task_instance*>(pstack.object()));

        // print plan
        for (task_instance* t = task; t != 0; t = t->link)
        {
            print_task(t);
        }
    }
    else
    {
        printf("plan not found.\n");
    }

    delete [] world.start;
    delete [] world.finish;
    delete [] world.short_distance;
    delete [] world.long_distance;
    delete [] world.airport;

    return 0;
}
