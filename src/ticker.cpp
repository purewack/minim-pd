#include <string>

extern "C"{
    #include "m_pd.h"
    static t_class *ticker_class;
    typedef struct _ticker{
        t_object    x_obj;
        // t_float     f;
        t_outlet*   o_tick_ratio;
        t_outlet**   o_tick_sync;
        // t_outlet**  o_outs;
        int         tick_count;
        int         tick_state;
        int         tick_div;
        t_float     tick_len;
        t_float     tick_pos;
        t_float     tick_ppos;
        t_float     tick_ratio;
        t_float     tick_start;
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
        x->tick_ppos += 1.f;
        x->tick_pos += 1.f;
        x->tick_ratio = (x->tick_ppos / x->tick_len);
        if(x->tick_ratio > 1.f) x->tick_ratio -= 1.f;
        outlet_float(x->o_tick_ratio, x->tick_ratio);
        if(x->tick_pos >= x->tick_start+x->tick_len){
            x->tick_ppos = 0;
            x->tick_start = x->tick_pos;
        }
        int tt = int(x->tick_ppos);
        int ll = int(x->tick_len);
        if(tt == (ll*0)/4) outlet_bang(x->o_tick_sync[0]);
        if(tt == (ll*1)/4) outlet_bang(x->o_tick_sync[1]);
        if(tt == (ll*2)/4) outlet_bang(x->o_tick_sync[2]);
        if(tt == (ll*3)/4) outlet_bang(x->o_tick_sync[3]);
    }

  return (w+2);
}

void ticker_oninfo(t_ticker* x){
    post("+++++++ticker++++++");
    post("ppos:%f tick len: %f, tick %f, pos:%f",x->tick_ratio, x->tick_len, x->tick_pos, x->tick_ppos);
    post("state %f", x->tick_state);
}


void ticker_oninfoShort(t_ticker* x){
    post("++++Ticker++++");
    post("ppos:%f / len : %f", x->tick_ppos, x->tick_len);
}


void ticker_onset(t_ticker* x, t_floatarg t){
    if(x->tick_len > 0) return;
    if(t <= 0) return;
    
    x->tick_len = t;
    x->tick_pos = 0;
    x->tick_ratio = 0.f;
    x->tick_ppos = 0.f;
    post("ticker set tick len %f", t);
}

void ticker_onstart(t_ticker* x){
    if(x->tick_state == 0 && x->tick_len == 0.0f)
        x->tick_state = 1;
    else if(x->tick_len){
        x->tick_pos = -1.0f;
        x->tick_start = -1.0f;
        x->tick_state = 2;
        outlet_bang(x->o_tick_sync[0]);
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
    x->tick_ratio = 0.f;
    x->tick_ppos = 0.f;
    post("sync len: %f",x->tick_len);
    outlet_float(x->o_tick_ratio,0);
}

void* ticker_new(){
 
    t_ticker* x = (t_ticker*)pd_new(ticker_class);
 
    x->o_tick_ratio = (t_outlet*)outlet_new(&x->x_obj, &s_float);
    x->o_tick_sync = (t_outlet**)malloc(sizeof(t_outlet*)*4);
    for(int i=0; i<4; i++){
        x->o_tick_sync[i] = (t_outlet*)outlet_new(&x->x_obj, &s_bang);
    }

    x->tick_count = 0;
    x->tick_state = 0;
    x->tick_pos = 0.0f;
    x->tick_len = 0.0f;
    x->tick_div = 2;
 
    return (void*)x;
}

void ticker_free(t_ticker* x){
    outlet_free(x->o_tick_ratio);
    for(int i=0; i<4; i++)
        outlet_free(x->o_tick_sync[i]);
    free(x->o_tick_sync);
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
    class_addmethod(ticker_class,(t_method)ticker_oninfoShort, gensym("dbg"),  (t_atomtype)0);
    class_addmethod(ticker_class,(t_method)ticker_onstart, gensym("start"),  (t_atomtype)0);
    class_addmethod(ticker_class,(t_method)ticker_onend,   gensym("stop"),   (t_atomtype)0);
    class_addmethod(ticker_class,(t_method)ticker_onreset, gensym("reset"),  (t_atomtype)0);
}
