#include "ticker.h"

void ticker_dsp(t_ticker *x, t_signal **sp)
{
  dsp_add(ticker_perform, 1, x);
}
t_int* ticker_perform(t_int *w)
{ 
    t_ticker *x = (t_ticker *)(w[1]);
    if(x->tick_state == 1){
        x->tick_len += 1.0f;
    }
    else if(x->tick_state == 2){
        x->tick_pos += 1.0f;
        if(x->tick_pos >= x->tick_start+x->tick_len){
            x->tick_start = x->tick_pos;
            outlet_float(x->o_tick_sync, x->tick_pos);
        }
        int ll = int(x->tick_len);
        ll >>= x->tick_div;
        int pulse = int(x->tick_pos) % ll;
        if(pulse == 0) outlet_bang(x->o_tick_pulse);
    }

  return (w+2);
}

void ticker_oninfo(t_ticker* x){
    int tl = int(x->tick_len);
    float p = x->tick_len ? float(int(x->tick_pos)%tl) / float(tl) : 0.0f;
    post("+++++++ticker++++++");
    post("tick len: %f, tick %f, tickPer: %f", x->tick_len, x->tick_pos, p);
    post("state %f", x->tick_state);
}

void ticker_onset(t_ticker* x, t_floatarg t){
    if(x->tick_len > 0) return;
    if(t <= 0) return;
    
    x->tick_len = t;
    x->tick_pos = -1;
    post("ticker set tick len %f", t);
}

void ticker_onstart(t_ticker* x){
    if(x->tick_state == 0 && x->tick_len == 0.0f)
        x->tick_state = 1;
    else if(x->tick_len){
        x->tick_pos = -1.0f;
        x->tick_start = 0.0f;
        x->tick_state = 2;
        outlet_float(x->o_tick_sync, x->tick_pos);
    }
}

void ticker_onend(t_ticker* x){
    if(x->tick_state != 0){
        x->tick_state = 0;
    }
}

void ticker_onreset(t_ticker* x){
    x->tick_state = 0;
    x->tick_pos = 0.0f;
    x->tick_len = 0.0f;
    x->tick_start = 0.0f;
    post("sync len: %f",x->tick_len);
}

void ticker_ondiv(t_ticker* x, t_floatarg t){
    x->tick_div = t;
}

void* ticker_new(){
 
    t_ticker* x = (t_ticker*)pd_new(ticker_class);
 
    x->o_tick_sync = (t_outlet*)outlet_new(&x->x_obj, &s_float);
    x->o_tick_pulse = (t_outlet*)outlet_new(&x->x_obj, &s_bang);

    x->tick_count = 0;
    x->tick_state = 0;
    x->tick_pos = 0.0f;
    x->tick_len = 0.0f;
    x->tick_div = 2;
 
    return (void*)x;
}

void ticker_free(t_ticker* x){
    outlet_free(x->o_tick_sync);
    outlet_free(x->o_tick_pulse);
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
        (t_atomtype)0
    );
    class_addmethod(ticker_class,
        (t_method)ticker_dsp, gensym("dsp"), A_CANT, 0);
    class_addfloat(ticker_class,(t_method)ticker_onset);
    class_addmethod(ticker_class,(t_method)ticker_oninfo, gensym("debug_info"),  (t_atomtype)0);
    class_addmethod(ticker_class,(t_method)ticker_onstart, gensym("start"),  (t_atomtype)0);
    class_addmethod(ticker_class,(t_method)ticker_onend,   gensym("stop"),   (t_atomtype)0);
    class_addmethod(ticker_class,(t_method)ticker_onreset, gensym("reset"),  (t_atomtype)0);
    class_addmethod(ticker_class,(t_method)ticker_ondiv, gensym("div"), A_DEFFLOAT, (t_atomtype)0);
}
