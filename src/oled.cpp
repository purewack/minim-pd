
extern "C" {
#include "m_pd.h"
#include <stdlib.h>

static t_class *oled_class;
typedef struct _oled{
    t_object    x_obj;
    t_outlet*   o_midi;
    t_outlet*   o_info;
    int         context;
    t_atom*     a_table;
    t_atom*     a_info;
    int         a_table_i;
    char*       str;
    int         loaded_bitmap_offset;
    const char*       cwd;
    t_canvas*   canvas;
} t_oled;

void* oled_new(t_floatarg f);
void oled_free(t_oled* x);
void oled_setup(void);
void oled_onCommand(t_oled *x, t_symbol *s, int argc, t_atom *argv);

}

#include <cmath>
#include <limits>
#include <iostream>
#include <fstream>

void oled_fresh(t_oled *oled){
    SETFLOAT(oled->a_table+oled->a_table_i,(float)(0xf0));
    SETFLOAT(oled->a_table+oled->a_table_i+1,(float)('x'));
    SETFLOAT(oled->a_table+oled->a_table_i+2,(float)(2));
    SETFLOAT(oled->a_table+oled->a_table_i+3,(float)('G'));
    SETFLOAT(oled->a_table+oled->a_table_i+4,(float)(oled->context));
    oled->a_table_i = 5;
}

