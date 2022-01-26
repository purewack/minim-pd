#include "bank.h"

void bank_dsp(t_bank *x, t_signal **sp)
{
  dsp_add(bank_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
  //dsp_add(bank_perform, 1, x);
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
    int dataHeadMod = m->isLong ? MOTIF_BUF_SIZE : m->len_spl;

    switch(m->state){
        case _motif_state::m_base:
            while (n--) {
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
                bank_outlet_mstats(x, m->len_syncs);
                x->tick_action_nstate = _motif_state::m_play;
                bank_q(x, 0); 
            }
        break;
           
        case _motif_state::m_dub:
        case _motif_state::m_play:
            while (n--) {
                ins = *in++;
                *out++ = m->_data[m->dataHead];
                m->_data[m->dataHead] += (ins * dub);
                m->pos_spl = (m->pos_spl+1) % m->len_spl;
                m->dataHead = (m->dataHead+1) % m->len_spl;
            }

            m->pos_syncs += 1;
            if(m->pos_syncs >= m->len_syncs){
                m->pos_syncs = 0;
                m->last_sync = x->tick_current;
                msync = x->tick_current;
                outlet_float(x->o_sync, x->tick_current);
            }
        break;

        default:
            while(n--) *out++ = 0;
        break;
    }

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
                x->work_type = 1;
            }
            else if(m->state == _motif_state::m_dub && x->tick_action_nstate == _motif_state::m_stop){
                //cancel
                m->isDubbing = 0;
                bank_motif_swapStreams(m);
                x->work_type = 1;
                x->tick_action_nstate = _motif_state::m_play;
            }

            m->state = x->tick_action_nstate;
            x->tick_action_pending = 0;
        }
    }

    if(x->tick_duration > 0 && x->tick_current >= x->tick_start+x->tick_duration)
    {
        x->tick_start = x->tick_current;
        x->tick_next = x->tick_start + x->tick_duration;
        x->last_sync = x->tick_current;
        // bank_outlet_sync(x, msync);
        if(msync != x->tick_current)
        outlet_float(x->o_sync, float(x->tick_current * -1));
    }

    if(x->work_type){
        if(pthread_mutex_trylock(&x->mtx_work)){
            x->work_type_inthread = x->work_type;
            x->work_type = 0;
            pthread_cond_signal(&x->cond_work);
            pthread_mutex_unlock(&x->mtx_work);
        }
    }
    
    return (w+5);
    // return (w+2);
}

void bank_outlet_mstats(t_bank* x, t_float ticklen){
    int c = 3;
    SETFLOAT(x->a_m_stats   ,x->active_motif_ptr->state);
    SETFLOAT(x->a_m_stats+1 ,x->tick_action_pending ? x->tick_action_nstate : 0);
    SETFLOAT(x->a_m_stats+2 ,x->tick_action_pending ? x->tick_action_when : 0);
    
    SETFLOAT(x->a_m_stats+c, ticklen);
    if(ticklen > 0) c+=1;
    //  SETFLOAT(x->a_m_stats,x->active_motif_ptr->len_syncs);
    //  SETFLOAT(x->a_m_stats,x->active_motif_ptr->pos_syncs);
    outlet_list(x->o_m_state, &s_list, c, x->a_m_stats);
}

// void bank_outlet_sync(t_bank* x, int mstats){
//     //  int c = 1;
     
//     //  SETFLOAT(x->a_sync, x->tick_current);
//     //  SETFLOAT(x->a_sync+1, x->tick_next);
    
//     // //  if(mstats){
//     // //      c += 1;
//     // //      t_float m_next = x->tick_start + x->active_motif_ptr->len_syncs;
//     // //      SETFLOAT(x->a_sync+2, m_next);
//     // //  }

//     // outlet_list(x->o_sync, &s_list, c, x->a_sync);
// }

void bank_onReset(t_bank* x){
    x->tick_current = 0;
    x->tick_duration = 0;
    x->tick_next = 0;
    x->tick_start = 0;
    x->tick_action_pending = 0;
    x->tick_action_nstate = 0;
    x->tick_action_when = 0;
    bank_outlet_mstats(x,0);
    // bank_outlet_sync(x,0);
    bank_clear_motif(x->active_motif_ptr);
    post("reset: %d",x->active_motif_idx);
}

