#include <stdio.h>

#define PLNNRC_COROUTINE_BEGIN(state) switch (state.stage) { case 0:
#define PLNNRC_COROUTINE_YIELD(state) do { state.stage = __LINE__; return true; case __LINE__:; } while (0)
#define PLNNRC_COROUTINE_END() } return false

struct coroutine_state
{
    int range;
    int i;
    int stage;
};

void init(coroutine_state& state, int range)
{
    state.stage = 0;
    state.range = range;
}

bool next(coroutine_state& state)
{
    PLNNRC_COROUTINE_BEGIN(state);

    for (state.i = 0; state.i < state.range; ++state.i)
    {
        PLNNRC_COROUTINE_YIELD(state);
    }

    PLNNRC_COROUTINE_END();
}

int main()
{
    coroutine_state state;
    init(state, 20);

    while (next(state))
    {
        printf("i = %d\n", state.i);
    }

    return 0;
}
