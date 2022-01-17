#include "ticker.h"

void ticker_dsp(t_ticker *x, t_signal **sp)
{
  dsp_add(ticker_perform, 1, x);
}
t_int* ticker_perform(t_int *w)
{ 
    t_ticker *x = (t_ticker *)(w[1]);
    if(x->tick_mode == 1){
        x->tick_len += 1.0f;
    }
    else if(x->tick_mode == 2){
        x->tick_pos += 1.0f;
        if(x->tick_pos >= x->tick_len){
            x->tick_pos = 0.0f;
            
            outlet_bang(x->o_tick_sync);
            // for(int i=0; i<x->o_count; i++)
            //     outlet_bang(x->o_outs[i]);
        }
    }
//   t_sample    *in  =      (t_sample *)(w[2]);
//   t_sample    *out =      (t_sample *)(w[3]);
//   int            n =             (int)(w[4]);
//   while (n--) *out++ = (*in++)*(x->phase);
  return (w+2);
}


void ticker_onstart(t_ticker* x){
    if(x->tick_mode == 0 && x->tick_len == 0.0f)
        x->tick_mode = 1;
    else if(x->tick_len){
        x->tick_pos = 0.0f;
        x->tick_mode = 2;
    }
}

void ticker_onend(t_ticker* x){
    if(x->tick_mode != 0){
        x->tick_mode = 0;
    }
}

void ticker_onreset(t_ticker* x){
    x->tick_mode = 0;
    x->tick_pos = 0.0f;
    x->tick_len = 0.0f;
}

void* ticker_new(t_floatarg out_count){
 
    t_ticker* x = (t_ticker*)pd_new(ticker_class);
 
    x->o_tick_sync = (t_outlet*)outlet_new(&x->x_obj, &s_bang);
    // x->o_count = out_count > 0 ? (int)out_count : 1;
    // x->o_outs = (t_outlet**)malloc(x->o_count * sizeof(t_outlet*));
    // for(int i=0; i<x->o_count; i++) 
    //     x->o_outs[i] = (t_outlet*)outlet_new(&x->x_obj, &s_bang);
    
    x->tick_mode = 0;
    x->tick_pos = 0.0f;
    x->tick_len = 0.0f;
 
    return (void*)x;
}

void ticker_free(t_ticker* x){
    outlet_free(x->o_tick_sync);
    // for(int i=0; i<x->o_count; i++) 
    //     outlet_free(x->o_outs[i]);
}

void ticker_setup(void){
    ticker_class = class_new(
        gensym("ticker"),
        (t_newmethod)ticker_new,
        (t_method)ticker_free,
        sizeof(t_ticker),
        CLASS_DEFAULT,
        A_DEFFLOAT,
        (t_atomtype)0
    );
    class_addmethod(ticker_class,
        (t_method)ticker_dsp, gensym("dsp"), A_CANT, 0);
    class_addmethod(ticker_class,(t_method)ticker_onstart,gensym("start"),(t_atomtype)0);
    class_addmethod(ticker_class,(t_method)ticker_onend,gensym("stop"),(t_atomtype)0);
    class_addmethod(ticker_class,(t_method)ticker_onreset,gensym("reset"),(t_atomtype)0);
}
