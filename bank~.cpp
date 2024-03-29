#include "bank~.h"
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

void bank_postUnlatchUpdate(t_bank* x);

bool bank_isMainCtrlHeld(t_bank* x){
    return x->stateTrigger && std::abs(x->holdCounter) > HOLD_TIME;
}
bool bank_isAltCtrlHeld(t_bank* x){
    return x->stateCAlt && std::abs(x->holdCounter) > HOLD_TIME;
}
bool bank_hasQuanTick(t_bank* x){
    return x->hasQuantick;
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
    t_sample dub = m->isDubbing ? 1.0f : 0.0f;
    t_sample ins = 0;
    //int dataHeadMod = m->isLong ? MOTIF_BUF_SIZE : m->len_spl;


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
            if(!x->hasQuantick)
                x->tick_duration -= 1;

            //fail safe when record length goes to max time
            if(m->len_spl >= MOTIF_BUF_SIZE){
                m->len_spl = MOTIF_BUF_SIZE;
                m->len_syncs = m->len_spl / 64;
                x->tick_action_nstate = _motif_state::m_play;
                bank_onTriggerOn(x);
            }
        break;
           
        case _motif_state::m_dub:
        case _motif_state::m_play:
        
            if(m->isDone && x->onetime){
                for (int i=0; i<n; i++)  *out++ = 0;
            } 
            else{
                for (int i=0; i<n; i++) {
                    ins = *in++;
                    *out++ = m->_data[m->pos_spl];
                    m->_data[m->pos_spl] += (ins * dub);
                    m->pos_spl = (m->pos_spl+1) % m->len_spl;  
                }
                m->pos_syncs += 1;

                if(m->pos_syncs >= m->len_syncs){
                    if(x->onetime) m->isDone = 1;
                    m->pos_syncs = 0;
                }
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

            if(m->state == _motif_state::m_clear || x->tick_action_nstate == _motif_state::m_base){
                x->when_base = x->tick_current;
            }

            if((m->state == _motif_state::m_clear || m->state == _motif_state::m_stop) && x->tick_action_nstate == _motif_state::m_play) 
                bank_motif_toStart(m);
            
            else if(m->state == _motif_state::m_base && x->tick_action_nstate == _motif_state::m_play ){  
                x->active_motif_ptr->pos_syncs = 0;
                x->active_motif_ptr->len_syncs = x->tick_current - x->when_base;
                if(!x->hasQuantick){
                    bank_onTransportReset(x);
                    x->hasQuantick = true;
                    x->tick_duration+=1;
                    x->active_motif_ptr->len_syncs-=1;
                } 
                x->active_motif_ptr->len_spl = x->active_motif_ptr->len_syncs * 64;
                //master quan tick recorder fix len
            }

            else if((m->state == _motif_state::m_play || m->state == _motif_state::m_stop) && x->tick_action_nstate == _motif_state::m_dub){
                //start dub
                if(m->state == _motif_state::m_stop) 
                    bank_motif_toStart(m);
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
                x->holdCounter = 0;
                m->isDubbing = 0;
                bank_motif_swapStreams(m);
                x->work_type |= _motif_work_type::REFILL;
                x->work_data = m->len_spl;
                x->tick_action_nstate = x->gate ? _motif_state::m_stop : _motif_state::m_play;
                bank_postUnlatchUpdate(x);
            }

            m->state = x->tick_action_nstate;
            x->tick_action_pending = 0;
            post("pending action done bank %d",x->id);
            bank_postStateUpdate(x);
        }
    }

    //next sync tick calculation
    if(x->tick_duration > 0 && x->tick_current >= x->tick_start+x->tick_duration)
    {
        x->tick_start = x->tick_current;
        x->tick_next = x->tick_start + x->tick_duration;
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

    //hold button counter clear
    if(x->holdCounter > 0 && !x->stateCAlt)
        x->holdCounter = 0;
    if(x->holdCounter < 0 && !x->stateTrigger)
        x->holdCounter = 0;

    if(x->holdCounter == 0 && x->stateCAlt)
        x->holdCounter = 1;
    if(x->holdCounter == 0 && x->stateTrigger)
        x->holdCounter = -1;
    
    if(x->holdCounter > 0) x->holdCounter++;
    if(x->holdCounter < 0) x->holdCounter--;

    if(x->debounceCounter) x->debounceCounter++;
    if(x->debounceCounter == DEBOUNCE_TIME) x->debounceCounter = 0;

    return (w+5);
    // return (w+2);
}




