#include <stdio.h>
#include <stddef.h> // size_t

// for test
#include <iostream>
#include <chrono>

#define PLNNRC_COROUTINE_BEGIN(state) switch ((state).stage) { case 0:
#define PLNNRC_COROUTINE_YIELD(state) do { (state).stage = __LINE__; return true; case __LINE__:; } while (0)
#define PLNNRC_COROUTINE_END() } return false

// Stack

class stack
{
public:
    stack(size_t capacity)
        : _capacity(capacity)
    {
        _buffer = new char[_capacity];
        _object = _buffer;
        _top = _buffer;
    }

    ~stack()
    {
        delete [] _buffer;
    }

    template <typename T>
    T* begin_object()
    {
        _object = _top;
        _top += sizeof(T);
        return reinterpret_cast<T*>(_object);
    }

    template <typename T>
    T* push()
    {
        char* top = _top;
        _top += sizeof(T);
        return reinterpret_cast<T*>(top);
    }

    template <typename T>
    T* top()
    {
        if (_top > _object)
        {
            return reinterpret_cast<T*>(_object);
        }

        return 0;
    }

    void* top_()
    {
        return _top;
    }

    template <typename T>
    void rewind(void* obj, void* top)
    {
        if (obj)
        {
            _object = reinterpret_cast<char*>(obj);
            _top = reinterpret_cast<char*>(top);
        }
        else
        {
            _object = _buffer;
            _top = _buffer;
        }
    }

private:
    size_t _capacity;
    char* _buffer;
    char* _top;
    char* _object;
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

typedef bool (*expand_func)(method_instance*, stack&, stack&, worldstate& world);

struct branch_state
{
    void* precondition;
    int stage;
};

struct method_instance
{
    expand_func expand;
    void* args;
    branch_state branch;

    void* trewind_obj;
    void* trewind_top;

    void* mrewind_obj;
    void* mrewind_top;

    bool expanded;

