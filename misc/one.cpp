#include "one.hpp"

void cpppost(){
    std::string st = "hello from cpp";
    post("%s",st.c_str());
}

std::string complexcpp(std::string s){
    std::string ss = s;
    ss += "[parsed from cpp]";
    return ss;
}

void one_bang(t_one *x){
    post("%s","hello");
    cpppost();
    post("%s", complexcpp("yo").c_str());
}
void* one_new(void){
    t_one* x = (t_one*)pd_new(one_class);
    x->o_out = (t_outlet*)outlet_new(&x->x_obj, &s_float);
    return (void*)x;
}
void one_free(t_one* x){

}

void one_setup(void){
    one_class = class_new(
        gensym("one"),
        (t_newmethod)one_new,
        (t_method)one_free,
        sizeof(t_one),
        CLASS_DEFAULT,
        (t_atomtype)0
    );
    class_addbang(one_class, one_bang);
}
