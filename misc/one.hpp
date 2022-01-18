#pragma once
#include <string>

extern "C"{
#include "m_pd.h"
}

void cpppost();
std::string complexcpp(std::string s);

extern "C" {
static t_class *one_class;
typedef struct _one{
    t_object    x_obj;
    t_outlet*   o_out;
} t_one;

void one_bang(t_one *x);
void* one_new(void);
void one_free(t_one* x);
void one_setup(void);
}