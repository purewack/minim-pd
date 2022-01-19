#include "m_pd.h"
#include "plonk~.h"
#include <math.h>

void* plonk_tilde_new(t_floatarg freq){
    t_plonk_tilde* x = (t_plonk_tilde*)pd_new(plonk_tilde_class);
    x->o_f = outlet_new(&x->x_obj,&s_signal);
    x->plonk = 0;
    x->sin_f = freq > 0.f ? freq : 1000.f; 
    return (void*)x;
}
void plonk_tilde_free(t_plonk_tilde* x){
    outlet_free(x->o_f);
}
void plonk_tilde_dsp(t_plonk_tilde *x, t_signal **sp)
{
  dsp_add(plonk_tilde_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);
}
t_int* plonk_tilde_perform(t_int *w)
{
  t_plonk_tilde *x = (t_plonk_tilde *)(w[1]);
  t_sample    *out =      (t_sample *)(w[2]);
  int            n =             (int)(w[3]);

  if(x->plonk != 0.f){
    while (n--) {
        *out++ = sin(x->sin_phase * 2.0f*3.1415f) * x->plonk;
        x->sin_phase += x->sin_f / 48000.f;
        if(x->sin_phase > 1.0f) x->sin_phase -= 1.0f;
        x->plonk -= x->unplonk;
        if(x->plonk < 0.0f) x->plonk = 0.f;
    }
  }

  return (w+4);
}

void plonk_tilde_onSyncBang(t_plonk_tilde *x){
   x->plonk = 1.0f;
   x->unplonk = 1.f /1000.f;
   x->sin_phase = 0;
}

void plonk_tilde_setup(void){
    plonk_tilde_class = class_new(
        gensym("plonk~"),
        (t_newmethod)plonk_tilde_new,
        (t_method)plonk_tilde_free,
        sizeof(t_plonk_tilde),
        CLASS_DEFAULT,
        A_DEFFLOAT,
        (t_atomtype)0
    );
    class_addmethod(plonk_tilde_class,
        (t_method)plonk_tilde_dsp, gensym("dsp"), A_CANT, 0);
    CLASS_MAINSIGNALIN(plonk_tilde_class, t_plonk_tilde, f);
    class_addbang(plonk_tilde_class, (t_method)plonk_tilde_onSyncBang);
}