void oled_onCommand(t_oled *oled, t_symbol *s, int argc, t_atom *argv){
    if(oled->a_table_i > 128) {
        post("byte buffer full!");
        return;
    }
    if(s == gensym("reset_bytes")){
        oled->a_table_i = 0;
    }
    else if(s == gensym("send")){
        SETFLOAT(oled->a_table+oled->a_table_i,(float)('u'));
        SETFLOAT(oled->a_table+oled->a_table_i+1,(float)(0xf7));
        oled->a_table_i+=2;
        outlet_list(oled->o_midi,&s_list,oled->a_table_i,oled->a_table);
        oled_fresh(oled);
    }
    else if(s == gensym("clear")){
        SETFLOAT(oled->a_table+oled->a_table_i,(float)('c'));
        SETFLOAT(oled->a_table+oled->a_table_i+1,(float)('I'));
        SETFLOAT(oled->a_table+oled->a_table_i+2,(float)(0));
        oled->a_table_i+=3;
    }
    else if(s == gensym("xor")){
        SETFLOAT(oled->a_table+oled->a_table_i,(float)('I'));
        SETFLOAT(oled->a_table+oled->a_table_i+1,atom_getfloat(&argv[0]));
        oled->a_table_i+=2;
    }
    else if(s == gensym("scale") || s == gensym("S")){
        SETFLOAT(oled->a_table+oled->a_table_i,(float)('S'));
        SETFLOAT(oled->a_table+oled->a_table_i+1,atom_getfloat(&argv[0]));
        oled->a_table_i+=2;
    }

    else if((s == gensym("fill") || s == gensym("f")) && argc == 4){
        SETFLOAT(oled->a_table+oled->a_table_i,(float)('r'));
        SETFLOAT(oled->a_table+oled->a_table_i+1,atom_getfloat(&argv[0]));
        SETFLOAT(oled->a_table+oled->a_table_i+2,atom_getfloat(&argv[1]));
        SETFLOAT(oled->a_table+oled->a_table_i+3,atom_getfloat(&argv[2]));
        SETFLOAT(oled->a_table+oled->a_table_i+4,atom_getfloat(&argv[3]));
        SETFLOAT(oled->a_table+oled->a_table_i+5,float(1));
        oled->a_table_i+=6;
    }  
    else if((s == gensym("rect") || s == gensym("r")) && argc == 4){
        SETFLOAT(oled->a_table+oled->a_table_i,(float)('r'));
        SETFLOAT(oled->a_table+oled->a_table_i+1,atom_getfloat(&argv[0]));
        SETFLOAT(oled->a_table+oled->a_table_i+2,atom_getfloat(&argv[1]));
        SETFLOAT(oled->a_table+oled->a_table_i+3,atom_getfloat(&argv[2]));
        SETFLOAT(oled->a_table+oled->a_table_i+4,atom_getfloat(&argv[3]));
        SETFLOAT(oled->a_table+oled->a_table_i+5,float(0));
        oled->a_table_i+=6;
    } 
    else if((s == gensym("box") || s == gensym("b")) && argc == 5){
        SETFLOAT(oled->a_table+oled->a_table_i,(float)('r'));
        SETFLOAT(oled->a_table+oled->a_table_i+1,atom_getfloat(&argv[0]));
        SETFLOAT(oled->a_table+oled->a_table_i+2,atom_getfloat(&argv[1]));
        SETFLOAT(oled->a_table+oled->a_table_i+3,atom_getfloat(&argv[2]));
        SETFLOAT(oled->a_table+oled->a_table_i+4,atom_getfloat(&argv[3]));
        SETFLOAT(oled->a_table+oled->a_table_i+5,atom_getfloat(&argv[4]));
        oled->a_table_i+=6;
    }
    else if((s == gensym("line") || s == gensym("l")) && argc == 4){
        SETFLOAT(oled->a_table+oled->a_table_i,(float)('l'));
        SETFLOAT(oled->a_table+oled->a_table_i+1,atom_getfloat(&argv[0]));
        SETFLOAT(oled->a_table+oled->a_table_i+2,atom_getfloat(&argv[1]));
        SETFLOAT(oled->a_table+oled->a_table_i+3,atom_getfloat(&argv[2]));
        SETFLOAT(oled->a_table+oled->a_table_i+4,atom_getfloat(&argv[3]));
        oled->a_table_i+=5;
    } 
    else if((s == gensym("string") || s == gensym("s")) && argc >= 3){  
        SETFLOAT(oled->a_table+oled->a_table_i,(float)('s'));
        SETFLOAT(oled->a_table+oled->a_table_i+1,atom_getfloat(&argv[0]));
        SETFLOAT(oled->a_table+oled->a_table_i+2,atom_getfloat(&argv[1]));
        oled->a_table_i+=3;
        
        for(int i=2; i<argc; i++){
            atom_string(&argv[i],oled->str,128);
            for(int j=0; j<128; j++){
                char cc = oled->str[j];
                SETFLOAT(oled->a_table+oled->a_table_i,float(cc));
                oled->a_table_i++;
                if(cc == 0) break;
            }
        }
    }
    else if(s == gensym("upload") && argc >= 1){
        const char* filename = atom_getsymbol(&argv[0])->s_name;
        char path[MAXPDSTRING], *dummy;
        open_via_path(oled->cwd,filename,"",path,&dummy,MAXPDSTRING,1);
        snprintf(oled->str,MAXPDSTRING,"%s/%s",path,filename);
        auto glyph_file = sys_fopen(oled->str,"r");
        if(glyph_file){
            post("processing file gfx data");
            int len = 0;
            int bbc = 0;
            int offset = argc > 1 ? atom_getint(&argv[1]) : 0;
            SETFLOAT(oled->a_table+oled->a_table_i,(float)('x'));
            SETFLOAT(oled->a_table+oled->a_table_i+1,(float)(3));
            SETFLOAT(oled->a_table+oled->a_table_i+2,(float)('l'));
            SETFLOAT(oled->a_table+oled->a_table_i+3,(float)(offset));
            auto len_idx = oled->a_table_i+4;
            oled->a_table_i+=5;

            int ww = 0;
            int hh = 0;
            while(fgets(oled->str,512,glyph_file)){
                uint32_t byte = 0;
                int cc = 0;
                for(int j=0; j<512; j++){
                    auto c = oled->str[j];
                    if(c == 0) break;
                    if(c == '\n') break;
                    if(c == '\r') break;
                    if(c != ' '){
                        byte |= (1<<cc);
                    }
                    cc+=1;
                }   
                hh += 1;
                ww = std::max(ww,cc);
                bbc = std::max(bbc,int(std::floor(float(cc)/4.f)));
                if(bbc&1) bbc++;
                len += bbc;
                for(int i=0; i<bbc; i++){
                    auto bb = (byte>>(4*i))&0xf;
                    SETFLOAT(oled->a_table+oled->a_table_i,(float)(bb));
                    oled->a_table_i++;
                    //post("sending %x",bb);
                }
            }

            SETFLOAT(oled->a_table+len_idx,(float)(len));
            SETFLOAT(oled->a_table+oled->a_table_i,(float)(0xf7));
            oled->a_table_i++;
            outlet_list(oled->o_midi,&s_list,oled->a_table_i,oled->a_table);
            oled_fresh(oled);

            //post info of glyph loading
            SETFLOAT(oled->a_info+0,(float)(offset));
            SETFLOAT(oled->a_info+1,(float)(bbc/2));
            SETFLOAT(oled->a_info+2,(float)(len/2));
            SETFLOAT(oled->a_info+3,(float)(ww));
            SETFLOAT(oled->a_info+4,(float)(hh));
            outlet_anything(oled->o_info,gensym("gylph"),5,oled->a_info);

            sys_fclose(glyph_file);
        }
        else{
            pd_error(oled,"error opening file %s",filename);
        }
    }
    else if(s == gensym("glyph") && argc == 7){
        SETFLOAT(oled->a_table+oled->a_table_i,(float)('b'));
        SETFLOAT(oled->a_table+oled->a_table_i+1,atom_getfloat(&argv[3]));//x
        SETFLOAT(oled->a_table+oled->a_table_i+2,atom_getfloat(&argv[4]));//y
        SETFLOAT(oled->a_table+oled->a_table_i+3,atom_getfloat(&argv[5]));//w
        SETFLOAT(oled->a_table+oled->a_table_i+4,atom_getfloat(&argv[6]));//h
        SETFLOAT(oled->a_table+oled->a_table_i+5,atom_getfloat(&argv[0]));//image start idx
        SETFLOAT(oled->a_table+oled->a_table_i+6,atom_getfloat(&argv[1])); //bytes per column
        SETFLOAT(oled->a_table+oled->a_table_i+7,atom_getfloat(&argv[2])); //byte count
        oled->a_table_i+=8;
    }
    else{
        pd_error(oled,"invaild command");
    }
}


void* oled_new(t_floatarg ctx){
 
    t_oled* x = (t_oled*)pd_new(oled_class);
    x->o_midi = (t_outlet*)outlet_new(&x->x_obj, &s_list);
    x->o_info = (t_outlet*)outlet_new(&x->x_obj, &s_list);
    x->a_info = (t_atom*)malloc(sizeof(t_atom)*5);
    x->a_table = (t_atom*)malloc(sizeof(t_atom)*256);
    x->a_table_i = 0;
    x->context = int(ctx);
    x->str = (char*)malloc(sizeof(char)*MAXPDSTRING);
    x->canvas = canvas_getcurrent();
    x->cwd = canvas_getdir(x->canvas)->s_name;
    oled_fresh(x);
    post("New screen helper for context [%d]",x->context);
    
    return (void*)x;
}

void oled_free(t_oled* x){
    outlet_free(x->o_midi);
    outlet_free(x->o_info);
    free(x->a_table);
    free(x->a_info);
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
