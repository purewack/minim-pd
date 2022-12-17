
#include <cmath>

extern "C" {
    #include "m_pd.h"
    #include <unistd.h>
    #include <stdlib.h>
}

#define VER "S/W/R/0"
// void cpppost();
// std::string complexcpp(std::string s);
#define MOTIF_BUF_SIZE 4194304 //Total mem = banks * 2 * BUF_SIZE(4MB = 1024*1024*4)

//convenience checks
#define DEBOUNCE_TIME 20 // ~40ms
#define HOLD_TIME 1500 //sync ticks ~= 2s
#define REFILL_CHUNK 64

#define BAR_BEATS 24
typedef struct _motif{
    int state;
    int n_state;

    int isDubbing;
    int isLong; //if over buf size, will require disk file for stream from and to
    int isDone;

    int len_syncs; //number of sync ticks loop spans
    int pos_syncs; //playhead position in sync ticks
    int len_spl;// loop spans in absolute samples
    int pos_spl;// playhead position normalised to len_spl
    t_float pos_ratio; //playhead position normalised to len_syncs 
    int last_sync;//sync time of last tick

    //buffer management
    t_sample *_aData, *_bData, *_data, *_ndata;
} t_motif;

enum _motif_state{
    m_clear = -1,
    m_stop  = 0,
    m_base  = 1,
    m_play  = 2,
    m_dub   = 3,
};

enum _motif_work_type{
    REFILL = (1<<0)
};

