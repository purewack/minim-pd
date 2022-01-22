#pragma once
#include <string>

// void cpppost();
// std::string complexcpp(std::string s);


extern "C"{
    #include "m_pd.h"
    static t_class *inter_class;
    typedef struct _inter{
        t_object    x_obj;
        // t_float     f;
        t_outlet*   o_tick_sync;
        t_outlet*   o_tick_stats;
        // t_outlet**  o_outs;
        int         tick_count;
        int         tick_state;
        t_float     tick_len;
        t_float     tick_pos;
        t_float     tick_start;
    } t_inter;

    void* inter_new(t_floatarg f);
    void inter_free(t_inter* x);
    void inter_setup(void);
    void inter_onNewPointer(t_inter* x, );
}