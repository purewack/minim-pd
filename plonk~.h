#pragma once
#include <string>
#include "m_pd.h"
extern "C"{
   
    static t_class *plonk_tilde_class;
    typedef struct _plonk_tilde{
        t_object    x_obj;
        t_float     f;
        t_outlet*   o_f;
        t_float     plonk;
        t_float     unplonk;
        t_float     sin_phase;
        t_float     sin_f;
    } t_plonk_tilde;

    void plonk_tilde_onSyncBang(t_plonk_tilde *x);
    void* plonk_tilde_new(t_floatarg f);
    void plonk_tilde_free(t_plonk_tilde* x);
    void plonk_tilde_setup(void);
    void plonk_tilde_dsp(t_plonk_tilde *x, t_signal **sp);
    t_int* plonk_tilde_perform(t_int *w);
}