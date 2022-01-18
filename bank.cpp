#include "bank.h"

void bank_dsp(t_bank *x, t_signal **sp)
{
  dsp_add(bank_perform, 1, x);
}
t_int* bank_perform(t_int *w)
{ 
    t_bank *x = (t_bank *)(w[1]);
    t_motif *m = x->active_motif_ptr;
 
//   t_sample    *in  =      (t_sample *)(w[2]);
//   t_sample    *out =      (t_sample *)(w[3]);
//   int            n =             (int)(w[4]);
//   while (n--) *out++ = (*in++)*(x->phase);

    x->tick_current += 1.0f;
    
    switch(m->state){
        case _motif_state::m_base:
            m->len_syncs += 1.0f;
            if(x->tick_duration <= 0.0f)
                x->tick_duration -= 1.0f;
        break;

        case _motif_state::m_play:
            m->pos_syncs += 1.0f;
            if(m->pos_syncs >= m->len_syncs){
                m->pos_syncs = 0.0f;
                m->last_sync = x->tick_current;
                outlet_float(x->o_m_sync, x->tick_current);
            }
        break;
    }

    if(x->tick_action_pending){
        if(x->tick_current >= x->tick_action_when){
            //if state play and n_state stop => reset pos etc, next state machine essentially 
            m->state = x->tick_action_nstate;
            x->tick_action_pending = 0;
            outlet_float(x->o_tick_pending, 0.0f);
        }
    }

    if(x->tick_duration > 0 && x->tick_current >= x->tick_start+x->tick_duration)
    {
        x->tick_start = x->tick_current;
        x->tick_next = x->tick_start + x->tick_duration;
        x->last_sync = x->tick_current;
        outlet_float(x->o_sync, x->tick_current);
    }
    
    return (w+2);
}

void bank_onReset(t_bank* x){
    x->tick_current = 0;
    x->tick_duration = 0;
    x->tick_next = 0;
    x->tick_start = 0;
    x->tick_action_pending = 0;
    x->tick_action_nstate = 0;
    x->tick_action_when = 0;
    outlet_float(x->o_sync, 0);
    outlet_float(x->o_m_sync, 0);
    outlet_float(x->o_tick_pending, 0);
    bank_clear_motif(x->active_motif_ptr);
    post("reset: %d",x->active_motif_idx);
}

void bank_onTransportReset(t_bank* x){
    x->tick_start = 0;
    x->tick_current = 0;
    x->tick_action_when = 0;
    t_motif* m = x->active_motif_ptr;
    if(m->state != _motif_state::m_clear){
        m->state = _motif_state::m_stop;
        m->pos_syncs = 0;
    }
}
void bank_onActivate(t_bank* x){
    x->is_active = 1;
    post("%d activated", x->id);
}
void bank_onDeactivate(t_bank* x){
    x->is_active = 0;
    post("%d deactivated", x->id);
}
void bank_onTickLen(t_bank* x, t_floatarg t){
    if(x->tick_duration > 0) return;

    x->tick_duration = t;
    x->tick_current = 0;
    if(x->active_motif_ptr->state == _motif_state::m_base){
        x->active_motif_ptr->state = _motif_state::m_play;
        x->active_motif_ptr->pos_syncs = 0;
        x->active_motif_ptr->len_syncs = x->tick_duration;
    }
    post("bank %d tick len: %f", x->id, t);
}

void bank_onGetPos(t_bank* x){
    if(! x->is_active) return;
    float p = x->active_motif_ptr->len_syncs ? x->active_motif_ptr->pos_syncs / x->active_motif_ptr->len_syncs : 0.0f;
    post("pos: %f , len : %f",p, x->active_motif_ptr->len_syncs);
    post("bank %d tick len: %f", x->id, x->tick_duration);
    post("state %d", x->active_motif_ptr->state);
    post("mls:%f bls:%f", x->active_motif_ptr->last_sync, x->last_sync);
}

void bank_onNextSlot(t_bank* x){
    if(! x->is_active) return;

    if(x->active_motif_idx < 3) 
        x->active_motif_idx += 1;

    x->active_motif_ptr = x->motifs_array[x->active_motif_idx];
    // outlet_float(x->o_active_motif_slot, (float)x->active_motif_ptr->len_syncs);
    post("%d current slot: %d",x->id, x->active_motif_idx);
}

void bank_onPrevSlot(t_bank* x){
    if(! x->is_active) return;
    
    if(x->active_motif_idx > 0) 
        x->active_motif_idx -= 1;

    x->active_motif_ptr = x->motifs_array[x->active_motif_idx];
    // outlet_float(x->o_active_motif_slot, (float)x->active_motif_ptr->len_syncs);
    post("%d current slot: %d", x->id, x->active_motif_idx);
}

