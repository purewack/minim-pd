#include "m_pd.h"

static t_class *one_class;
typedef struct _one{
    t_object    x_obj;
    t_outlet*   o_out;
} t_one;
void one_bang(t_one *x);
void* one_new(void);
void one_free(t_one* x);
void one_setup(void);

static t_class *oscc_tilde_class;
typedef struct _oscc_tilde{
    t_object    x_obj;
    t_float     phase;
    t_float     f;
    t_inlet*    i_phase;
    t_outlet*   o_out;
} t_oscc_tilde;
void* oscc_tilde_new(void);
void oscc_tilde_free(t_oscc_tilde* x);
void oscc_tilde_setup(void);
void oscc_tilde_dsp(t_oscc_tilde *x, t_signal **sp);
t_int* oscc_tilde_perform(t_int *w);
void oscc_tilde_onPhaseBang(t_oscc_tilde *x);