void bank_setSyncTick(t_bank* x, t_floatarg t){
    if(x->tick_duration > 0) return;
    if(t <= 0) return;
    
    //banks with uninitiated quan tick data go through this routine once
    //normalize recorded sample len to sync len
    x->hasQuantick = true;
    int m_len_spl = x->active_motif_ptr->len_spl;
    x->tick_duration = t;
    x->tick_current = -1; //set sync tick in other banks will be out by 1 tick so need to compensate
    x->tick_next = t;
    post("bank %d has new tick len: %f tick len spl %d (%d pre)", x->id, t, x->active_motif_ptr->len_spl, m_len_spl);
    // bank_outlet_sync(x,1);
}

void bank_postBase(t_bank* x){
    SETFLOAT(x->a_info_list+0, x->isActive);
    SETFLOAT(x->a_info_list+1, x->populatedCount);
    SETFLOAT(x->a_info_list+2, x->tick_action_nstate);
    SETFLOAT(x->a_info_list+3, x->active_motif_ptr->state);
}

void bank_postUnlatchUpdate(t_bank* x){
    x->stateCAlt = false;
    outlet_float(x->o_control, 0);
}
void bank_postQuanUpdate(t_bank* x){
    bank_postBase(x);
    SETFLOAT(x->a_info_list+4, x->tick_duration);
    outlet_list(x->o_info, &s_list, 5, x->a_info_list);
}

void bank_postStateUpdate(t_bank* x){
    bank_postBase(x);
    outlet_list(x->o_info, &s_list, 4, x->a_info_list);
}


//que up action time aligned if alignment data exists
void bank_q(t_bank* x){
    if(x->synced && x->tick_duration) {//if not gated then schedule on next tick 
        x->tick_action_when = x->tick_start+x->tick_duration;
    }
    else //else execute on next block cycle (q64)
        x->tick_action_when = 0;
    x->tick_action_pending = 1; //signal action pending
    bank_postStateUpdate(x);
}




void bank_debugInfo(t_bank* x){
    if(! x->isActive) return;
    float p = x->active_motif_ptr->len_syncs ? float(x->active_motif_ptr->pos_syncs) / float(x->active_motif_ptr->len_syncs) : 0.0f;
    post("=======bank[id%d]=====", x->id);
    post("active slot [slot%d]",x->active_motif_idx);
    post("posPer: %f , posA:%d, len : %d",p, x->active_motif_ptr->pos_syncs, x->active_motif_ptr->len_syncs);
    post("bank %d tick len: %d, tick %d", x->id, x->tick_duration, x->tick_current);
    post("state %d", x->active_motif_ptr->state);
    post("gate%d", x->gate);
}




void bank_onActivate(t_bank* x){
    x->isActive = 1;
    post("%d activated", x->id);
    bank_postStateUpdate(x);
}

void bank_onDeactivate(t_bank* x){
    x->isActive = 0;
    post("%d deactivated", x->id);
    bank_postStateUpdate(x);
}

void bank_safeSlotChange(t_bank* x, int slot){
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

    bank_postStateUpdate(x);
    post("%d current slot: %d",x->id, x->active_motif_idx);
}

void bank_onNextSlot(t_bank* x){
    if(! x->isActive) return;
    bank_safeSlotChange(x,x->active_motif_idx + 1);
}

void bank_onPrevSlot(t_bank* x){
    if(! x->isActive) return;
    bank_safeSlotChange(x,x->active_motif_idx - 1);
}


