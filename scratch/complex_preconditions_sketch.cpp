#include <stdio.h>

#define PLNNRC_COROUTINE_BEGIN(state) switch (state.stage) { case 0:
#define PLNNRC_COROUTINE_YIELD(state) do { state.stage = __LINE__; return true; case __LINE__:; } while (0)
#define PLNNRC_COROUTINE_END() } return false

struct ax_tuple
{
    int _0;
    int _1;
    ax_tuple* next;
};

struct ay_tuple
{
    int _0;
    int _1;
    ay_tuple* next;
};

struct worldstate
{
    ax_tuple* ax;
    ay_tuple* ay;
};

// (:method (m1 ?u ?v)
//     ((ax ?t ?u) (ay ?t ?v))
//     ((m2 ?t ?t))
// )

struct p1_state
{
    int in_u;
    int in_v;

    int out_t;

    ax_tuple* ax;
    ay_tuple* ay;

    int stage;
};

bool next(p1_state& state, worldstate& world)
{
    PLNNRC_COROUTINE_BEGIN(state);

    for (state.ax = world.ax; state.ax != 0; state.ax = state.ax->next)
    {
        if (state.ax->_1 != state.in_u)
        {
            continue;
        }

        for (state.ay = world.ay; state.ay != 0; state.ay = state.ay->next)
        {
            if (state.ay->_0 == state.ax->_0 && state.ay->_1 == state.in_v)
            {
                continue;
            }

            state.out_t = state.ax->_0;
            PLNNRC_COROUTINE_YIELD(state);
        }
    }

    PLNNRC_COROUTINE_END();
}

// (:method (m1 ?u ?v)
//     ((ax ?t ?u) (ay ?t ?v))
//     ((m2 ?t ?t))
// )

int main()
{
    worldstate world;

    world.ax = new ax_tuple[2];
    world.ay = new ay_tuple[2];

    {
        ax_tuple* ax = world.ax;
        ax->_0 = 10;
        ax->_1 = 15;
        ax->next = ax + 1;
        ax++;

        ax->_0 = 25;
        ax->_1 = 20;
        ax->next = 0;
    }

    {
        ay_tuple* ay = world.ay;
        ay->_0 = 10;
        ay->_1 = 17;
        ay->next = ay + 1;
        ay++;

        ay->_0 = 25;
        ay->_1 = 30;
        ay->next = 0;
        ay++;
    }

    p1_state state = {0};
    state.in_u = 20;
    state.in_v = 30;

    while (next(state, world))
    {
        printf("t = %d\n", state.out_t);
    }

    return 0;
}
