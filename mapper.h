#pragma once

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