void bank_onTriggerOn(t_bank* x){
    x->stateTrigger = true;
    if(!x->isActive) return;

   
    //initiate base rec
    if(bank_activeMotifIsClear(x)){
        //[2] 
        //[3]
        x->tick_action_nstate = _motif_state::m_base;
        if(!x->stateCAlt) x->synced = true;
        bank_q(x);
        post("base rec bank %d [%s]",x->id, x->synced ? "SYNC" : "FREE");
        return;
    }

    //finish base rec
    if(bank_activeMotifIsBase(x)){
        //if start button held, change mode to loop
        //[4]
        //[5]
        if(!x->stateCAlt){
             //post changes to sync listeners to update banks with new project quan length
            if(!x->hasQuantick){
                x->tick_duration *= -1;
                bank_postQuanUpdate(x);
                post("bank %d setting new tick len %d", x->id, x->tick_duration);
            }

            //loop        
            x->gate = false;
            x->onetime = false;
            x->synced = bank_hasQuanTick(x) ? true : false;
            x->tick_action_nstate = _motif_state::m_play;
            bank_q(x);
            x->synced = true;

        }
        else{
            //bank_postUnlatchUpdate(x);
            //gated
            x->gate = true;
            x->synced = false;
            x->onetime = true;
            x->tick_action_nstate = _motif_state::m_stop;
            bank_q(x);
        }
        x->populatedCount++;
        post("base rec end bank %d [%s]",x->id, x->synced ? "SYNC" : "FREE");
        return;
    }
    
    if(x->gate){
        //[11]
        //[16]
        if(bank_activeMotifIsStop(x) && !bank_activeMotifIsClear(x)){
            x->tick_action_nstate = x->stateCAlt ? _motif_state::m_dub : _motif_state::m_play;
            bank_q(x);
        }
    }
    else{

        //[8]
        if(bank_activeMotifIsDub(x)){
            if(!(x->work_type &= _motif_work_type::REFILL)){
                x->tick_action_nstate = _motif_state::m_play;
                bank_q(x);
                x->tick_action_when = 0;
            }
        }
        
        //[6]
        //[7]
        else if(bank_activeMotifIsPlay(x)){
            if(x->tick_action_nstate == _motif_state::m_stop){
                x->tick_action_nstate = _motif_state::m_dub;
                bank_q(x);
                x->tick_action_when = 0;
            }
            else{
                x->tick_action_nstate = _motif_state::m_stop;
                bank_q(x);
            }
        }    
    
        //[10]
        else if(bank_activeMotifIsStop(x) && !bank_activeMotifIsClear(x)){
            x->tick_action_nstate = _motif_state::m_play;
            bank_postUnlatchUpdate(x);
            bank_q(x);
        }
    } 

 }

void bank_onTriggerOff(t_bank* x){
    x->stateTrigger = false;
    if(!x->isActive) return;

    // //reset of whole bank
    // if(bank_isAltCtrlHeld(x)){
    //     bank_onDelete(x);
    //     bank_postUnlatchUpdate(x);
    // }

    if(bank_activeMotifIsStop(x) && x->onetime) return;
    
    if(x->gate){
        x->active_motif_ptr->isDone = false;
        //[12]
        //[15]
        if(bank_activeMotifIsRunning(x)){
            x->tick_action_nstate = _motif_state::m_stop;
            bank_q(x);
            post("stop gate");
        }
    }
}


void bank_onControlAltOn(t_bank* x){
    if(!x->isActive) return;

    if(x->gate){
        //[13]
        if(bank_activeMotifIsPlay(x) && x->stateTrigger){
            x->tick_action_nstate = _motif_state::m_dub;
            bank_q(x);
        }
    }
    else{
        //[9]
        if(bank_activeMotifIsDub(x)){
            bank_postUnlatchUpdate(x);
            x->tick_action_nstate = _motif_state::m_stop;
            auto s = x->synced;
            x->synced = bank_activeMotifIsDub(x) ? false : x->synced;
            bank_q(x);
            x->synced = s;
        }
    }
   
}

void bank_onControlAltOff(t_bank* x){
    if(!x->gate || !x->isActive) return;

    if(x->gate){
        //[14]
        if(bank_activeMotifIsDub(x) && x->stateTrigger){
            if(!(x->work_data &= _motif_work_type::REFILL)){
                x->tick_action_nstate = _motif_state::m_play;
                bank_q(x);
            }
        };
    }
}