extern "C"{

    static t_class *bank_tilde_class;
    typedef struct _bank{
        t_object    x_obj;
        int         id;

        t_float     f;
        t_inlet*    i_trigger;
        t_inlet*    i_control;
        t_inlet*    i_sync;
        t_outlet*   o_loop_sig;
        t_outlet*   o_loop_pos;
        t_outlet*   o_info_status; 
        t_outlet*   o_info_sync;

        t_atom      a_status_list[3];
        //a_status_list:
        //  bank_idx
        //  status_peding
        //  status_current

        t_atom      a_sync_list[3];
        //a_sync_list:
        //  tick_len
        //  bar_beats
        //  len_syncs

        //sync tick = b64 boundary timing, 1 DSP loop
        //quan tick = L*sync ticks
        bool        isActive;// if not, does not take any input, still processes sound
        int         populatedCount;
        bool        hasQuantick;
        int         tick_current;//current sync tick timer, all values stem from
        int         tick_duration;// quantize Tick
        int         tick_start;//when the last tick started
        int         tick_next;//when the next tick will happen
        int         tick_action_pending;//if action is pending on tick
        int         tick_action_when;//when action should be exec
        int         tick_action_nstate;//next motif state on action
        int         when_base;//last time of sync tick
        int         sync_cbeat; //current bar beat
        int         sync_beats; // number of beats in bar
        int         quan_beats;
        bool        show_ratio;

        bool gate; //if play button let go, stop sound
        bool onetime; //dont loop
        bool synced; //honour quantize ticks

        t_motif**   motifs_array;
        int         motifs_array_count;
        t_motif*    active_motif_ptr;
        int         active_motif_idx;

        bool        stateTrigger; //main ctrl state of button
        bool        stateCAlt; //alt ctrl state of button
        long        holdCounter;// + for main ctrl, - for alt ctrl
        long        debounceCounter;

        int         work_type; //1 refill, -1 unfill, 2 fetch
        long        work_data;//count of data work task is still working on
    } t_bank;

    void bank_q(t_bank* x);
    void bank_postStateUpdate(t_bank* x);
    void bank_postSyncUpdate(t_bank* x, int len = 0);

    void bank_onActivate(t_bank* x);
    void bank_onDeactivate(t_bank* x);
    void bank_setSyncTick(t_bank* x, t_floatarg t);
    void bank_debugInfo(t_bank* x);
    void bank_onNextSlot(t_bank* x);
    void bank_onPrevSlot(t_bank* x);
    void bank_onLaunch(t_bank* x);
    void bank_onTransportReset(t_bank* x);
    void bank_onReset(t_bank* x);
    void bank_onDelete(t_bank* x);
    void bank_onOvertakeRecord(t_bank* x);
    
    void bank_onTriggerOn(t_bank* x);
    void bank_onTriggerOff(t_bank* x);
    void bank_onControlAltOn(t_bank* x);
    void bank_onControlAltOff(t_bank* x);
    void bank_safeSlotChange(t_bank* x);
    void bank_clear_motif(t_motif* m);
    void bank_motif_toStart(t_motif* m);
    void bank_motif_swapStreams(t_motif* m);
    
    void bank_dsp(t_bank *x, t_signal **sp);
    t_int* bank_perform(t_int *w);

    void* bank_tilde_new(t_floatarg f);
    void bank_tilde_free(t_bank* x);
    void bank_tilde_setup(void);
    
    void* bank_worker_thread(void* arg);
}


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

    if(x->show_ratio){
        if(m->len_syncs) m->pos_ratio = float(m->pos_syncs) / float(m->len_syncs);
        outlet_float(x->o_loop_pos,m->pos_ratio);
    }
    //n-state machine
    if(x->tick_action_pending){
        if(x->tick_current >= x->tick_action_when){
            bool doPost = true;
            post("     NSTATE[%d] @%d -> %d [%d](%d)", x->id, x->tick_current, x->tick_action_when,m->state, x->tick_action_nstate);
            //if state play and n_state stop => reset pos etc, next state machine essentially 
            x->tick_action_when = 0;

            if(m->state == _motif_state::m_clear || x->tick_action_nstate == _motif_state::m_base){
                x->when_base = x->tick_current;
                post("bank[%d] when_base %d",x->id, x->when_base);
            }

            if((m->state == _motif_state::m_clear || m->state == _motif_state::m_stop) && x->tick_action_nstate == _motif_state::m_play)
            { 
                bank_motif_toStart(m);
            }
            
            else if(m->state == _motif_state::m_base && x->tick_action_nstate == _motif_state::m_play ){  
                if(!x->hasQuantick){
                    int suggested_bar_beats = 24;
                    //post changes to sync listeners to update banks with new project quan length
                    auto dd = x->tick_duration * -1;
                    auto ll = int(dd / suggested_bar_beats);
                    auto delta = dd - (ll*suggested_bar_beats);
                    //dd = ll * BAR_BEATS;
                    x->active_motif_ptr->len_syncs = ll * suggested_bar_beats;
                    x->active_motif_ptr->len_spl = x->active_motif_ptr->len_syncs * 64;
                    x->sync_beats = suggested_bar_beats;

                    post("bank[%d] len compensation error %dq64 (%fms)",x->id,delta,float(64*(delta))/44.1f);
                    bank_postSyncUpdate(x, ll);
                }
                else{
                    x->active_motif_ptr->len_syncs = x->tick_current - x->when_base;
                    x->active_motif_ptr->len_spl = x->active_motif_ptr->len_syncs * 64;
                    post("perform[%d]@%d nstate ticklen:%d spllen:%d",x->id, x->tick_current, x->active_motif_ptr->len_syncs, x->active_motif_ptr->len_spl);
                    bank_postSyncUpdate(x);
                }
            }

            else if((m->state == _motif_state::m_play || m->state == _motif_state::m_stop) && x->tick_action_nstate == _motif_state::m_dub){
                //start dub
                if(m->state == _motif_state::m_stop){ 
                    bank_motif_toStart(m);
                }
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
            }
           
            m->state = x->tick_action_nstate;
            x->tick_action_pending = 0;
            //post("pending action done bank %d",x->id);
            if(doPost)
            bank_postStateUpdate(x);

            post("     NSTATE[%d] done", x->id);
        }
    }

    //next sync tick calculation
    if(x->tick_duration > 0 && x->tick_current >= x->tick_start+x->tick_duration)
    {
        //if(bank_activeMotifIsRunning(x)){
            x->sync_cbeat = (x->sync_cbeat+1) % x->sync_beats;
            if(!x->show_ratio)
            outlet_float(x->o_loop_pos,x->sync_cbeat);
        //}
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


void bank_onSetQuanBeats(t_bank* x, t_floatarg t){
    if(t > 0)
        x->quan_beats = int(t);
}

void bank_setSyncTick(t_bank* x, t_floatarg t){
    if(x->hasQuantick) return;
    if(t <= 0) return;
    
    //banks with uninitiated quan tick data go through this routine once
    //normalize recorded sample len to sync len

    bank_onTransportReset(x);
    x->sync_beats = BAR_BEATS;
    x->hasQuantick = true;
    x->tick_duration = t;
    x->tick_next = t;
    post("setSyncTick_bank:%d @%d has new tick len: %f tick len spl %d", x->id,x->tick_current, t, x->active_motif_ptr->len_spl);
    // bank_outlet_sync(x,1);
}


void bank_postStateUpdate(t_bank* x){
    SETFLOAT(x->a_status_list+0, x->active_motif_idx);
    SETFLOAT(x->a_status_list+1, x->tick_action_nstate);
    SETFLOAT(x->a_status_list+2, x->active_motif_ptr->state);
    outlet_list(x->o_info_status, &s_list, 3, x->a_status_list);
}

void bank_postSyncUpdate(t_bank* x, int len){
    SETFLOAT(x->a_sync_list+0, len != 0 ? len : x->tick_duration);
    SETFLOAT(x->a_sync_list+1, x->sync_beats);
    SETFLOAT(x->a_sync_list+2, x->active_motif_ptr->len_syncs);
    outlet_list(x->o_info_sync, &s_list, 3, x->a_sync_list);
}

//que up action time aligned if alignment data exists
void bank_q(t_bank* x){
    if(x->synced && x->tick_duration) {//if not gated then schedule on next tick beat 0
        int ww = x->tick_start + x->tick_duration * (x->quan_beats - (x->sync_cbeat%x->quan_beats));
        x->tick_action_when = ww;
    }
    else //else execute on next block cycle (q64)
        x->tick_action_when = 0;
    x->tick_action_pending = 1; //signal action pending
    bank_postStateUpdate(x);
}


void bank_debugInfo(t_bank* x){
    float p = x->active_motif_ptr->len_syncs ? float(x->active_motif_ptr->pos_syncs) / float(x->active_motif_ptr->len_syncs) : 0.0f;
    post("=======bank[id%d]=====", x->id);
    post("active slot [slot%d]",x->active_motif_idx);
    post("posPer: %f , posA:%d, len : %d",p, x->active_motif_ptr->pos_syncs, x->active_motif_ptr->len_syncs);
    post("bank %d tick len: %d, tick %d tick_action:%d", x->id, x->tick_duration, x->tick_current, x->tick_action_when);
    post("state %d", x->active_motif_ptr->state);
    post("gate %d", x->gate);
    post("sync %d", x->synced);
    post("one %d", x->onetime);
    post("Qb:%d  %d/%d", x->quan_beats, x->sync_cbeat, x->sync_beats);
}

void bank_debugInfoShort(t_bank* x){
    post("=======bank[id%d]=====", x->id);
    post("[%d] posA:%d / len : %d / tick len: %d",x->active_motif_ptr->state, x->active_motif_ptr->pos_syncs, x->active_motif_ptr->len_syncs, x->tick_duration);
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


void bank_onUndo(t_bank* x){
    if(bank_activeMotifIsDub(x)){
        x->tick_action_nstate = _motif_state::m_stop;
        auto s = x->synced;
        x->synced = bank_activeMotifIsDub(x) ? false : x->synced;
        bank_q(x);
        x->synced = s;
    }
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
        post("@%d base rec bank %d [%s]",x->tick_current, x->id, x->synced ? "SYNC" : "FREE");
        return;
    }

    //finish base rec
    if(bank_activeMotifIsBase(x)){
        //if start button held, change mode to loop
        //[4]
        //[5]
        if(!x->stateCAlt){
             
            //loop        
            x->gate = false;
            x->onetime = false;
            x->synced = bank_hasQuanTick(x) ? true : false;
            x->tick_action_nstate = _motif_state::m_play;
            bank_q(x);
            x->synced = true;

            post("onTriggerOn@%d - Base sync for %d",x->tick_current,x->tick_action_when);
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
        post("onTriggerOn@%d base rec end bank %d [%s]",x->tick_current, x->id, x->synced ? "SYNC" : "FREE");
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
        bank_onUndo(x);
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
    //outlet_float(x->o_control, x->stateCAlt);
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

void bank_onShowLoopRatio(t_bank*x, t_floatarg f){
    if(f == 0) x->show_ratio = false;
    else x->show_ratio = true;
}



void bank_onDelete(t_bank* x){
    x->gate = false;
    x->synced = false;
    x->onetime = false;
    // x->tick_duration = 0;
    x->tick_action_pending = 0;
    x->tick_action_nstate = _motif_state::m_clear;
    // x->hasQuantick = false;
    //bank_onTransportReset(x);
    bank_clear_motif(x->active_motif_ptr);
    x->populatedCount--;
    bank_postStateUpdate(x);
    post("Deleted motif %d in bank %d",x->active_motif_idx, x->id);
}

void bank_onReset(t_bank* x){
    x->stateCAlt = false;
    x->gate = false;
    x->synced = false;
    x->onetime = false;
    x->show_ratio = true;
    x->tick_duration = 0;
    x->tick_action_pending = 0;
    x->tick_action_nstate = _motif_state::m_clear;
    x->hasQuantick = false;
    x->quan_beats = BAR_BEATS;
    x->sync_beats = BAR_BEATS;
    x->sync_cbeat = 0;
    bank_onTransportReset(x);
    bank_onDelete(x);
    x->populatedCount = 0;
    for(int m=0; m<x->motifs_array_count; m++)
        bank_clear_motif(x->motifs_array[m]);
        
    post("Cleared all motifs in bank %d", x->id);
}

void bank_onTransportReset(t_bank* x){
    post("@%d bank [%d] transport reset",x->tick_current, x->id);
    x->tick_start = 0;
    x->tick_current = 0;
    x->tick_action_when = 0;
    x->tick_next = x->tick_duration;
    x->sync_cbeat = 0;
    t_motif* m = x->active_motif_ptr;
    if(!bank_activeMotifIsClear(x)){
        m->state = _motif_state::m_stop;
        bank_motif_toStart(m);
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
    x->o_loop_pos = outlet_new(&x->x_obj,&s_float);
    x->o_info_status = outlet_new(&x->x_obj,&s_list);
    x->o_info_sync = outlet_new(&x->x_obj, &s_list);
    

    x->motifs_array = (t_motif**)malloc(4 * sizeof(t_motif*));
    for(int i=0; i<4; i++){
        x->motifs_array[i] = (t_motif*)malloc(sizeof(t_motif));
        x->motifs_array[i]->_aData = (t_sample*)malloc(sizeof(t_sample) * MOTIF_BUF_SIZE);
        x->motifs_array[i]->_bData = (t_sample*)malloc(sizeof(t_sample) * MOTIF_BUF_SIZE);
    }

    x->id = (int)id;
    x->motifs_array_count = 4;
    x->active_motif_idx = 0;
    x->active_motif_ptr = x->motifs_array[x->active_motif_idx];
    x->isActive = 1;
    bank_onReset(x);

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
    outlet_free(x->o_loop_pos);
    outlet_free(x->o_info_status);
    outlet_free(x->o_info_sync);
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
    class_addmethod(bank_tilde_class, (t_method) bank_onUndo ,gensym("undo") ,(t_atomtype)0 );
    class_addmethod(bank_tilde_class, (t_method) bank_onTransportReset   ,gensym("transport_start")   ,(t_atomtype)0 );
    class_addmethod(bank_tilde_class, (t_method) bank_onTransportStop   ,gensym("transport_stop")   ,(t_atomtype)0 );
    class_addmethod(bank_tilde_class, (t_method) bank_onSetQuanBeats ,gensym("quan_beats"), A_DEFFLOAT ,(t_atomtype)0 );
    class_addmethod(bank_tilde_class, (t_method) bank_onShowLoopRatio ,gensym("loop_ratio"), A_DEFFLOAT ,(t_atomtype)0 );

    class_addmethod(bank_tilde_class, (t_method) bank_onControlAlt ,gensym("on_ctl_alt"), A_DEFFLOAT ,(t_atomtype)0 );
    class_addmethod(bank_tilde_class, (t_method) bank_onControlMain ,gensym("on_ctl_main"), A_DEFFLOAT ,(t_atomtype)0 );
    class_addmethod(bank_tilde_class, (t_method) bank_onOvertakeRecord   ,gensym("overtake_record")   ,(t_atomtype)0 );

    class_addmethod(bank_tilde_class, (t_method) bank_onActivate   ,gensym("activate")   ,(t_atomtype)0 );
    class_addmethod(bank_tilde_class, (t_method) bank_onDeactivate   ,gensym("deactivate")   ,(t_atomtype)0 );

    class_addmethod(bank_tilde_class, (t_method) bank_onNextSlot   ,gensym("next_slot")   ,(t_atomtype)0 );
    class_addmethod(bank_tilde_class, (t_method) bank_onPrevSlot   ,gensym("prev_slot")   ,(t_atomtype)0 );
    class_addmethod(bank_tilde_class, (t_method) bank_setSyncTick,    gensym("set_sync_tick"),  A_DEFFLOAT, (t_atomtype)0);

    class_addmethod(bank_tilde_class, (t_method) bank_debugInfo     ,gensym("debug_info")     ,(t_atomtype)0 );
    class_addmethod(bank_tilde_class, (t_method) bank_debugInfoShort     ,gensym("dbg")     ,(t_atomtype)0 );

    //class_sethelpsymbol(bank_class, gensym("bank_tilde-help"));
}
