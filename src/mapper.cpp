#include <stdlib.h>
#include <stdarg.h>
#include <string>

extern "C"{
    #include "m_pd.h"
    static t_class *mapper_class;
    typedef struct _mapper{
        t_object    x_obj;
        t_outlet**  o_state;
        int*        respondents;
        int         count;
    } t_mapper;

    void mapper_onInput(t_mapper *x, t_symbol *s, int argc, t_atom *argv);
    void* mapper_new(t_symbol *s, int argc, t_atom *argv);
    void mapper_free(t_mapper* x);
    void mapper_setup(void);
}

void mapper_onInput(t_mapper *x,t_symbol *s, int argc, t_atom *argv){
    if(argc < 2) return;
    if(s == gensym("set")){
        for(int i=0; i<std::min(x->count,argc); i++){
            x->respondents[i] = atom_getint(&argv[i]);
            post("remapped -> [%d]%d",i,x->respondents[i]);
        }
    }
    else if(s == gensym("change")){
        int i = atom_getint(&argv[0]);
        int v = atom_getint(&argv[1]);
        x->respondents[i] = v;
        post("changed -> [%d]%d",i,v);
    }
    else{
        for(int r=0; r<x->count; r++){
            if(x->respondents[r] == atom_getint(&argv[0]))
                outlet_float(x->o_state[r], atom_getint(&argv[1]) > 0 ? 1.f : 0.f);
        }
    }
}

void* mapper_new(t_symbol *s, int argc, t_atom *argv){

    t_mapper* x = (t_mapper*)pd_new(mapper_class);
    x->count = argc;
    //number of outlets dependant on argument count
    x->respondents = (int*)calloc(argc,sizeof(int));
    x->o_state = (t_outlet**)malloc(sizeof(t_outlet*)*argc);
    for(int i=0; i<x->count; i++){
        x->respondents[i] = atom_getint(&argv[i]);
        x->o_state[i] = (t_outlet*)outlet_new(&x->x_obj, &s_float);
    }
    for(int i=0; i<x->count; i++)
        post("mapping for [%d]",x->respondents[i]);

    return (void*)x;
}

void mapper_free(t_mapper* x){
    for(int i=0; i<x->count; i++)
        outlet_free(x->o_state[i]);
    free(x->o_state);
    free(x->respondents);
}

void mapper_setup(void){
    mapper_class = class_new(
        gensym("mapper"),
        (t_newmethod)mapper_new,
        (t_method)mapper_free,
        sizeof(t_mapper),
        CLASS_DEFAULT,
        A_GIMME,
        (t_atomtype)0
    );

    class_addanything(mapper_class,(t_method)mapper_onInput);
}
