#pragma once

#include "m_pd.h"

#define VER "S/W/R/0"
// void cpppost();
// std::string complexcpp(std::string s);
#define MOTIF_BUF_SIZE 4194304 //Total mem = banks * 2 * BUF_SIZE(4MB = 1024*1024*4)

//convenience checks
#define DEBOUNCE_TIME 20 // ~40ms
#define HOLD_TIME 1500 //sync ticks ~= 2s
#define REFILL_CHUNK 64
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
        t_outlet*   o_control;
        t_outlet*   o_info; 

        t_atom      a_info_list[5];
        
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

        bool gate; //if play button let go, stop sound
        bool onetime; //dont loop
        bool synced; //honour quantize ticks

        t_motif**   motifs_array;
        int         motifs_array_count;
        t_motif*    active_motif_ptr;
        int         active_motif_idx;

        bool        stateTrigger; //main ctrl state of button
        bool        stateCAlt; //alt ctrl state of button
        bool        stateCShift; 
        long        holdCounter;// + for main ctrl, - for alt ctrl
        long        debounceCounter;

        int         work_type; //1 refill, -1 unfill, 2 fetch
        long        work_data;//count of data work task is still working on
    } t_bank;

    void bank_q(t_bank* x);
    void bank_postStateUpdate(t_bank* x);
    void bank_postQuanUpdate(t_bank* x);

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