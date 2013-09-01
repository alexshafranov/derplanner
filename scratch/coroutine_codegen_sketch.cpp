#include <stdio.h>
#include <stddef.h> // size_t

// for test
#include <iostream>
#include <chrono>

#define PLNNRC_COROUTINE_BEGIN(state) switch ((state).stage) { case 0:
#define PLNNRC_COROUTINE_YIELD(state) do { (state).stage = __LINE__; return true; case __LINE__:; } while (0)
#define PLNNRC_COROUTINE_END() } return false

#define PLNRC_PRINT printf

// Stack

class stack
{
public:
    stack(size_t capacity)
        : _capacity(capacity)
    {
        _buffer = new char[_capacity];
        _top = _buffer;
    }

    ~stack()
    {
        delete [] _buffer;
    }

    template <typename T>
    T* push()
    {
        char* top = _top;
        _top += sizeof(T);
        return reinterpret_cast<T*>(top);
    }

    void* top()
    {
        return _top;
    }

    void rewind(void* top)
    {
        _top = static_cast<char*>(top);
    }

    void reset()
    {
        _top = _buffer;
    }

private:
    size_t _capacity;
    char* _buffer;
    char* _top;
};

// World State

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
    start_tuple* start;
    finish_tuple* finish;
    short_distance_tuple* short_distance;
    long_distance_tuple* long_distance;
    airport_tuple* airport;
};

struct p0_state
{
    int _0;
    int _1;
    start_tuple* start_0;
    finish_tuple* finish_1;
    int stage;
};

bool next(p0_state& state, worldstate& world)
{
    //PLNRC_PRINT("p0_state next, stage=%d\n", state.stage);

    PLNNRC_COROUTINE_BEGIN(state);

    for (state.start_0 = world.start; state.start_0 != 0; state.start_0 = state.start_0->next)
    {
        state._0 = state.start_0->_0;

        for (state.finish_1 = world.finish; state.finish_1 != 0; state.finish_1 = state.finish_1->next)
        {
            state._1 = state.finish_1->_0;

            PLNNRC_COROUTINE_YIELD(state);
        }
    }

    PLNNRC_COROUTINE_END();
}

struct p1_state
{
    int _0;
    int _1;
    short_distance_tuple* short_distance_0;
    int stage;
};

bool next(p1_state& state, worldstate& world)
{
    //PLNRC_PRINT("p1_state next, stage=%d\n", state.stage);

    PLNNRC_COROUTINE_BEGIN(state);

    for (state.short_distance_0 = world.short_distance; state.short_distance_0 != 0; state.short_distance_0 = state.short_distance_0->next)
    {
        if (state.short_distance_0->_0 != state._0)
        {
            continue;
        }

        if (state.short_distance_0->_1 != state._1)
        {
            continue;
        }

        PLNNRC_COROUTINE_YIELD(state);
    }

    PLNNRC_COROUTINE_END();
}

struct p2_state
{
    int _0;
    int _1;
    long_distance_tuple* long_distance_0;
    int stage;
};

bool next(p2_state& state, worldstate& world)
{
    //PLNRC_PRINT("p2_state next, stage=%d\n", state.stage);

    PLNNRC_COROUTINE_BEGIN(state);

    for (state.long_distance_0 = world.long_distance; state.long_distance_0 != 0; state.long_distance_0 = state.long_distance_0->next)
    {
        if (state.long_distance_0->_0 != state._0)
        {
            continue;
        }

        if (state.long_distance_0->_1 != state._1)
        {
            continue;
        }

        PLNNRC_COROUTINE_YIELD(state);
    }

    PLNNRC_COROUTINE_END();
}

struct p3_state
{
    int _0;
    int _1;
    int _2;
    int _3;
    airport_tuple* airport_0;
    airport_tuple* airport_1;
    int stage;
};

bool next(p3_state& state, worldstate& world)
{
    //PLNRC_PRINT("p3_state next, stage=%d\n", state.stage);

    PLNNRC_COROUTINE_BEGIN(state);

    for (state.airport_0 = world.airport; state.airport_0 != 0; state.airport_0 = state.airport_0->next)
    {
        if (state.airport_0->_0 != state._0)
        {
            continue;
        }

        state._1 = state.airport_0->_1;

        for (state.airport_1 = world.airport; state.airport_1 != 0; state.airport_1 = state.airport_1->next)
        {
            if (state.airport_1->_0 != state._2)
            {
                continue;
            }

            state._3 = state.airport_1->_1;

            PLNNRC_COROUTINE_YIELD(state);
        }
    }

    PLNNRC_COROUTINE_END();
}

// Primitive tasks

