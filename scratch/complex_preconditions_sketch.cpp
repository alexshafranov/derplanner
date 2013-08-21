
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
};

void init(p1_state& state, const worldstate& world)
{
    state.ax = world.ax;
    state.ay = world.ay;
}

void next(p1_state& state)
{
    for (; state.ax != 0; state.ax = state.ax->next)
    {
        if (state.ax->_1 == state.in_u)
        {
            state.out_t = state.ax->_0;

            for (; state.ay != 0; state.ay = state.ay->next)
            {
                if (state.ay->_0 == state.out_t && state.ay->_1 == state.in_v)
                {
                    return true;
                }
            }
        }
    }

    return false;
}
