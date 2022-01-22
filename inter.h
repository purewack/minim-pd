#pragma once

extern "C"{
    #include "m_pd.h"
    #include <pthread.h>
    #include <unistd.h>
    static t_class *inter_class;
    typedef struct _inter{
        t_object    x_obj;
        t_inlet*    i_work_start;
        t_outlet*   o_work_result;
        pthread_t   thread;
    } t_inter;

    void* inter_work(void* arg);
    void inter_onWorkStart(t_inter* x, t_floatarg f);
    void* inter_new(t_floatarg f);
    void inter_free(t_inter* x);
    void inter_setup(void);
}