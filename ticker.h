#pragma once
#include <string>

// void cpppost();
// std::string complexcpp(std::string s);


extern "C"{
    #include "m_pd.h"
    static t_class *ticker_class;
    typedef struct _ticker{
        t_object    x_obj;
        t_float     f;
        t_outlet*   o_tick_sync;
        t_outlet**  o_outs;
        int         o_count;
        t_float     tick_len;
        t_float     tick_pos;
        t_float     tick_mode;
    } t_ticker;

    void* ticker_new(t_floatarg f);
    void ticker_free(t_ticker* x);
    void ticker_setup(void);
    void ticker_onstart(t_ticker* x);
    void ticker_onend(t_ticker* x);
    void ticker_onreset(t_ticker* x);
    void ticker_dsp(t_ticker *x, t_signal **sp);
    t_int* ticker_perform(t_int *w);
}