#include "bank.h"


bool bank_isTopCtrlHeld(t_bank* x){
    return x->stateCTop && std::abs(x->holdCounter) > HOLD_TIME;
}
bool bank_isBotCtrlHeld(t_bank* x){
    return x->stateCBot && std::abs(x->holdCounter) > HOLD_TIME;
}
bool bank_hasQuanTick(t_bank* x){
    return x->tick_duration > 0;
}
bool bank_activeMotifIsClear(t_bank* x){
    return x->active_motif_ptr->state == _motif_state::m_clear;
}
bool bank_activeMotifIsBase(t_bank* x){
    return x->active_motif_ptr->state == _motif_state::m_base;
}
bool bank_activeMotifIsRec(t_bank* x){
    return x->active_motif_ptr->state == _motif_state::m_base || x->active_motif_ptr->state == _motif_state::m_dub;
}
bool bank_activeMotifIsDub(t_bank* x){
    return x->active_motif_ptr->state == _motif_state::m_dub;
}
bool bank_activeMotifIsPlay(t_bank* x){    
    return x->active_motif_ptr->state == _motif_state::m_play;
}
bool bank_activeMotifIsRunning(t_bank* x){    
    return x->active_motif_ptr->state == _motif_state::m_play || x->active_motif_ptr->state == _motif_state::m_dub;
}
bool bank_activeMotifIsStop(t_bank* x){
    return x->active_motif_ptr->state == _motif_state::m_stop;
}
////

