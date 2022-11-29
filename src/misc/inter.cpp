
#include "inter.h"

void* inter_work(void* arg){
    t_inter* x = (t_inter*)arg;
        usleep(2000000);
    post("work done");
    outlet_bang(x->o_work_result);
    return NULL;
};

void inter_onWorkStart(t_inter* x, t_floatarg f){
    pthread_t t = x->thread;
    pthread_create(&t, NULL, inter_work, (void*)x);
    post("work started");
}

void* inter_new(t_floatarg out_count){
 
    t_inter* x = (t_inter*)pd_new(inter_class);
    x->o_work_result = (t_outlet*)outlet_new(&x->x_obj, &s_float);

    return (void*)x;
}

void inter_free(t_inter* x){
    
}

void inter_setup(void){
    inter_class = class_new(
        gensym("inter"),
        (t_newmethod)inter_new,
        (t_method)inter_free,
        sizeof(t_inter),
        CLASS_DEFAULT,
        (t_atomtype)0
    );

    class_addfloat(inter_class,(t_method)inter_onWorkStart);
}