void bank_q(t_bank* x){
    x->tick_action_when = x->tick_start+x->tick_duration;
    x->tick_action_pending = 1;
    outlet_float(x->o_tick_pending, x->tick_action_when);
}

void bank_onLaunch(t_bank* x){
    if(x->is_active == 0) return;

    switch (x->active_motif_ptr->state)
    {
    case _motif_state::m_clear:
        x->tick_action_nstate = _motif_state::m_base;
        bank_q(x);
    break;

    case _motif_state::m_base:
        if(x->tick_duration < 0){
            float t = x->tick_duration * -1.0;
            outlet_float(x->o_tick_len, t);
            post("%d set new tick len %f", x->id, t);
        }
    case _motif_state::m_stop:
        x->tick_action_nstate = _motif_state::m_play;
        bank_q(x);
    break;
    }

    post("%d current motif state: %d", x->id, x->tick_action_nstate);
}

void bank_onStop(t_bank* x){
    if(x->is_active == 0) return;

    switch (x->active_motif_ptr->state)
    {
    case _motif_state::m_base:
        x->tick_action_nstate = _motif_state::m_clear;
    break;

    case _motif_state::m_play:
        x->tick_action_nstate = _motif_state::m_stop;
    break;
    }

    x->tick_action_when = x->tick_start+x->tick_duration;
    x->tick_action_pending = 1;
    outlet_float(x->o_tick_pending, x->tick_action_when);
    post("%d current motif state: %d", x->id, x->tick_action_nstate);
}

void bank_clear_motif(t_motif* m){
    m->state      = _motif_state::m_clear;
    m->n_state    = _motif_state::m_clear;
    m->pos_spl    = 0.0f;
    m->pos_ratio  = 0.0f;
    m->pos_syncs  = 0.0f;
    m->len_spl    = 0.0f;
    m->len_syncs  = 0.0f;
    //m->buf
}

void* bank_new(t_floatarg id){
    t_bank* x = (t_bank*)pd_new(bank_class);
    x->i_tick_stats = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("on_tick_len"));
    x->o_tick_len = outlet_new(&x->x_obj,&s_float);
    x->o_tick_pending = outlet_new(&x->x_obj,&s_float);
    x->o_sync = outlet_new(&x->x_obj,&s_float);
    x->o_m_sync = outlet_new(&x->x_obj,&s_float);

    x->motifs_array = (t_motif**)malloc(4 * sizeof(t_motif*));
    for(int i=0; i<4; i++){
        x->motifs_array[i] = (t_motif*)malloc(sizeof(t_motif));
        bank_clear_motif(x->motifs_array[i]);
    }
    
    x->id = (int)id;
    x->active_motif_idx = 0;
    x->active_motif_ptr = x->motifs_array[x->active_motif_idx];
    x->is_active = 0;
    x->tick_duration = 0;
    x->tick_start = 0;
    x->tick_next = 0;
    x->tick_current = 0;

    post("new bank with id: %d", x->id);

    return (void*)x;
}

void bank_free(t_bank* x){
    inlet_free(x->i_tick_stats);
    outlet_free(x->o_tick_len);
    outlet_free(x->o_tick_pending);
    outlet_free(x->o_sync);
    for(int i=0; i<4; i++)
        free(x->motifs_array[i]);
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

    class_addmethod(bank_class, (t_method) bank_dsp      ,gensym("dsp")    , A_CANT, (t_atomtype)0);
    class_addmethod(bank_class, (t_method) bank_onReset  ,gensym("clear")   ,(t_atomtype)0 );
    class_addmethod(bank_class, (t_method) bank_onLaunch ,gensym("q_launch") ,(t_atomtype)0 );
    class_addmethod(bank_class, (t_method) bank_onStop   ,gensym("q_stop")   ,(t_atomtype)0 );
    class_addmethod(bank_class, (t_method) bank_onTransportReset   ,gensym("t_start")   ,(t_atomtype)0 );

    class_addmethod(bank_class, (t_method) bank_onActivate   ,gensym("activate")   ,(t_atomtype)0 );
    class_addmethod(bank_class, (t_method) bank_onDeactivate   ,gensym("deactivate")   ,(t_atomtype)0 );
    
    class_addmethod(bank_class, (t_method) bank_onNextSlot   ,gensym("next_slot")   ,(t_atomtype)0 );
    class_addmethod(bank_class, (t_method) bank_onPrevSlot   ,gensym("prev_slot")   ,(t_atomtype)0 );
    class_addmethod(bank_class, (t_method) bank_onGetPos     ,gensym("get_pos")     ,(t_atomtype)0 );

    class_addmethod(bank_class, (t_method) bank_onTickLen,    gensym("on_tick_len"),  A_DEFFLOAT, (t_atomtype)0);
}