void bank_dsp(t_bank *x, t_signal **sp)
{
  dsp_add(bank_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}
t_int* bank_perform(t_int *w)
{ 
    t_bank *x = (t_bank *)(w[1]);
    t_motif *m = x->active_motif_ptr;
 
    t_sample    *in  =      (t_sample *)(w[2]);
    t_sample    *out =      (t_sample *)(w[3]);
    int            n =             (int)(w[4]);

    x->tick_current += 1;
    int msync = 0;
    t_sample dub = m->isDubbing ? 1.0f : 0.0f;
    t_sample ins = 0;
    //int dataHeadMod = m->isLong ? MOTIF_BUF_SIZE : m->len_spl;

    //reset of whole bank
    if(bank_isBotCtrlHeld(x) && bank_activeMotifIsStop(x))
        bank_onReset(x);

    //audio processing
    switch(m->state){
        case _motif_state::m_base:
            for (int i=0; i<n; i++) {
                t_float ii = *in++;
                *out++ = 0;
                m->_aData[m->len_spl] = ii;
                m->_bData[m->len_spl] = ii;
                m->len_spl++;
                m->isLong = m->len_spl - MOTIF_BUF_SIZE/2; //if positive, true
            }
            m->len_syncs += 1;
            if(x->tick_duration <= 0)
                x->tick_duration -= 1;

            if(m->len_spl >= MOTIF_BUF_SIZE){
                m->len_spl = MOTIF_BUF_SIZE;
                m->len_syncs = m->len_spl / 64;
                x->tick_action_nstate = _motif_state::m_play;
                bank_outlet_mstats(x, m->len_syncs);
                bank_q(x); 
            }
        break;
           
        case _motif_state::m_dub:
        case _motif_state::m_play:
            for (int i=0; i<n; i++) {
                ins = *in++;
                *out++ = m->_data[m->pos_spl];
                m->_data[m->pos_spl] += (ins * dub);
                m->pos_spl = (m->pos_spl+1) % m->len_spl;
                if(m->pos_spl == 0 && x->onetime) m->pos_spl = m->len_spl-1;
            }

            m->pos_syncs += 1;
            if(m->pos_syncs >= m->len_syncs){
                m->pos_syncs = 0;
                m->last_sync = x->tick_current;
                msync = x->tick_current;
            }
        break;

        default:
            for (int i=0; i<n; i++)  *out++ = 0;
        break;
    }


    //n-state machine
    if(x->tick_action_pending){
        if(x->tick_current >= x->tick_action_when){
            //if state play and n_state stop => reset pos etc, next state machine essentially 
            x->tick_action_when = 0;
            bank_outlet_mstats(x, 0);

            if(m->state == _motif_state::m_stop && x->tick_action_nstate == _motif_state::m_play) 
                bank_motif_toStart(m);

            else if(m->state == _motif_state::m_play && x->tick_action_nstate == _motif_state::m_dub){
                //start dub
                m->isDubbing = 1;
                bank_motif_swapStreams(m);
            }
            else if(m->state == _motif_state::m_dub && x->tick_action_nstate == _motif_state::m_play){
                //confirm dub
                m->isDubbing = 0;
                x->work_type |= _motif_work_type::REFILL;
                x->work_data = m->len_spl;
            }
            else if(m->state == _motif_state::m_dub && x->tick_action_nstate == _motif_state::m_stop){
                //cancel
                m->isDubbing = 0;
                bank_motif_swapStreams(m);
                x->work_type |= _motif_work_type::REFILL;
                x->work_data = m->len_spl;
                x->tick_action_nstate = x->gate ? _motif_state::m_stop : _motif_state::m_play;
            }

            m->state = x->tick_action_nstate;
            x->tick_action_pending = 0;
        }
    }

    //sync signal outlet
    if(x->tick_duration > 0 && x->tick_current >= x->tick_start+x->tick_duration)
    {
        x->tick_start = x->tick_current;
        x->tick_next = x->tick_start + x->tick_duration;
        x->last_sync = x->tick_current;
        // bank_outlet_sync(x, msync);
        if(msync != x->tick_current)
        outlet_float(x->o_sync, float(x->tick_current * -1));
    }

    //signal worker thread that there is work to be done, i.e. confirm overdub data
    if(x->work_type){
        if(x->work_type &= _motif_work_type::REFILL){
            if(x->work_data == 0){
                x->work_type &= ~(_motif_work_type::REFILL);
                post("Refilled %d.spls for motif %d in bank %d", m->len_spl, x->active_motif_idx, x->id);
            }
            else{
                for(int c=0; c<REFILL_CHUNK; c++){
                    int j = m->len_spl - x->work_data;
                    for(int i=0; i<64; i++){
                        int ii = i+j;
                        m->_ndata[ii] = m->_data[ii];
                    }
                    x->work_data -= 64;

                    if(x->work_data == 0) break;
                }
            }
        }
    }

    //hold button counter
    if(x->holdCounter > 0 && !x->stateCTop)
        x->holdCounter = 0;
    if(x->holdCounter < 0 && !x->stateCBot)
        x->holdCounter = 0;

    if(x->holdCounter == 0 && x->stateCTop)
        x->holdCounter = 1;
    if(x->holdCounter == 0 && x->stateCBot)
        x->holdCounter = -1;
    
    if(x->holdCounter > 0) x->holdCounter++;
    if(x->holdCounter < 0) x->holdCounter--;

    if(x->debounceCounter) x->debounceCounter++;
    if(x->debounceCounter == DEBOUNCE_TIME) x->debounceCounter = 0;

    return (w+5);
    // return (w+2);
}







void bank_outlet_mstats(t_bank* x, t_float ticklen){
    int c = 3;
    SETFLOAT(x->a_m_stats   ,x->active_motif_ptr->state);
    SETFLOAT(x->a_m_stats+1 ,x->tick_action_pending ? x->tick_action_nstate : -1);
    SETFLOAT(x->a_m_stats+2 ,x->tick_action_pending ? x->tick_action_when : -1);
    
    SETFLOAT(x->a_m_stats+c, ticklen);
    if(ticklen > 0) c+=1;
    //  SETFLOAT(x->a_m_stats,x->active_motif_ptr->len_syncs);
    //  SETFLOAT(x->a_m_stats,x->active_motif_ptr->pos_syncs);
    outlet_list(x->o_m_state, &s_list, c, x->a_m_stats);
}

//que up action time aligned if alignment data exists
void bank_q(t_bank* x){
    if(x->synced && x->tick_duration) //if not gated then schedule on next tick 
        x->tick_action_when = x->tick_start+x->tick_duration;
    else //else execute on next block cycle (q64)
        x->tick_action_when = 0;
    x->tick_action_pending = 1; //signal action pending
    bank_outlet_mstats(x,0); // reflect stats on outlet
}


void bank_onTickLen(t_bank* x, t_floatarg t){
    if(x->tick_duration > 0) return;
    
    //banks with uninitiated quan tick data go through this routine once
    //normalize recorded sample len to sync len
    int m_len_spl = x->active_motif_ptr->len_spl;
    x->tick_duration = t;
    x->tick_current = 0;
    x->tick_next = t;
    if(bank_activeMotifIsBase(x)){
        x->active_motif_ptr->pos_syncs = 0;
        x->active_motif_ptr->len_syncs = x->tick_duration;
        x->active_motif_ptr->len_spl = x->tick_duration * 64;
        x->tick_action_nstate = x->gate ? _motif_state::m_play : _motif_state::m_stop;
        bank_q(x);
    }
    post("bank %d tick len: %f tick len spl %d (%d pre)", x->id, t, x->active_motif_ptr->len_spl, m_len_spl);
    // bank_outlet_sync(x,1);
}

void bank_onGetPos(t_bank* x){
    if(! x->is_active) return;
    float p = x->active_motif_ptr->len_syncs ? float(x->active_motif_ptr->pos_syncs) / float(x->active_motif_ptr->len_syncs) : 0.0f;
    post("pos: %f , len : %d",p, x->active_motif_ptr->len_syncs);
    post("bank %d tick len: %d", x->id, x->tick_duration);
    post("state %d", x->active_motif_ptr->state);
    post("mls:%d bls:%f", x->active_motif_ptr->last_sync, x->last_sync);
    post("gate%d", x->gate);
}





void bank_onReset(t_bank* x){
    x->gate = false;
    x->synced = false;
    x->onetime = false;
    x->tick_duration = 0;
    x->tick_action_pending = 0;
    x->tick_action_nstate = 0;
    bank_onTransportReset(x);
    bank_outlet_mstats(x,0);
    for(int m=0; m<x->motifs_array_count; m++)
        bank_clear_motif(x->motifs_array[m]);
        
    post("Cleared all motifs in bank %d", x->id);
}

void bank_onTransportReset(t_bank* x){
    x->tick_start = 0;
    x->tick_current = 0;
    x->tick_action_when = 0;
    x->tick_next = x->tick_duration;
    t_motif* m = x->active_motif_ptr;
    if(!bank_activeMotifIsClear(x)){
        m->state = _motif_state::m_stop;
        m->pos_syncs = 0;
    }
    // bank_outlet_sync(x,1);
}

void bank_onActivate(t_bank* x){
    x->is_active = 1;
    post("%d activated", x->id);
}

void bank_onDeactivate(t_bank* x){
    x->is_active = 0;
    post("%d deactivated", x->id);
}

void bank_signalSlotChange(t_bank* x, int slot){
    x->slotToChange = slot;
    bank_safeSlotChange(x);
}

void bank_safeSlotChange(t_bank* x){
    int slot = x->slotToChange;
    x->slotToChange = -1;

    if(slot > x->motifs_array_count-1 || slot < 0){
        return;
    }
    
    if(!bank_activeMotifIsClear(x)) {
        bank_motif_toStart(x->active_motif_ptr);
        x->active_motif_ptr->state = _motif_state::m_stop;
    }
    x->active_motif_ptr = x->motifs_array[slot];
    x->active_motif_idx = slot;
    
    if(!bank_activeMotifIsClear(x)) {
        bank_motif_toStart(x->active_motif_ptr);
        x->active_motif_ptr->state = _motif_state::m_stop;
    }

    post("%d current slot: %d",x->id, x->active_motif_idx);
}

void bank_onNextSlot(t_bank* x){
    if(! x->is_active) return;
    bank_signalSlotChange(x,x->active_motif_idx + 1);
}

void bank_onPrevSlot(t_bank* x){
    if(! x->is_active) return;
    bank_signalSlotChange(x,x->active_motif_idx - 1);
}


void bank_onControlTopOn(t_bank* x){
    x->stateCTop = true;
    if(!x->is_active) return;

    if(bank_activeMotifIsRunning(x) && !x->gate){
        x->tick_action_nstate = _motif_state::m_dub;
        bank_q(x);
    }
    else if(bank_activeMotifIsDub(x) && !x->gate){
        if(!(x->work_type &= _motif_work_type::REFILL)){
            x->tick_action_nstate = _motif_state::m_play;
            bank_q(x);
        }
    }
    else if(bank_activeMotifIsStop(x) && !bank_activeMotifIsClear(x)){
        x->tick_action_nstate = _motif_state::m_play;
        bank_q(x);
    }
   
    post("TOP_ON %d current motif state: %d", x->id, x->tick_action_nstate);
}

void bank_onControlTopOff(t_bank* x){
    x->stateCTop = false;
    if(!x->gate || !x->is_active) return;

    if(x->gate){
        if(bank_activeMotifIsRunning(x)){
            x->tick_action_nstate = _motif_state::m_stop;
            bank_q(x);
            post("stop gate");
        }
    }
    
    post("TOP_OFF %d current motif state: %d", x->id, x->tick_action_nstate);
}


void bank_onControlBotOn(t_bank* x){
    x->stateCBot = true;
    if(!x->is_active) return;
    
    //initiate base rec
    if(bank_activeMotifIsClear(x)){
        x->tick_action_nstate = _motif_state::m_base;
        if(x->stateCTop) x->synced = true;
        bank_q(x);
        return;
    }

    //finish base rec
    if(bank_activeMotifIsBase(x)){
        //if start button held, change mode to loop
        
        if(x->stateCTop){
            //post changes to sync listeners to update banks with new project quan length
            if(x->tick_duration < 0){
                t_float t = x->tick_duration * -1.0;
                bank_outlet_mstats(x, t);
                //bank_postTicklenChange(x,t);
                post("bank %d set new tick len %f", x->id, t);
            }
            //loop        
            x->gate = false;
            x->onetime = false;
            x->synced = bank_hasQuanTick(x) ? false : true;
            x->tick_action_nstate = _motif_state::m_play;
            bank_q(x);
            x->synced = true;
        }
        else{
            //gated
            x->gate = true;
            x->synced = false;
            x->onetime = false;
            x->tick_action_nstate = _motif_state::m_stop;
            bank_q(x);
        }
        return;
    }
    
    if(x->gate){
        if(bank_activeMotifIsRunning(x)){
            x->tick_action_nstate = _motif_state::m_dub;
            bank_q(x);
        }
    }
    else{
        if(bank_activeMotifIsRunning(x)){
            x->tick_action_nstate = _motif_state::m_stop;
            bank_q(x);
        }
    } 

    // bank_outlet_mstats(x,0);
    post("BOT_ON %d current motif state: %d", x->id, x->tick_action_nstate);
}

void bank_onControlBotOff(t_bank* x){
    x->stateCBot = false;
    if(!x->gate || !x->is_active) return;

    if(x->gate){
        if(bank_activeMotifIsDub(x)){
            if(!(x->work_type &= _motif_work_type::REFILL)){
                x->tick_action_nstate = _motif_state::m_play;
                bank_q(x);
            }
        }
    }
    post("BOT_OFF %d current motif state: %d", x->id, x->tick_action_nstate);
}

void bank_onControlTop(t_bank *x, t_floatarg state){
    if(x->debounceCounter) return;
    x->debounceCounter = 1;
    if(state > 0.f) bank_onControlTopOn(x);
    else            bank_onControlTopOff(x);
}

void bank_onControlBot(t_bank *x, t_floatarg state){
    if(x->debounceCounter) return;
    x->debounceCounter = 1;
    if(state > 0.f) bank_onControlBotOn(x);
    else            bank_onControlBotOff(x);
}


void bank_onOvertakeRecord(t_bank* x){
    if(bank_activeMotifIsDub(x)){
        x->active_motif_ptr->n_state = _motif_state::m_stop;
        bank_q(x);
    }
}






void bank_clear_motif(t_motif* m){
    m->state      = _motif_state::m_clear;
    m->n_state    = _motif_state::m_clear;
    m->pos_spl    = 0;
    m->pos_ratio  = 0;
    m->pos_syncs  = 0;
    m->len_spl    = 0;
    m->len_syncs  = 0;
    m->dataHead = 0;
    m->isDubbing = 0;
    m->_data = m->_aData;
    m->_ndata = m->_bData;
}

void bank_motif_toStart(t_motif* m){
    m->pos_syncs = 0;
    m->pos_spl = 0;
    m->dataHead = 0;
}

void bank_motif_swapStreams(t_motif* m){
    if(m->_data == m->_aData) {
        m->_data = m->_bData;
        m->_ndata = m->_aData;
    }
    else {
        m->_data = m->_aData;
        m->_ndata = m->_bData;
    }
}

// void* bank_worker_thread(void* arg){
//     t_bank* x = (t_bank*)arg;

//     post("new thread for bank id: %d", x->id);

//     while(x->worker_thread_alive){
//         pthread_mutex_lock(&x->mtx_work);
//         while(x->work_type_inthread == 0)
//             pthread_cond_wait(&x->cond_work, &x->mtx_work);
        
//         post("new work for worker thread");
//         if(x->work_type_inthread & _motif_work_type::REFILL){
//             t_motif* m = x->active_motif_ptr;
//             for(int i=0; i<m->len_spl; i++){
//                 m->_ndata[i] = m->_data[i];
//             }
//             x->work_type_inthread &= ~(_motif_work_type::REFILL);
//             post("refilled %d.spls for motif %d in bank %d", m->len_spl, x->active_motif_idx, x->id);
//         }

//         pthread_mutex_unlock(&x->mtx_work);
//     }

//     post("ending thread for bank id: %d", x->id);
//     return NULL;
// }





void* bank_new(t_floatarg id){
    t_bank* x = (t_bank*)pd_new(bank_class);
    //f signal in
    x->i_ctl_top = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("on_ctl_top"));
    x->i_ctl_bot = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("on_ctl_bot"));
    x->i_tick_stats = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("on_tick_len"));
    x->o_loop_sig = outlet_new(&x->x_obj,&s_signal);
    x->o_sync = outlet_new(&x->x_obj,&s_float);
    x->o_m_state = outlet_new(&x->x_obj,&s_list);

    x->motifs_array = (t_motif**)malloc(4 * sizeof(t_motif*));
    for(int i=0; i<4; i++){
        x->motifs_array[i] = (t_motif*)malloc(sizeof(t_motif));
        x->motifs_array[i]->_aData = (t_sample*)malloc(sizeof(t_sample) * MOTIF_BUF_SIZE);
        x->motifs_array[i]->_bData = (t_sample*)malloc(sizeof(t_sample) * MOTIF_BUF_SIZE);
        bank_clear_motif(x->motifs_array[i]);
    }
    
    x->id = (int)id;
    x->motifs_array_count = 4;
    x->active_motif_idx = 0;
    x->active_motif_ptr = x->motifs_array[x->active_motif_idx];
    x->is_active = 0;
    x->tick_duration = 0;
    x->tick_start = 0;
    x->tick_next = 0;
    x->tick_current = 0;
    x->gate = false;
    x->onetime = false;
    x->synced = false;
    x->debounceCounter = 0;

    post("[%s] new bank with id: %d", VER, x->id);
    // pthread_mutex_init(&x->mtx_work, NULL);
    // pthread_cond_init(&x->cond_work, NULL);
    // x->worker_thread_alive = 1;
    // x->work_type = 0;
    // x->work_type_inthread = 0;
    // pthread_create(&x->worker_thread, NULL, bank_worker_thread, x);

    return (void*)x;
}

