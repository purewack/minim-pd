#pragma once
#include <string>
#include "m_pd.h"

// void cpppost();
// std::string complexcpp(std::string s);
#define MOTIF_BUF_SIZE 4194304 //4MB = 1024*1024*4

//sync tick = n64 block boundary timing
//quan tick = n*sync ticks
typedef struct _motif{
    int state;
    int n_state;
    bool gate; //if play button let go, stop sound
    bool onetime; //dont loop
    bool synced; //honour quantize ticks

    int isDubbing;
    int isLong; //if over buf size, will require disk file for stream from and to
    
    int len_syncs; //number of sync ticks loop spans
    int pos_syncs; //playhead position in sync ticks
    int len_spl;// loop spans in absolute samples
    int pos_spl;// playhead position normalised to len_spl
    t_float pos_ratio; //playhead position normalised to len_syncs 
    int last_sync;//sync time of last tick

    //buffer management
    int dataHead;
    t_sample *_aData, *_bData, *_data, *_ndata;
} t_motif;

enum _motif_state{
    m_clear = -1,
    m_stop  = 0,
    m_base  = 1,
    m_play  = 2,
    m_dub   = 3,
};

extern "C"{
   
    #include <pthread.h>
    #include <unistd.h>
    #include <pthread.h>

    static t_class *bank_class;
    typedef struct _bank{
        t_object    x_obj;
        int         id;

        t_float     f;
        t_inlet*    i_tick_stats;
        t_outlet*   o_loop_sig;
        t_outlet*   o_m_state;
        t_outlet*   o_sync; 

        t_atom      a_m_stats[5];
        
        int         is_active;// if not, does not take any input, still processes sound
        int         tick_current;//current tick timer, all values stem from
        int         tick_duration;// quantize Tick
        int         tick_start;//when the last tick started
        int         tick_next;//when the next tick will happen
        int         tick_action_pending;//if action is pending on tick
        int         tick_action_when;//when action should be exec
        int         tick_action_nstate;//next motif state on action
        float       last_sync;//last time of sync tick

        t_motif**   motifs_array;
        int         motifs_array_count;
        t_motif*    active_motif_ptr;
        int         active_motif_idx;

        bool        stateBtnStart; //top ctrl
        bool        stateBtnStop; //bot ctrl
        long        holdCounter;// + for top ctrl, - for bot ctrl

        pthread_t   worker_thread;
        int         worker_thread_alive;
        int         work_type; //1 refill, -1 unfill, 2 fetch 
        int         work_type_inthread;//internal buffer flag for thread
        pthread_cond_t  cond_work;
        pthread_mutex_t mtx_work; 
    } t_bank;

    void* bank_worker_thread(void* arg);
    void bank_outlet_sync(t_bank* x, int msync);
    void bank_outlet_mstats(t_bank* x, t_float ticklen);
    void bank_q(t_bank* x, int now);
    void bank_onActivate(t_bank* x);
    void bank_onDeactivate(t_bank* x);
    void bank_onTickLen(t_bank* x, t_floatarg t);
    void bank_onGetPos(t_bank* x);
    void bank_onNextSlot(t_bank* x);
    void bank_onPrevSlot(t_bank* x);
    void bank_onLaunch(t_bank* x);
    void bank_onStateStopOn(t_bank* x);
    void bank_onTransportReset(t_bank* x);
    void bank_onReset(t_bank* x);
    void bank_clear_motif(t_motif* m);
    void bank_motif_toStart(t_motif* m);
    void bank_motif_swapStreams(t_motif* m);
    void* bank_new(t_floatarg f);
    void bank_free(t_bank* x);
    void bank_setup(void);
    void bank_dsp(t_bank *x, t_signal **sp);
    t_int* bank_perform(t_int *w);
}