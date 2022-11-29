
extern "C" {
#include "../m_pd.h"
#include <stdlib.h>

static t_class *oled_class;
typedef struct _oled{
    t_object    x_obj;
    t_outlet*   o_midi;
    int         context;
    t_atom*     table_bytes;
    int         table_idx;
    char*       str;
} t_oled;

void* oled_new(t_floatarg f);
void oled_free(t_oled* x);
void oled_setup(void);
void oled_onCommand(t_oled *x, t_symbol *s, int argc, t_atom *argv);

}


void oled_fresh(t_oled *oled){
    SETFLOAT(oled->table_bytes+oled->table_idx,(float)(0xf0));
    SETFLOAT(oled->table_bytes+oled->table_idx+1,(float)('G'));
    SETFLOAT(oled->table_bytes+oled->table_idx+2,(float)(oled->context));
    oled->table_idx = 3;
}

void oled_onCommand(t_oled *oled, t_symbol *s, int argc, t_atom *argv){

    if(s == gensym("send")){
        SETFLOAT(oled->table_bytes+oled->table_idx,(float)('u'));
        SETFLOAT(oled->table_bytes+oled->table_idx+1,(float)(0xf7));
        oled->table_idx+=2;
        outlet_list(oled->o_midi,&s_list,oled->table_idx,oled->table_bytes);
        oled_fresh(oled);
    }
    else if(s == gensym("clear")){
        SETFLOAT(oled->table_bytes+oled->table_idx,(float)('c'));
        SETFLOAT(oled->table_bytes+oled->table_idx+1,(float)('I'));
        SETFLOAT(oled->table_bytes+oled->table_idx+2,(float)(0));
        oled->table_idx+=3;
    }
    else if(s == gensym("xor")){
        SETFLOAT(oled->table_bytes+oled->table_idx,(float)('I'));
        SETFLOAT(oled->table_bytes+oled->table_idx+1,atom_getfloat(&argv[0]));
        oled->table_idx+=2;
    }
    else if(s == gensym("scale") || s == gensym("S")){
        SETFLOAT(oled->table_bytes+oled->table_idx,(float)('S'));
        SETFLOAT(oled->table_bytes+oled->table_idx+1,atom_getfloat(&argv[0]));
        oled->table_idx+=2;
    }

    else if((s == gensym("fill") || s == gensym("f")) && argc == 4){
        SETFLOAT(oled->table_bytes+oled->table_idx,(float)('r'));
        SETFLOAT(oled->table_bytes+oled->table_idx+1,atom_getfloat(&argv[0]));
        SETFLOAT(oled->table_bytes+oled->table_idx+2,atom_getfloat(&argv[1]));
        SETFLOAT(oled->table_bytes+oled->table_idx+3,atom_getfloat(&argv[2]));
        SETFLOAT(oled->table_bytes+oled->table_idx+4,atom_getfloat(&argv[3]));
        SETFLOAT(oled->table_bytes+oled->table_idx+5,float(1));
        oled->table_idx+=6;
    }  
    else if((s == gensym("rect") || s == gensym("r")) && argc == 4){
        SETFLOAT(oled->table_bytes+oled->table_idx,(float)('r'));
        SETFLOAT(oled->table_bytes+oled->table_idx+1,atom_getfloat(&argv[0]));
        SETFLOAT(oled->table_bytes+oled->table_idx+2,atom_getfloat(&argv[1]));
        SETFLOAT(oled->table_bytes+oled->table_idx+3,atom_getfloat(&argv[2]));
        SETFLOAT(oled->table_bytes+oled->table_idx+4,atom_getfloat(&argv[3]));
        SETFLOAT(oled->table_bytes+oled->table_idx+5,float(0));
        oled->table_idx+=6;
    } 
    else if((s == gensym("box") || s == gensym("b")) && argc == 5){
        SETFLOAT(oled->table_bytes+oled->table_idx,(float)('r'));
        SETFLOAT(oled->table_bytes+oled->table_idx+1,atom_getfloat(&argv[0]));
        SETFLOAT(oled->table_bytes+oled->table_idx+2,atom_getfloat(&argv[1]));
        SETFLOAT(oled->table_bytes+oled->table_idx+3,atom_getfloat(&argv[2]));
        SETFLOAT(oled->table_bytes+oled->table_idx+4,atom_getfloat(&argv[3]));
        SETFLOAT(oled->table_bytes+oled->table_idx+5,atom_getfloat(&argv[4]));
        oled->table_idx+=6;
    }
    else if((s == gensym("line") || s == gensym("l")) && argc == 4){
        SETFLOAT(oled->table_bytes+oled->table_idx,(float)('l'));
        SETFLOAT(oled->table_bytes+oled->table_idx+1,atom_getfloat(&argv[0]));
        SETFLOAT(oled->table_bytes+oled->table_idx+2,atom_getfloat(&argv[1]));
        SETFLOAT(oled->table_bytes+oled->table_idx+3,atom_getfloat(&argv[2]));
        SETFLOAT(oled->table_bytes+oled->table_idx+4,atom_getfloat(&argv[3]));
        oled->table_idx+=5;
    } 
    else if((s == gensym("string") || s == gensym("s")) && argc >= 3){  
        SETFLOAT(oled->table_bytes+oled->table_idx,(float)('s'));
        SETFLOAT(oled->table_bytes+oled->table_idx+1,atom_getfloat(&argv[0]));
        SETFLOAT(oled->table_bytes+oled->table_idx+2,atom_getfloat(&argv[1]));
        oled->table_idx+=3;
        
        for(int i=2; i<argc; i++){
            atom_string(&argv[i],oled->str,32);
            for(int j=0; j<32; j++){
                char cc = oled->str[j];
                SETFLOAT(oled->table_bytes+oled->table_idx,float(cc));
                oled->table_idx++;
                if(cc == 0) break;
            }
        }
    }
}


void* oled_new(t_floatarg ctx){
 
    t_oled* x = (t_oled*)pd_new(oled_class);
    x->o_midi = (t_outlet*)outlet_new(&x->x_obj, &s_list);
    x->table_bytes = (t_atom*)malloc(sizeof(t_atom)*128);
    x->table_idx = 0;
    x->context = int(ctx);
    x->str = (char*)malloc(sizeof(char)*32);
    oled_fresh(x);
    post("New screen helper for context[%d]",x->context);
    return (void*)x;
}

void oled_free(t_oled* x){
    outlet_free(x->o_midi);
    free(x->table_bytes);
    free(x->str);
}

void oled_setup(void){
    oled_class = class_new(
        gensym("oled"),
        (t_newmethod)oled_new,
        (t_method)oled_free,
        sizeof(t_oled),
        CLASS_DEFAULT,
        A_DEFFLOAT,
        (t_atomtype)0
    );

    class_addanything(oled_class,(t_method)oled_onCommand);
}
