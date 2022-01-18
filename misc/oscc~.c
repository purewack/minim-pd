#include "m_pd.h"
#include "motif.h"
#include <math.h>

void* oscc_tilde_new(void){
    t_oscc_tilde* x = (t_oscc_tilde*)pd_new(oscc_tilde_class);
    x->i_phase = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_bang, &s_bang);
    x->o_out = outlet_new(&x->x_obj,&s_signal);
    x->phase = 1.0f;
    return (void*)x;
}
void oscc_tilde_free(t_oscc_tilde* x){
    inlet_free(x->i_phase);
    outlet_free(x->o_out);
}
void oscc_tilde_dsp(t_oscc_tilde *x, t_signal **sp)
{
  dsp_add(oscc_tilde_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}
t_int* oscc_tilde_perform(t_int *w)
{
  t_oscc_tilde *x = (t_oscc_tilde *)(w[1]);
  t_sample    *in  =      (t_sample *)(w[2]);
  t_sample    *out =      (t_sample *)(w[3]);
  int            n =             (int)(w[4]);

  while (n--) *out++ = (*in++)*(x->phase);
  return (w+5);
}

void oscc_tilde_onPhaseBang(t_oscc_tilde *x){
   x->phase = x->phase != 1.0f ? 1.0f : -1.0f; 
   post("%f",x->phase);
}

void oscc_tilde_setup(void){
    oscc_tilde_class = class_new(
        gensym("oscc~"),
        (t_newmethod)oscc_tilde_new,
        (t_method)oscc_tilde_free,
        sizeof(t_oscc_tilde),
        CLASS_DEFAULT,
        A_DEFFLOAT,
        0
    );
    class_addmethod(oscc_tilde_class,
        (t_method)oscc_tilde_dsp, gensym("dsp"), A_CANT, 0);
    CLASS_MAINSIGNALIN(oscc_tilde_class, t_oscc_tilde, f);
    class_addbang(oscc_tilde_class, (t_method)oscc_tilde_onPhaseBang);
}

