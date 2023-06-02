
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
#include "../minim_hw/api/cmd.h"

void oled_fresh(t_oled *oled){
    SETFLOAT(oled->a_table+oled->a_table_i,(float)(0xf0));
    SETFLOAT(oled->a_table+oled->a_table_i+1,(float)(CMD_SYMBOL_F_MODE));
    SETFLOAT(oled->a_table+oled->a_table_i+2,(float)(CMD_SYMBOL_MODE_GFX));
    oled->a_table_i = 3;
}

void oled_onCommand(t_oled *oled, t_symbol *s, int argc, t_atom *argv){
    if(oled->a_table_i > 1000) {
        post("byte buffer full!");
        outlet_list(oled->o_midi,&s_list,oled->a_table_i,oled->a_table);
        oled->a_table_i = 0;
        return;
    }
    if(s == gensym("reset_bytes")){
        oled->a_table_i = 0;
    }
    else if(s == gensym("context")){
        oled->context = int(atom_getfloat(&argv[0]));
        post("[context changed to %d]",oled->context);        
        oled_fresh(oled);
        SETFLOAT(oled->a_table+oled->a_table_i+0,(float)(CMD_SYMBOL_C_CONTEXT));
        SETFLOAT(oled->a_table+oled->a_table_i+1,oled->context);
        oled->a_table_i+=2;
        return;
    }
    else if(oled->a_table_i > 128 || s == gensym("send")){
        SETFLOAT(oled->a_table+oled->a_table_i,(float)(CMD_SYMBOL_F_DRAW));
        SETFLOAT(oled->a_table+oled->a_table_i+1,(float)(0xf7));
        oled->a_table_i+=2;
        outlet_list(oled->o_midi,&s_list,oled->a_table_i,oled->a_table);      
        oled_fresh(oled);
        SETFLOAT(oled->a_table+oled->a_table_i+0,(float)(CMD_SYMBOL_C_CONTEXT));
        SETFLOAT(oled->a_table+oled->a_table_i+1,oled->context);
        oled->a_table_i+=2;
    }
    else if(s == gensym("clear")){
        SETFLOAT(oled->a_table+oled->a_table_i,(float)(CMD_SYMBOL_F_CLEAR));
        oled->a_table_i++;
    }
    else if(s == gensym("xor")){
        SETFLOAT(oled->a_table+oled->a_table_i,(float)(CMD_SYMBOL_C_XOR));
        SETFLOAT(oled->a_table+oled->a_table_i+1,atom_getfloat(&argv[0]));
        oled->a_table_i+=2;
    }
    else if(s == gensym("scale") || s == gensym("S")){
        SETFLOAT(oled->a_table+oled->a_table_i,(float)(CMD_SYMBOL_C_SCALE));
        SETFLOAT(oled->a_table+oled->a_table_i+1,atom_getfloat(&argv[0]));
        oled->a_table_i+=2;
    }

    else if((s == gensym("fill") || s == gensym("f")) && argc == 4){
        SETFLOAT(oled->a_table+oled->a_table_i,(float)(CMD_SYMBOL_F_RECT));
        SETFLOAT(oled->a_table+oled->a_table_i+1,atom_getfloat(&argv[0]));
        SETFLOAT(oled->a_table+oled->a_table_i+2,atom_getfloat(&argv[1]));
        SETFLOAT(oled->a_table+oled->a_table_i+3,atom_getfloat(&argv[2]));
        SETFLOAT(oled->a_table+oled->a_table_i+4,atom_getfloat(&argv[3]));
        SETFLOAT(oled->a_table+oled->a_table_i+5,float(1));
        oled->a_table_i+=6;
    }  
    else if((s == gensym("rect") || s == gensym("r")) && argc == 4){
        SETFLOAT(oled->a_table+oled->a_table_i,(float)(CMD_SYMBOL_F_RECT));
        SETFLOAT(oled->a_table+oled->a_table_i+1,atom_getfloat(&argv[0]));
        SETFLOAT(oled->a_table+oled->a_table_i+2,atom_getfloat(&argv[1]));
        SETFLOAT(oled->a_table+oled->a_table_i+3,atom_getfloat(&argv[2]));
        SETFLOAT(oled->a_table+oled->a_table_i+4,atom_getfloat(&argv[3]));
        SETFLOAT(oled->a_table+oled->a_table_i+5,float(0));
        oled->a_table_i+=6;
    } 
    else if((s == gensym("box") || s == gensym("b")) && argc == 5){
        SETFLOAT(oled->a_table+oled->a_table_i,(float)(CMD_SYMBOL_F_RECT));
        SETFLOAT(oled->a_table+oled->a_table_i+1,atom_getfloat(&argv[0]));
        SETFLOAT(oled->a_table+oled->a_table_i+2,atom_getfloat(&argv[1]));
        SETFLOAT(oled->a_table+oled->a_table_i+3,atom_getfloat(&argv[2]));
        SETFLOAT(oled->a_table+oled->a_table_i+4,atom_getfloat(&argv[3]));
        SETFLOAT(oled->a_table+oled->a_table_i+5,atom_getfloat(&argv[4]));
        oled->a_table_i+=6;
    }
    else if((s == gensym("line") || s == gensym("l")) && argc == 4){
        SETFLOAT(oled->a_table+oled->a_table_i,(float)(CMD_SYMBOL_F_LINE));
        SETFLOAT(oled->a_table+oled->a_table_i+1,atom_getfloat(&argv[0]));
        SETFLOAT(oled->a_table+oled->a_table_i+2,atom_getfloat(&argv[1]));
        SETFLOAT(oled->a_table+oled->a_table_i+3,atom_getfloat(&argv[2]));
        SETFLOAT(oled->a_table+oled->a_table_i+4,atom_getfloat(&argv[3]));
        oled->a_table_i+=5;
    } 
    else if((s == gensym("string") || s == gensym("s")) && argc >= 3){  
        SETFLOAT(oled->a_table+oled->a_table_i,(float)(CMD_SYMBOL_F_STRING));
        SETFLOAT(oled->a_table+oled->a_table_i+1,atom_getfloat(&argv[0]));
        SETFLOAT(oled->a_table+oled->a_table_i+2,atom_getfloat(&argv[1]));
        oled->a_table_i+=3;
        for(int i=2; i<argc; i++){
            atom_string(&argv[i],oled->str,128);
            for(int j=0; j<128; j++){
                char cc = oled->str[j];
                if(cc == 0) break;
                SETFLOAT(oled->a_table+oled->a_table_i,float(cc));
                oled->a_table_i++;
            }
            SETFLOAT(oled->a_table+oled->a_table_i,float(' '));
            oled->a_table_i++;
        }
        SETFLOAT(oled->a_table+oled->a_table_i,float(0));
        oled->a_table_i++;
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
            int npc = 0;
            int offset = argc > 1 ? atom_getint(&argv[1]) : 0;
            SETFLOAT(oled->a_table+oled->a_table_i,(float)(CMD_SYMBOL_F_MODE));
            SETFLOAT(oled->a_table+oled->a_table_i+1,(float)(CMD_SYMBOL_MODE_DATA));
            SETFLOAT(oled->a_table+oled->a_table_i+2,(float)(CMD_SYMBOL_F_UPLOAD));
            SETFLOAT(oled->a_table+oled->a_table_i+3,(float)(offset));
            auto len_idx = oled->a_table_i+4;
            oled->a_table_i+=5;

            int ww = 0;
            int hh = 0;
            while(fgets(oled->str,512,glyph_file)){
                uint8_t ybyte[16] = {0};
                int cc = 0;
                for(int j=0; j<512; j++){
                    auto c = oled->str[j];
                    if(c == 0) break;
                    if(c == '\n') break;
                    if(c == '\r') break;
                    if(c != ' '){
                        ybyte[(cc>>3)] |= (1<<(cc%8));
                    }
                    cc+=1;
                }   
                //bitmap height
                hh += 1;
                //bitmap width
                ww = std::max(ww,cc);
                //nibbles per column
                npc = std::max(npc,int(std::floor(float(ww)/4.f)));
                if(npc&1) npc++;
                len += npc;
                for(int i=0; i<npc; i+=2){
                    auto n1 = (ybyte[(i>>1)]&0xf);
                    auto n2 = (ybyte[(i>>1)]>>4)&(0xf);
                    SETFLOAT(oled->a_table+oled->a_table_i,(float)(n1));
                    SETFLOAT(oled->a_table+oled->a_table_i+1,(float)(n2));
                    oled->a_table_i+=2;
                    //post("sending %x",bb);
                }
            }

            SETFLOAT(oled->a_table+len_idx,(float)(len));
            SETFLOAT(oled->a_table+oled->a_table_i,(float)(0xf7));
            oled->a_table_i++;
            outlet_list(oled->o_midi,&s_list,oled->a_table_i,oled->a_table);      
            oled_fresh(oled);
            SETFLOAT(oled->a_table+oled->a_table_i+0,(float)(CMD_SYMBOL_C_CONTEXT));
            SETFLOAT(oled->a_table+oled->a_table_i+1,oled->context);
            oled->a_table_i+=2;

            //post info of glyph loading
            post("uploaded glyph [w:%d h:%d off:%d bpc:%d bcount:%d offset_next:%d]",
                hh,ww,offset,npc/2,len/2,offset+len/2
            );
            SETFLOAT(oled->a_info+0,(float)(offset));
            SETFLOAT(oled->a_info+1,(float)(npc/2));
            SETFLOAT(oled->a_info+2,(float)(len/2));
            SETFLOAT(oled->a_info+3,(float)(offset + len/2));
            outlet_anything(oled->o_info,gensym("glyph_upload_info"),4,oled->a_info);

            sys_fclose(glyph_file);
        }
        else{
            pd_error(oled,"error opening file %s",filename);
        }
    }
    else if(s == gensym("glyph") && argc == 7){
        int negs = 0;
        int xx = int(atom_getfloat(&argv[0]));
        int yy = int(atom_getfloat(&argv[1]));
        int ww = int(atom_getfloat(&argv[2]));
        int hh = int(atom_getfloat(&argv[3]));
        if(xx < 0) {xx = -xx; negs |= 0b0001;}
        if(yy < 0) {yy = -yy; negs |= 0b0010;}
        if(ww < 0) {ww = -ww; negs |= 0b0100;}
        if(hh < 0) {hh = -hh; negs |= 0b1000;}
        SETFLOAT(oled->a_table+oled->a_table_i,(float)(CMD_SYMBOL_F_BITMAP));
        SETFLOAT(oled->a_table+oled->a_table_i+1,xx);//x
        SETFLOAT(oled->a_table+oled->a_table_i+2,yy);//y
        SETFLOAT(oled->a_table+oled->a_table_i+3,ww);
        SETFLOAT(oled->a_table+oled->a_table_i+4,hh);
        SETFLOAT(oled->a_table+oled->a_table_i+5,negs);//y
        SETFLOAT(oled->a_table+oled->a_table_i+6,atom_getfloat(&argv[4]));//image start idx
        SETFLOAT(oled->a_table+oled->a_table_i+7,atom_getfloat(&argv[5])); //bytes per column
        SETFLOAT(oled->a_table+oled->a_table_i+8,atom_getfloat(&argv[6])); //byte count
        oled->a_table_i+=9;
    }
    else{
        pd_error(oled,"invaild command");
    }
}


void* oled_new(t_floatarg ctx){
 
    t_oled* x = (t_oled*)pd_new(oled_class);
    x->o_midi = (t_outlet*)outlet_new(&x->x_obj, &s_list);
    x->o_info = (t_outlet*)outlet_new(&x->x_obj, &s_list);
    x->a_info = (t_atom*)malloc(sizeof(t_atom)*10);
    x->a_table = (t_atom*)malloc(sizeof(t_atom)*1024);
    x->a_table_i = 0;
    x->context = int(ctx);
    x->str = (char*)malloc(sizeof(char)*MAXPDSTRING);
    x->canvas = canvas_getcurrent();
    x->cwd = canvas_getdir(x->canvas)->s_name;      
    oled_fresh(x);
    SETFLOAT(x->a_table+x->a_table_i+0,(float)(CMD_SYMBOL_C_CONTEXT));
    SETFLOAT(x->a_table+x->a_table_i+1,x->context);
    x->a_table_i+=2;
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