void bank_onTransportReset(t_bank* x){
    x->tick_start = 0;
    x->tick_current = 0;
    x->tick_action_when = 0;
    x->tick_next = x->tick_duration;
    t_motif* m = x->active_motif_ptr;
    if(m->state != _motif_state::m_clear){
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
void bank_onTickLen(t_bank* x, t_floatarg t){
    if(x->tick_duration > 0) return;

    int m_len_spl = x->active_motif_ptr->len_spl;
    x->tick_duration = t;
    x->tick_current = 0;
    x->tick_next = t;
    if(x->active_motif_ptr->state == _motif_state::m_base){
        x->active_motif_ptr->state = _motif_state::m_play;
        x->active_motif_ptr->pos_syncs = 0;
        x->active_motif_ptr->len_syncs = x->tick_duration;
        x->active_motif_ptr->len_spl = x->tick_duration * 64;
    }
    post("bank %d tick len: %f tick len spl %d (%d pre)", x->id, t, x->active_motif_ptr->len_spl, m_len_spl);
    // bank_outlet_sync(x,1);
}

void bank_onGetPos(t_bank* x){
    if(! x->is_active) return;
    float p = x->active_motif_ptr->len_syncs ? float(x->active_motif_ptr->pos_syncs) / float(x->active_motif_ptr->len_syncs) : 0.0f;
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
    post("%d current slot: %d",x->id, x->active_motif_idx);
}

void bank_onPrevSlot(t_bank* x){
    if(! x->is_active) return;
    
    if(x->active_motif_idx > 0) 
        x->active_motif_idx -= 1;

    x->active_motif_ptr = x->motifs_array[x->active_motif_idx];
    post("%d current slot: %d", x->id, x->active_motif_idx);
}

void bank_q(t_bank* x, int now){
    if(!now)
        x->tick_action_when = x->tick_start+x->tick_duration;
    else
        x->tick_action_when = 0;
    x->tick_action_pending = 1;
    bank_outlet_mstats(x,0);
}

void bank_onLaunch(t_bank* x){
    if(x->is_active == 0) return;

    switch (x->active_motif_ptr->state)
    {
    case _motif_state::m_clear:
        x->tick_action_nstate = _motif_state::m_base;
        bank_q(x, 0);
    break;

    case _motif_state::m_base:
        if(x->tick_duration < 0){
            t_float t = x->tick_duration * -1.0;
            bank_outlet_mstats(x, t);
            post("%d set new tick len %f", x->id, t);
        }
    case _motif_state::m_stop:
        x->tick_action_nstate = _motif_state::m_play;
        bank_q(x, 0);
    break;

    case _motif_state::m_play:
        x->tick_action_nstate = _motif_state::m_dub;
        bank_q(x, 1); 
    break;

    case _motif_state::m_dub:
        x->tick_action_nstate = _motif_state::m_play;
        bank_q(x, 1); 
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
        bank_q(x,0);
    break;

    case _motif_state::m_dub:
    case _motif_state::m_play:
        x->tick_action_nstate = _motif_state::m_stop;
        bank_q(x, x->active_motif_ptr->isDubbing);
    break;
    }

    bank_outlet_mstats(x,0);
    post("%d current motif state: %d", x->id, x->tick_action_nstate);
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
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <filesystem>

void* bank_new(t_floatarg id){
    t_bank* x = (t_bank*)pd_new(bank_class);
    //f signal in
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
    x->work_type = 0;
    x->work_type_inthread = 0;

    post("new bank with id: %f", x->id);

    x->worker_thread_alive = 1;
    pthread_create(&x->worker_thread, NULL, bank_worker_thread, x);


    std::string temploc;
    temploc = std::filesystem::temp_directory_path().string();

    std::stringstream  fname;
    fname << temploc << "tfile" << x->id << ".txt";
    std::ofstream myfile;
    myfile.open (fname.str().c_str());
    myfile << "Hello.\n";
    myfile << x->id << "\n";
    myfile.close();

    std::ifstream readfile;
    readfile.open(fname.str().c_str());
    std::string ss;
    readfile >> ss;
    readfile.close();

    post("read: %s",ss.c_str());
    
    return (void*)x;
}

void bank_free(t_bank* x){

    pthread_mutex_lock(&x->mtx_work);
    x->work_type = 0;
    x->worker_thread_alive = 0;
    pthread_mutex_unlock(&x->mtx_work);
    pthread_join(x->worker_thread, NULL);

    inlet_free(x->i_tick_stats);
    outlet_free(x->o_loop_sig);
    outlet_free(x->o_m_state);
    outlet_free(x->o_sync);
    for(int i=0; i<4; i++){    
        free(x->motifs_array[i]->_aData);
        free(x->motifs_array[i]->_bData);
        free(x->motifs_array[i]);
    }
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

void* bank_worker_thread(void* arg){
    t_bank* x = (t_bank*)arg;

    post("new thread for bank id: %d", x->id);

    while(x->worker_thread_alive){
        pthread_mutex_lock(&x->mtx_work);
        pthread_cond_wait(&x->cond_work, &x->mtx_work);
        
        t_motif* m = x->active_motif_ptr;
        if(x->work_type_inthread == 1){
            for(int i=0; i<m->len_spl; i++){
                m->_ndata[i] = m->_data[i];
            }
            x->work_type_inthread = 0;
            post("refilled %d.spls for motif %d in bank %d", m->len_spl, x->active_motif_idx, x->id);
        }

        pthread_mutex_unlock(&x->mtx_work);
    }

    post("ending thread for bank id: %d", x->id);
    return NULL;
}