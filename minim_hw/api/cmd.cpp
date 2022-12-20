#include "cmd.h"
#include "gfx.h"

static int cmd_mode = 0;

int parseNote(unsigned char note, unsigned char vel){
    note %= 64;
    auto vv = &vsliders[note];
    cmd_gfx_on_context(vv->context);
    gfx.modexor = 0;
    gfx_fillSection(vv->x,vv->y,vv->w,vv->h);
    gfx.modexor = 1;
    gfx_fillSection(vv->x,vv->y,vv->w,vv->h);
    float prog = float(vel) / 127.0f;
    int ww = int(float(vv->w) * prog);
    gfx_drawRectSize(vv->x,vv->y,vv->w,vv->h);
    gfx_fillSection(vv->x,vv->y,ww,vv->h);
    cmd_gfx_on_draw();
    return 0;
}

int parseCommand(const unsigned char* cmd_bytes, int len){
    for(int i=0; i<len; i++){
        if(cmd_bytes[i] == CMD_SYMBOL_F_MODE){
        	cmd_mode = cmd_bytes[++i];
            if(cmd_mode == CMD_SYMBOL_MODE_DRAW){
                cmd_gfx_on_draw();
            }
        }
        
        if(cmd_mode == CMD_SYMBOL_MODE_DATA){
            if(cmd_bytes[i] == CMD_SYMBOL_F_UPLOAD){
                int start = cmd_bytes[++i];
                int bytes = cmd_bytes[++i];
                int nibbles = bytes<<1;
                unsigned char* buf = (unsigned char*)&cmd_bytes[++i];
                if(start+bytes < 512){
                    int d = 0;
                    for(int j=0; j<nibbles; j+=2){
                        data_buf[start+d] = uint8_t(buf[j+0]) << 0;
                        data_buf[start+d] |= uint8_t(buf[j+1]) << 4;
                        d++;
                    }
                    i+=nibbles;
                }
            }
            else if(cmd_bytes[i] == CMD_SYMBOL_F_SLIDER){
                int context = cmd_bytes[++i];
                int note = cmd_bytes[++i];
                int x = cmd_bytes[++i];
                int y = cmd_bytes[++i];
                int w = cmd_bytes[++i];
                int h = cmd_bytes[++i];
                int rot = cmd_bytes[++i];

                auto vs = &vsliders[note];
                vs->context = context;
                vs->x = x;
                vs->y = y;
                vs->w = w;
                vs->h = h;
                vs->rot = rot;
                vs->val = 0;
            }
        }
        else if(cmd_mode == CMD_SYMBOL_MODE_GFX){
            
            if(cmd_bytes[i] == CMD_SYMBOL_C_CONTEXT){
                cmd_gfx_on_context(cmd_bytes[++i]);
            }
            else if(cmd_bytes[i] == CMD_SYMBOL_C_SCALE){
                gfx.scale = cmd_bytes[++i];
                if(gfx.scale <= 0) gfx.scale = 1;
            }
			else if(cmd_bytes[i] == CMD_SYMBOL_C_XOR){
                gfx.modexor = cmd_bytes[++i];
            }

            else if(cmd_bytes[i] == CMD_SYMBOL_F_CLEAR){
                gfx_clear();
            }
            else if(cmd_bytes[i] == CMD_SYMBOL_F_DRAW){
                cmd_gfx_on_draw();
            }

            else if(cmd_bytes[i] == CMD_SYMBOL_F_LINE){
                int x = cmd_bytes[++i];
                int y = cmd_bytes[++i];
                int x2 = cmd_bytes[++i];
                int y2 = cmd_bytes[++i];
                gfx_drawLine(x,y,x2,y2);
            }
            else if(cmd_bytes[i] == CMD_SYMBOL_F_RECT){
                int x = cmd_bytes[++i];
                int y = cmd_bytes[++i];
                int w = cmd_bytes[++i];
                int h = cmd_bytes[++i];
                int fill = cmd_bytes[++i];
                if(fill)
                  gfx_fillSection(x,y,w,h);
                else
                  gfx_drawRectSize(x,y,w,h);
            }
            else if(cmd_bytes[i] == CMD_SYMBOL_F_STRING){
                int x = cmd_bytes[++i];
                int y = cmd_bytes[++i];
                const char* str = (char*)&cmd_bytes[++i];
                gfx_drawString(str,x,y); 
                int j = 0;
                while(cmd_bytes[i+j] != 0) j++;
                i+=j;
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
                int llen = cmd_bytes[++i];
                gfx_drawBitmap(x,y,w,h,bytes_per_col,llen,data_buf+start);
            }
            
        }
        else if(cmd_mode == CMD_SYMBOL_MODE_SYS){
            if(cmd_bytes[i] == CMD_SYMBOL_F_UPLOAD){
                int cc = cmd_bytes[++i];
                unsigned char* buf = (unsigned char*)&cmd_bytes[++i];
                if(cc <= 512){
                    cmd_sys_on_upload_boot_begin();
                    unsigned char data_byte = 0;
                    for(int j=0; j<cc; j++){
                        data_byte = uint8_t(buf[j]) << 0;
                        cmd_sys_on_upload_boot_byte(data_byte);
                    }
                    i+=cc;
                    cmd_sys_on_upload_boot_end();
                }
            }
            else if(cmd_bytes[i] == CMD_SYMBOL_F_SLEEP){
                cmd_sys_on_sleep(cmd_bytes[++i]);
            }
        }
    }

    return 0;
}

