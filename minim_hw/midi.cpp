#include "include/cmd.h"

int parseCommand(const char* cmd_bytes, int len){
    for(int i=0; i<len; i++){
        if(cmd_bytes[i] == CMD_SYMBOL_F_MODE){
        	cmd_on_mode(cmd_bytes[++i]);
        }
        
        if(cmd_is_mode_data()){
          if(cmd_bytes[i] == CMD_SYMBOL_F_UPLOAD){
            int start = cmd_bytes[++i];
            int bytes = cmd_bytes[++i];
            unsigned char* buf = (unsigned char*)&cmd_bytes[++i];
            i += cmd_data_on_load(start,bytes,buf);
          }
        }
        else if(cmd_is_mode_gfx()){
            if(cmd_bytes[i] == CMD_SYMBOL_C_CONTEXT){
                cmd_gfx_on_context(cmd_bytes[++i]);
            }
            else if(cmd_bytes[i] == CMD_SYMBOL_C_SCALE){
                cmd_gfx_on_scale(cmd_bytes[++i]);
            }
			else if(cmd_bytes[i] == CMD_SYMBOL_C_XOR){
                cmd_gfx_on_xor(cmd_bytes[++i]);
            }

            else if(cmd_bytes[i] == CMD_SYMBOL_F_CLEAR){
                cmd_gfx_on_clear();
            }
            else if(cmd_bytes[i] == CMD_SYMBOL_F_DRAW){
                cmd_gfx_on_draw();
            }

            else if(cmd_bytes[i] == CMD_SYMBOL_F_LINE){
                int x = cmd_bytes[++i];
                int y = cmd_bytes[++i];
                int x2 = cmd_bytes[++i];
                int y2 = cmd_bytes[++i];
                cmd_gfx_on_line(x,y,x2,y2);
            }
            else if(cmd_bytes[i] == CMD_SYMBOL_F_RECT){
                int x = cmd_bytes[++i];
                int y = cmd_bytes[++i];
                int w = cmd_bytes[++i];
                int h = cmd_bytes[++i];
                int fill = cmd_bytes[++i];
                cmd_gfx_on_rect(x,y,w,h,fill);
            }
            else if(cmd_bytes[i] == CMD_SYMBOL_F_STRING){
                int x = cmd_bytes[++i];
                int y = cmd_bytes[++i];
                const char* str = &cmd_bytes[++i];
                i += cmd_gfx_on_string(x,y,str);
            }
            else if(cmd_bytes[i] == CMD_SYMBOL_F_BITMAP){
                int x = cmd_bytes[++i];
                int y = cmd_bytes[++i];
                int w = cmd_bytes[++i];
                int h = cmd_bytes[++i];
                unsigned int negs = cmd_bytes[++i];
                if(negs&0b0001) x = -x;
                if(negs&0b0010) y = -y;
                if(negs&0b0100) w = -w;
                if(negs&0b1000) h = -h;
                int start = cmd_bytes[++i];
                int bytes_per_col = cmd_bytes[++i];
                int len = cmd_bytes[++i];
                i += cmd_gfx_on_bitmap_from_buffer(x,y,w,h,negs,start,bytes_per_col,len);
            }
        }
        else{
            return -1;
        }
    }

    return 0;
}