enum task_type
{
    task_none = 0,
    task_ride_taxi,
    task_fly,
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

// Runtime Structs

struct method_instance;
struct plannerstate;

typedef bool (*expand_func)(plannerstate&, worldstate& world);

struct method_instance
{
    expand_func expand;
    void* args;
    void* precondition;
    void* mrewind;
    void* trewind;
    method_instance* parent;
    bool expanded;
    int stage;
};

struct task_instance
{
    int            type;
    void*          args;
    task_instance* link;
};

struct plannerstate
{
    method_instance* top_method;
    task_instance* top_task;
    stack* mstack;
    stack* tstack;
};

method_instance* push_method(plannerstate& pstate, expand_func expand)
{
    method_instance* new_method = pstate.mstack->push<method_instance>();
    new_method->expand = expand;
    new_method->args = 0;
    new_method->parent = pstate.top_method;
    new_method->precondition = 0;
    new_method->trewind = 0;
    new_method->mrewind = 0;
    new_method->expanded = false;
    new_method->stage = 0;

    pstate.top_method = new_method;

    return new_method;
}

task_instance* push_task(plannerstate& pstate, int task_type)
{
    task_instance* new_task = pstate.tstack->push<task_instance>();
    new_task->type = task_type;
    new_task->args = 0;
    new_task->link = pstate.top_task;

    pstate.top_task = new_task;

    return new_task;
}

method_instance* rewind_top_method(plannerstate& pstate, bool rewind_tasks)
{
    method_instance* old_top = pstate.top_method;
    method_instance* new_top = old_top->parent;

    if (new_top)
    {
        pstate.mstack->rewind(new_top->mrewind);

        if (rewind_tasks && new_top->trewind < pstate.tstack->top())
        {
            // todo: align trewind to task_instance here
            task_instance* task = static_cast<task_instance*>(new_top->trewind);
            task_instance* top_task = task->link;

            pstate.tstack->rewind(new_top->trewind);

            pstate.top_task = top_task;
        }
    }

    pstate.top_method = new_top;

    return new_top;
}

bool next_branch(plannerstate& pstate, expand_func expand, worldstate& world)
{
    method_instance* method = pstate.top_method;

    if (expand)
    {
        method->stage = 0;
        method->expand = expand;
        return method->expand(pstate, world);
    }

    method->expand = 0;
    return false;
}

// Forwards

bool root_branch_0_expand(plannerstate&, worldstate& world);
bool travel_branch_0_expand(plannerstate&, worldstate& world);
bool travel_branch_1_expand(plannerstate&, worldstate& world);
bool travel_by_air_branch_0_expand(plannerstate&, worldstate& world);

// Method Expansions

bool root_branch_0_expand(plannerstate& pstate, worldstate& world)
{
    method_instance* method = pstate.top_method;
    p0_state* precondition = static_cast<p0_state*>(method->precondition);

    //PLNRC_PRINT("root_branch_0_expand, stage=%d\n", method->stage);

    PLNNRC_COROUTINE_BEGIN(*method);

    precondition = pstate.mstack->push<p0_state>();
    precondition->stage = 0;

    method->precondition = precondition;
    method->mrewind = pstate.mstack->top();
    method->trewind = pstate.tstack->top();

    while (next(*precondition, world))
    {
        {
            method_instance* m = push_method(pstate, travel_branch_0_expand);
            travel_args* a = pstate.mstack->push<travel_args>();
            a->_0 = precondition->_0;
            a->_1 = precondition->_1;
            m->args = a;
        }

        method->expanded = true;
        PLNNRC_COROUTINE_YIELD(*method);
    }

    return next_branch(pstate, 0, world);
    PLNNRC_COROUTINE_END();
}

bool travel_branch_0_expand(plannerstate& pstate, worldstate& world)
{
    method_instance* method = pstate.top_method;
    p1_state* precondition = static_cast<p1_state*>(method->precondition);
    travel_args* method_args = static_cast<travel_args*>(method->args);
    //PLNRC_PRINT("travel_branch_0_expand, stage=%d, 0=%d, 1=%d\n", method->stage, method_args->_0, method_args->_1);

    PLNNRC_COROUTINE_BEGIN(*method);

    precondition = pstate.mstack->push<p1_state>();
    precondition->_0 = method_args->_0;
    precondition->_1 = method_args->_1;
    precondition->stage = 0;

    method->precondition = precondition;
    method->mrewind = pstate.mstack->top();
    method->trewind = pstate.tstack->top();

    while (next(*precondition, world))
    {
        {
            task_instance* t = push_task(pstate, task_ride_taxi);
            ride_taxi_args* a = pstate.tstack->push<ride_taxi_args>();
            a->_0 = method_args->_0;
            a->_1 = method_args->_1;
            t->args = a;
        }

        method->expanded = true;
        PLNNRC_COROUTINE_YIELD(*method);
    }

    return next_branch(pstate, travel_branch_1_expand, world);
    PLNNRC_COROUTINE_END();
}

bool travel_branch_1_expand(plannerstate& pstate, worldstate& world)
{
    method_instance* method = pstate.top_method;
    p2_state* precondition = static_cast<p2_state*>(method->precondition);
    travel_args* method_args = static_cast<travel_args*>(method->args);
    //PLNRC_PRINT("travel_branch_1_expand, stage=%d, 0=%d, 1=%d\n", method->stage, method_args->_0, method_args->_1);

    PLNNRC_COROUTINE_BEGIN(*method);

    precondition = pstate.mstack->push<p2_state>();
    precondition->_0 = method_args->_0;
    precondition->_1 = method_args->_1;
    precondition->stage = 0;

    method->mrewind = pstate.mstack->top();
    method->trewind = pstate.tstack->top();

    while (next(*precondition, world))
    {
        {
            method_instance* m = push_method(pstate, travel_by_air_branch_0_expand);
            travel_by_air_args* a = pstate.mstack->push<travel_by_air_args>();
            a->_0 = method_args->_0;
            a->_1 = method_args->_1;
            m->args = a;
        }

        method->expanded = true;
        PLNNRC_COROUTINE_YIELD(*method);
    }

    return next_branch(pstate, 0, world);
    PLNNRC_COROUTINE_END();
}

// (:method (travel_by_air ?x ?y)
//     ((or (and (airport ?x ?ax) (airport ?y ?ay)) (and (start ?x) (finish ?y))))
//     ((travel ?x ?ax) (!fly ?ax ?ay) (travel ?y ?ay))
// )

bool travel_by_air_branch_0_expand(plannerstate& pstate, worldstate& world)
{
    method_instance* method = pstate.top_method;
    p3_state* precondition = static_cast<p3_state*>(method->precondition);
    travel_by_air_args* method_args = static_cast<travel_by_air_args*>(method->args);
    //PLNRC_PRINT("travel_by_air_branch_0_expand, stage=%d, 0=%d, 1=%d\n", method->stage, method_args->_0, method_args->_1);

    PLNNRC_COROUTINE_BEGIN(*method);

    precondition = pstate.mstack->push<p3_state>();
    precondition->_0 = method_args->_0;
    precondition->_2 = method_args->_1;
    precondition->stage = 0;

    method->precondition = precondition;
    method->mrewind = pstate.mstack->top();
    method->trewind = pstate.tstack->top();

    while (next(*precondition, world))
    {
        {
            method_instance* m = push_method(pstate, travel_branch_0_expand);
            travel_args* a = pstate.mstack->push<travel_args>();
            a->_0 = method_args->_0;
            a->_1 = precondition->_1;
            m->args = a;
        }

        PLNNRC_COROUTINE_YIELD(*method);

        {
            task_instance* t = push_task(pstate, task_fly);
            fly_args* a = pstate.tstack->push<fly_args>();
            a->_0 = precondition->_1;
            a->_1 = precondition->_3;
            t->args = a;
        }

        {
            method_instance* m = push_method(pstate, travel_branch_0_expand);
            travel_args* a = pstate.mstack->push<travel_args>();
            a->_0 = precondition->_3;
            a->_1 = method_args->_1;
            m->args = a;
        }

        method->expanded = true;
        PLNNRC_COROUTINE_YIELD(*method);
    }

    return next_branch(pstate, 0, world);
    PLNNRC_COROUTINE_END();
}

bool find_plan(plannerstate& pstate, worldstate& world)
{
    push_method(pstate, root_branch_0_expand);

    while (pstate.top_method)
    {
        method_instance* method = pstate.top_method;

        // if found satisfying preconditions
        if (method->expand(pstate, world))
        {
            // if expanded to primitive tasks
            if (method == pstate.top_method)
            {
                while (method && method->expanded)
                {
                    method = rewind_top_method(pstate, false);
                }

                if (!method)
                {
                    return true;
                }
            }
        }
        // backtrack otherwise
        else
        {
            rewind_top_method(pstate, true);
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
    printf("!ride_taxi(%d, %d)\n", args->_0, args->_1);
}

void print_task_fly(const task_instance* task)
{
    fly_args* args = static_cast<fly_args*>(task->args);
    printf("!fly(%d, %d)\n", args->_0, args->_1);
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
    stack tstack(32768);

    auto timestamp = std::chrono::high_resolution_clock::now();

    for (int i=0; i<1000000; ++i)
    {
        plannerstate pstate;
        pstate.top_method = 0;
        pstate.top_task = 0;
        pstate.mstack = &mstack;
        pstate.tstack = &tstack;

        find_plan(pstate, world);

        mstack.reset();
        tstack.reset();
    }

    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - timestamp);
    std::cout << "elapsed: " << elapsed.count() / 1000.f << " s" << std::endl;

    plannerstate pstate;
    pstate.top_method = 0;
    pstate.top_task = 0;
    pstate.mstack = &mstack;
    pstate.tstack = &tstack;

    bool result = find_plan(pstate, world);

    if (result)
    {
        task_instance* task = reverse_task_list(pstate.top_task);

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