void bank_onControlAlt(t_bank *x, t_floatarg state){
    if(x->debounceCounter) return;
    x->debounceCounter = 1;
    if(state > 0.f)
    x->stateCAlt = !x->stateCAlt;
    
    if(x->stateCAlt) bank_onControlAltOn(x);
    else{
        bank_onControlAltOff(x);
    }
    outlet_float(x->o_control, x->stateCAlt);
    post("bank [%d] alt:%d",x->id,x->stateCAlt);
}

void bank_onControlMain(t_bank *x, t_floatarg state){
    if(x->debounceCounter) return;
    x->debounceCounter = 1;
    if(state > 0.f) bank_onTriggerOn(x);
    else            bank_onTriggerOff(x);
}


void bank_onOvertakeRecord(t_bank* x){
    if(bank_activeMotifIsDub(x)){
        x->active_motif_ptr->n_state = _motif_state::m_stop;
        bank_q(x);
    }
}

void bank_onUnlatch(t_bank* x){
    x->stateCAlt = false;
}



void bank_onDelete(t_bank* x){
    x->gate = false;
    x->synced = false;
    x->onetime = false;
    // x->tick_duration = 0;
    x->tick_action_pending = 0;
    x->tick_action_nstate = 0;
    // x->hasQuantick = false;
    //bank_onTransportReset(x);
    bank_clear_motif(x->active_motif_ptr);
    x->populatedCount--;
    bank_postStateUpdate(x);
    post("Deleted motif %d in bank %d",x->active_motif_idx, x->id);
}

void bank_onReset(t_bank* x){
    x->stateCAlt = false;
    bank_postUnlatchUpdate(x);
    x->gate = false;
    x->synced = false;
    x->onetime = false;
    x->tick_duration = 0;
    x->tick_action_pending = 0;
    x->tick_action_nstate = 0;
    x->hasQuantick = false;
    bank_onTransportReset(x);
    for(int m=0; m<x->motifs_array_count; m++)
        bank_clear_motif(x->motifs_array[m]);
        
    post("Cleared all motifs in bank %d", x->id);
}

void bank_onTransportReset(t_bank* x){
    x->tick_start = 0;
    //x->tick_current = 0;
    x->tick_action_when = 0;
    x->tick_next = x->tick_duration;
    t_motif* m = x->active_motif_ptr;
    if(!bank_activeMotifIsClear(x)){
        m->state = _motif_state::m_stop;
        m->pos_syncs = 0;
    }
    // bank_outlet_sync(x,1);
}

void bank_onTransportStop(t_bank* x){
    x->tick_current = 0;
    bank_onTransportReset(x);
}

void bank_clear_motif(t_motif* m){
    m->state      = _motif_state::m_clear;
    m->n_state    = _motif_state::m_clear;
    m->pos_spl    = 0;
    m->pos_ratio  = 0;
    m->pos_syncs  = 0;
    m->len_spl    = 0;
    m->len_syncs  = 0;
    m->isDubbing = 0;
    m->isDone = 0;
    m->_data = m->_aData;
    m->_ndata = m->_bData;
}

void bank_motif_toStart(t_motif* m){
    m->pos_syncs = 0;
    m->pos_spl = 0;
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





void* bank_tilde_new(t_floatarg id){
    t_bank* x = (t_bank*)pd_new(bank_tilde_class);
    //f signal in
    x->i_trigger = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("on_ctl_main"));
    x->i_control = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("on_ctl_alt"));
    x->i_sync = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("set_sync_tick"));
    x->o_loop_sig = outlet_new(&x->x_obj,&s_signal);
    x->o_control = outlet_new(&x->x_obj, &s_float);
    x->o_info = outlet_new(&x->x_obj,&s_list);
    

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
    x->isActive = 0;
    x->tick_duration = 0;
    x->tick_start = 0;
    x->tick_next = 0;
    x->tick_current = -1;
    x->gate = false;
    x->onetime = false;
    x->synced = false;
    x->debounceCounter = 0;
    x->stateCShift = 0;
    x->hasQuantick = false;

    post("[%s] new bank with id: %d", VER, x->id);
    // pthread_mutex_init(&x->mtx_work, NULL);
    // pthread_cond_init(&x->cond_work, NULL);
    // x->worker_thread_alive = 1;
    // x->work_type = 0;
    // x->work_type_inthread = 0;
    // pthread_create(&x->worker_thread, NULL, bank_worker_thread, x);

    return (void*)x;
}

