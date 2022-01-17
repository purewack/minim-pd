#pragma once
#include <string>

// void cpppost();
// std::string complexcpp(std::string s);


extern "C"{
    #include "m_pd.h"
    static t_class *bank_class;
    typedef struct _bank{
        t_object    x_obj;
        t_float     f;
        t_outlet*   o_active_motif;
        t_outlet*   o_active_motif_state;
        int         active_motif_state;
        int         active_motif;
        t_float     tick_when_action;
    } t_bank;

    void* bank_new(t_floatarg f);
    void bank_free(t_bank* x);
    void bank_setup(void);
    void bank_dsp(t_bank *x, t_signal **sp);
    t_int* bank_perform(t_int *w);
}