    int stage;
};

struct task_instance
{
    int            type;
    void*          args;
    task_instance* link;
};

// Forwards

bool root_expand(method_instance* instance, stack& mstack, stack& tstack, worldstate& world);
bool root_branch_0_expand(method_instance* method, stack& mstack, stack& tstack, worldstate& world);
bool travel_expand(method_instance* instance, stack& mstack, stack& tstack, worldstate& world);
bool travel_branch_0_expand(method_instance* method, stack& mstack, stack& tstack, worldstate& world);
bool travel_branch_1_expand(method_instance* method, stack& mstack, stack& tstack, worldstate& world);
bool travel_by_air_expand(method_instance* instance, stack& mstack, stack& tstack, worldstate& world);
bool travel_by_air_branch_0_expand(method_instance* instance, stack& mstack, stack& tstack, worldstate& world);

// Method Expansions

bool root_expand(method_instance* instance, stack& mstack, stack& tstack, worldstate& world)
{
    printf("root_expand, stage=%d\n", instance->stage);

    PLNNRC_COROUTINE_BEGIN(*instance);

    instance->trewind_obj = tstack.top<task_instance>();
    instance->trewind_top = tstack.top_();

    {
        p0_state* pstate = mstack.push<p0_state>();
        pstate->stage = 0;

        instance->branch.precondition = pstate;
        instance->branch.stage = 0;
    }

    while (root_branch_0_expand(instance, mstack, tstack, world))
    {
        PLNNRC_COROUTINE_YIELD(*instance);
    }

    PLNNRC_COROUTINE_END();
}

bool root_branch_0_expand(method_instance* method, stack& mstack, stack& tstack, worldstate& world)
{
    printf("root_branch_0_expand, stage=%d\n", method->branch.stage);

    p0_state* precondition = static_cast<p0_state*>(method->branch.precondition);

    PLNNRC_COROUTINE_BEGIN(method->branch);

    while (next(*precondition, world))
    {
        {
            method_instance* prev = mstack.top<method_instance>();
            void* mrewind_top = mstack.top_();
            method_instance* m0 = mstack.begin_object<method_instance>();
            travel_args* a0 = mstack.push<travel_args>();
            m0->args = a0;
            m0->expand = travel_expand;
            m0->stage = 0;
            m0->expanded = false;
            m0->mrewind_obj = prev;
            m0->mrewind_top = mrewind_top;
            a0->_0 = precondition->_0;
            a0->_1 = precondition->_1;
        }

        method->expanded = true;

        PLNNRC_COROUTINE_YIELD(method->branch);
    }

    PLNNRC_COROUTINE_END();
}

bool travel_expand(method_instance* instance, stack& mstack, stack& tstack, worldstate& world)
{
    travel_args* args = static_cast<travel_args*>(instance->args);

    printf("travel_expand, stage=%d, 0=%d 1=%d\n", instance->stage, args->_0, args->_1);

    PLNNRC_COROUTINE_BEGIN(*instance);

    instance->trewind_obj = tstack.top<task_instance>();
    instance->trewind_top = tstack.top_();

    {
        p1_state* pstate = mstack.push<p1_state>();
        pstate->_0 = args->_0;
        pstate->_1 = args->_1;
        pstate->stage = 0;

        instance->branch.precondition = pstate;
        instance->branch.stage = 0;
    }

    while (travel_branch_0_expand(instance, mstack, tstack, world))
    {
        PLNNRC_COROUTINE_YIELD(*instance);
    }

    {
        p2_state* pstate = mstack.push<p2_state>();
        pstate->_0 = args->_0;
        pstate->_1 = args->_1;
        pstate->stage = 0;

        instance->branch.precondition = pstate;
        instance->branch.stage = 0;
    }

    while (travel_branch_1_expand(instance, mstack, tstack, world))
    {
        PLNNRC_COROUTINE_YIELD(*instance);
    }

    PLNNRC_COROUTINE_END();
}

bool travel_branch_0_expand(method_instance* method, stack& mstack, stack& tstack, worldstate& world)
{
    travel_args* method_args = static_cast<travel_args*>(method->args);
    printf("travel_branch_0_expand, stage=%d, 0=%d, 1=%d\n", method->branch.stage, method_args->_0, method_args->_1);
    p1_state* pstate = static_cast<p1_state*>(method->branch.precondition);

    PLNNRC_COROUTINE_BEGIN(method->branch);

    while (next(*pstate, world))
    {
        {
            task_instance* prev = tstack.top<task_instance>();
            task_instance* task = tstack.begin_object<task_instance>();
            ride_taxi_args* args = tstack.push<ride_taxi_args>();
            args->_0 = method_args->_0;
            args->_1 = method_args->_1;
            task->args = args;
            task->link = prev;
            task->type = task_ride_taxi;
        }

        method->expanded = true;

        PLNNRC_COROUTINE_YIELD(method->branch);
    }

    PLNNRC_COROUTINE_END();
}

bool travel_branch_1_expand(method_instance* method, stack& mstack, stack& tstack, worldstate& world)
{
    travel_args* method_args = static_cast<travel_args*>(method->args);
    printf("travel_branch_1_expand, stage=%d, 0=%d, 1=%d\n", method->branch.stage, method_args->_0, method_args->_1);
    p2_state* pstate = static_cast<p2_state*>(method->branch.precondition);

    PLNNRC_COROUTINE_BEGIN(method->branch);

    while (next(*pstate, world))
    {
        {
            method_instance* prev = mstack.top<method_instance>();
            void* mrewind_top = mstack.top_();
            method_instance* m = mstack.begin_object<method_instance>();
            travel_by_air_args* args = mstack.push<travel_by_air_args>();
            m->expand = travel_by_air_expand;
            m->args = args;
            m->expanded = false;
            m->stage = 0;
            m->mrewind_obj = prev;
            m->mrewind_top = mrewind_top;
            args->_0 = method_args->_0;
            args->_1 = method_args->_1;
        }

        method->expanded = true;

        PLNNRC_COROUTINE_YIELD(method->branch);
    }

    PLNNRC_COROUTINE_END();
}

// (:method (travel_by_air ?x ?y)
//     ((or (and (airport ?x ?ax) (airport ?y ?ay)) (and (start ?x) (finish ?y))))
//     ((travel ?x ?ax) (!fly ?ax ?ay) (travel ?y ?ay))
// )

bool travel_by_air_expand(method_instance* instance, stack& mstack, stack& tstack, worldstate& world)
{
    travel_by_air_args* args = static_cast<travel_by_air_args*>(instance->args);
    printf("travel_by_air_expand, stage=%d, 0=%d, 1=%d\n", instance->stage, args->_0, args->_1);

    PLNNRC_COROUTINE_BEGIN(*instance);

    instance->trewind_obj = tstack.top<task_instance>();
    instance->trewind_top = tstack.top_();

    {
        p3_state* pstate = mstack.push<p3_state>();
        pstate->_0 = args->_0;
        pstate->_2 = args->_1;
        pstate->stage = 0;

        instance->branch.precondition = pstate;
        instance->branch.stage = 0;
    }

    while (travel_by_air_branch_0_expand(instance, mstack, tstack, world))
    {
        PLNNRC_COROUTINE_YIELD(*instance);
    }

    PLNNRC_COROUTINE_END();
}

bool travel_by_air_branch_0_expand(method_instance* method, stack& mstack, stack& tstack, worldstate& world)
{
    travel_by_air_args* method_args = static_cast<travel_by_air_args*>(method->args);
    printf("travel_by_air_branch_0_expand, stage=%d, 0=%d, 1=%d\n", method->branch.stage, method_args->_0, method_args->_1);
    p3_state* pstate = static_cast<p3_state*>(method->branch.precondition);

    PLNNRC_COROUTINE_BEGIN(method->branch);

    while (next(*pstate, world))
    {
        {
            method_instance* prev = mstack.top<method_instance>();
            void* mrewind_top = mstack.top_();
            method_instance* m = mstack.begin_object<method_instance>();
            travel_args* args = mstack.push<travel_args>();
            m->expand = travel_expand;
            m->args = args;
            m->expanded = false;
            m->stage = 0;
            m->mrewind_obj = prev;
            m->mrewind_top = mrewind_top;
            args->_0 = method_args->_0;
            args->_1 = pstate->_1;
        }

        PLNNRC_COROUTINE_YIELD(method->branch);

        {
            task_instance* prev = tstack.top<task_instance>();
            task_instance* task = tstack.begin_object<task_instance>();
            fly_args* args = tstack.push<fly_args>();
            task->type = task_fly;
            task->args = args;
            task->link = prev;
            args->_0 = pstate->_1;
            args->_1 = pstate->_3;
        }

        {
            method_instance* prev = mstack.top<method_instance>();
            void* mrewind_top = mstack.top_();
            method_instance* m = mstack.begin_object<method_instance>();
            travel_args* args = mstack.push<travel_args>();
            m->expand = travel_expand;
            m->args = args;
            m->expanded = false;
            m->stage = 0;
            m->mrewind_obj = prev;
            m->mrewind_top = mrewind_top;
            args->_0 = pstate->_3;
            args->_1 = method_args->_1;
        }

        method->expanded = true;

        PLNNRC_COROUTINE_YIELD(method->branch);
    }

    PLNNRC_COROUTINE_END();
}

bool find_plan(worldstate& world, stack& mstack, stack& tstack)
{
    method_instance* root = mstack.begin_object<method_instance>();
    root->expand = root_expand;
    root->mrewind_obj = 0;
    root->mrewind_top = 0;
    root->trewind_obj = 0;
    root->trewind_top = 0;
    root->expanded = false;
    root->stage = 0;

    while (mstack.top<method_instance>())
    {
        method_instance* method = mstack.top<method_instance>();

        if (method->expand(method, mstack, tstack, world))
        {
            printf("->expanded\n");

            if (mstack.top<method_instance>() == method)
            {
                printf("->top\n");

                while (method != root && method->expanded)
                {
                    mstack.rewind<method_instance>(method->mrewind_obj, method->mrewind_top);
                    method = mstack.top<method_instance>();
                }

                if (method == root)
                {
                    printf("Done true.\n");
                    return true;
                }
            }
        }
        else
        {
            printf("->backtrack!\n");

            tstack.rewind<task_instance>(method->trewind_obj, method->trewind_top);
            mstack.rewind<method_instance>(method->mrewind_obj, method->mrewind_top);

            if (mstack.top<method_instance>() == root)
            {
                printf("Done false.\n");
                return false;
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
    stack pstack(32768);

    auto timestamp = std::chrono::high_resolution_clock::now();

    // for (int i=0; i<1000000; ++i)
    // {
    //     find_plan(world, mstack, pstack);

    //     mstack.rewind<method_instance>(0, 0);
    //     pstack.rewind<task_instance>(0, 0);
    // }

    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - timestamp);
    std::cout << "elapsed: " << elapsed.count() / 1000.f << " s" << std::endl;

    bool result = find_plan(world, mstack, pstack);

    if (result)
    {
        task_instance* task = reverse_task_list(pstack.top<task_instance>());

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