void bank_free(t_bank* x){

    // pthread_mutex_lock(&x->mtx_work);
    // pthread_cond_broadcast(&x->cond_work);
    // x->work_type = 0;
    // x->worker_thread_alive = 0;
    // pthread_mutex_unlock(&x->mtx_work);
    // pthread_join(x->worker_thread, NULL);
    // post("bank [%d] worker thread ended",x->id);

    inlet_free(x->i_ctl_top);
    inlet_free(x->i_ctl_bot);
    inlet_free(x->i_tick_stats);
    outlet_free(x->o_loop_sig);
    outlet_free(x->o_m_state);
    outlet_free(x->o_sync);
    for(int i=0; i<4; i++){    
        free(x->motifs_array[i]->_aData);
        free(x->motifs_array[i]->_bData);
        free(x->motifs_array[i]);
    }
    //free(x->motifs_array);
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
    
    CLASS_MAINSIGNALIN(bank_class, t_bank, f);

    class_addmethod(bank_class, (t_method) bank_dsp      ,gensym("dsp")    , A_CANT, (t_atomtype)0);

    class_addmethod(bank_class, (t_method) bank_onReset  ,gensym("clean")   ,(t_atomtype)0 );
    class_addmethod(bank_class, (t_method) bank_onTransportReset   ,gensym("t_start")   ,(t_atomtype)0 );

    class_addmethod(bank_class, (t_method) bank_onControlTop ,gensym("on_ctl_top"), A_DEFFLOAT ,(t_atomtype)0 );
    class_addmethod(bank_class, (t_method) bank_onControlBot ,gensym("on_ctl_bot"), A_DEFFLOAT ,(t_atomtype)0 );
    class_addmethod(bank_class, (t_method) bank_onOvertakeRecord   ,gensym("overtake_record")   ,(t_atomtype)0 );

    class_addmethod(bank_class, (t_method) bank_onActivate   ,gensym("activate")   ,(t_atomtype)0 );
    class_addmethod(bank_class, (t_method) bank_onDeactivate   ,gensym("deactivate")   ,(t_atomtype)0 );

    class_addmethod(bank_class, (t_method) bank_onNextSlot   ,gensym("next_slot")   ,(t_atomtype)0 );
    class_addmethod(bank_class, (t_method) bank_onPrevSlot   ,gensym("prev_slot")   ,(t_atomtype)0 );
    class_addmethod(bank_class, (t_method) bank_onGetPos     ,gensym("get_pos")     ,(t_atomtype)0 );

    class_addmethod(bank_class, (t_method) bank_onTickLen,    gensym("on_tick_len"),  A_DEFFLOAT, (t_atomtype)0);
}
