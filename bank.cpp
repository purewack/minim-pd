#include "bank.h"

void bank_dsp(t_bank *x, t_signal **sp)
{
  dsp_add(bank_perform, 1, x);
}
t_int* bank_perform(t_int *w)
{ 
    t_bank *x = (t_bank *)(w[1]);
 
//   t_sample    *in  =      (t_sample *)(w[2]);
//   t_sample    *out =      (t_sample *)(w[3]);
//   int            n =             (int)(w[4]);
//   while (n--) *out++ = (*in++)*(x->phase);
  return (w+2);
}

void* bank_new(t_floatarg out_count){
    t_bank* x = (t_bank*)pd_new(bank_class);

    return (void*)x;
}

void bank_free(t_bank* x){
    outlet_free(x->o_tick_sync);

}

void bank_setup(void){
    bank_class = class_new(
        gensym("bank"),
        (t_newmethod)bank_new,
        (t_method)bank_free,
        sizeof(t_bank),
        CLASS_DEFAULT,
        A_DEFFLOAT,
        (t_atomtype)0
    );
    class_addmethod(bank_class,
        (t_method)bank_dsp, gensym("dsp"), A_CANT, 0);
    
}