void bank_tilde_free(t_bank* x){

    // pthread_mutex_lock(&x->mtx_work);
    // pthread_cond_broadcast(&x->cond_work);
    // x->work_type = 0;
    // x->worker_thread_alive = 0;
    // pthread_mutex_unlock(&x->mtx_work);
    // pthread_join(x->worker_thread, NULL);
    // post("bank [%d] worker thread ended",x->id);

    inlet_free(x->i_trigger);
    inlet_free(x->i_control);
    inlet_free(x->i_sync);
    outlet_free(x->o_loop_sig);
    outlet_free(x->o_control);
    outlet_free(x->o_info);
    for(int i=0; i<4; i++){    
        free(x->motifs_array[i]->_aData);
        free(x->motifs_array[i]->_bData);
        free(x->motifs_array[i]);
    }
    //free(x->motifs_array);
}

void bank_tilde_setup(void){
    bank_tilde_class = class_new(
        gensym("bank~"),
        (t_newmethod)bank_tilde_new,
        (t_method)bank_tilde_free,
        sizeof(t_bank),
        CLASS_DEFAULT,
        A_DEFFLOAT,
        (t_atomtype)0
    );
    
    CLASS_MAINSIGNALIN(bank_tilde_class, t_bank, f);
    class_addmethod(bank_tilde_class, (t_method) bank_dsp      ,gensym("dsp")    , A_CANT, (t_atomtype)0);

    class_addmethod(bank_tilde_class, (t_method) bank_onDelete  ,gensym("delete")   ,(t_atomtype)0 );
    class_addmethod(bank_tilde_class, (t_method) bank_onReset  ,gensym("clean")   ,(t_atomtype)0 );
    class_addmethod(bank_tilde_class, (t_method) bank_onTransportReset   ,gensym("transport_start")   ,(t_atomtype)0 );
    class_addmethod(bank_tilde_class, (t_method) bank_onTransportStop   ,gensym("transport_stop")   ,(t_atomtype)0 );

    class_addmethod(bank_tilde_class, (t_method) bank_onControlAlt ,gensym("on_ctl_alt"), A_DEFFLOAT ,(t_atomtype)0 );
    class_addmethod(bank_tilde_class, (t_method) bank_onControlMain ,gensym("on_ctl_main"), A_DEFFLOAT ,(t_atomtype)0 );
    class_addmethod(bank_tilde_class, (t_method) bank_onOvertakeRecord   ,gensym("overtake_record")   ,(t_atomtype)0 );

    class_addmethod(bank_tilde_class, (t_method) bank_onActivate   ,gensym("activate")   ,(t_atomtype)0 );
    class_addmethod(bank_tilde_class, (t_method) bank_onDeactivate   ,gensym("deactivate")   ,(t_atomtype)0 );
    class_addmethod(bank_tilde_class, (t_method) bank_onUnlatch   ,gensym("unlatch")   ,(t_atomtype)0 );

    class_addmethod(bank_tilde_class, (t_method) bank_onNextSlot   ,gensym("next_slot")   ,(t_atomtype)0 );
    class_addmethod(bank_tilde_class, (t_method) bank_onPrevSlot   ,gensym("prev_slot")   ,(t_atomtype)0 );
    class_addmethod(bank_tilde_class, (t_method) bank_setSyncTick,    gensym("set_sync_tick"),  A_DEFFLOAT, (t_atomtype)0);

    class_addmethod(bank_tilde_class, (t_method) bank_debugInfo     ,gensym("debug_info")     ,(t_atomtype)0 );

    //class_sethelpsymbol(bank_class, gensym("bank_tilde-help"));
}
