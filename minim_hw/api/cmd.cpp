#include "cmd.h"
#include "gfx.h"
static int cmd_mode = 0;
frame_t frames[6];
int var_bytes[FRAME_VAR_LIMIT];
int cSlot = 0; //current var slot pointer

//byte 1 LSB = 0b0nnnnnnn
//byte 2 MSB = 0b0VSnnnnn
//max value = +- 4095
// V = variable flag
// S = sign bit
int getCByte(unsigned char cc, unsigned char val){
    int ii = int(cc) | (int(val)<<7);
    if(ii & (1<<13)) return var_bytes[cc];
    if(ii & (1<<12)) return -ii;
    return ii;
}

int setCByte(unsigned char ch, unsigned char cc, unsigned char val){
    int ii = int(cc) | (int(val)<<7);
    var_bytes[ch] = ii;
    return ii;
}

int parseCommand(const unsigned char* cmd_bytes, int len){
    for(int i=0; i<len; i++){
        if(cmd_bytes[i] == CMD_SYMBOL_F_MODE){
        	cmd_mode = cmd_bytes[++i];
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
            else if(cmd_bytes[i] == CMD_SYMBOL_F_FRAME_CLEAR){
                int context = cmd_bytes[++i];
                frames[context].isFramed = false;
                frames[context].cmd_count = 0;
                continue;
            }
            else if(cmd_bytes[i] == CMD_SYMBOL_F_FRAME_TOGGLE){
                int context = cmd_bytes[++i];
                frames[context].isFramed = ! frames[context].isFramed;
                continue;
            }
            else if(cmd_bytes[i] == CMD_SYMBOL_F_FRAME_SET){
                int context = cmd_bytes[++i];
                int bytes = cmd_bytes[++i];
                unsigned char* buf = (unsigned char*)&cmd_bytes[++i];
                if(bytes < FRAME_BYTE_LIMIT){
                    frames[context].isFramed = true;
                    frames[context].fps_counter = 0;
                    frames[context].cmd_count = bytes;
                    auto fbuf = frames[context].cmd_bytes;
                    for(int j=0; j<bytes; j++){
                        fbuf[j] = buf[j] << 0;
                    }
                    i+=bytes;
                }
            }
            else if(cmd_bytes[i] == CMD_SYMBOL_F_BOOTCMD){
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
        }
        else if(cmd_mode == CMD_SYMBOL_MODE_GFX){
            
            if(cmd_bytes[i] == CMD_SYMBOL_C_CONTEXT){
                cmd_gfx_on_context(cmd_bytes[++i]);
            }
            else if(cmd_bytes[i] == CMD_SYMBOL_C_SCALE){
                gfx.scale = getCByte(cmd_bytes[++i],cmd_bytes[++i]);
                if(gfx.scale <= 0) gfx.scale = 1;
            }
			else if(cmd_bytes[i] == CMD_SYMBOL_C_XOR){
                gfx.modexor = getCByte(cmd_bytes[++i],cmd_bytes[++i]);
            }

            else if(cmd_bytes[i] == CMD_SYMBOL_F_CLEAR){
                gfx_clear();
            }
            else if(cmd_bytes[i] == CMD_SYMBOL_F_DRAW){
                cmd_gfx_on_draw();
            }

            else if(cmd_bytes[i] == CMD_SYMBOL_F_LINE){
                int x = getCByte(cmd_bytes[++i],cmd_bytes[++i]);
                int y = getCByte(cmd_bytes[++i],cmd_bytes[++i]);
                int x2 = getCByte(cmd_bytes[++i],cmd_bytes[++i]);
                int y2 = getCByte(cmd_bytes[++i],cmd_bytes[++i]);
                gfx_drawLine(x,y,x2,y2);
            }
            else if(cmd_bytes[i] == CMD_SYMBOL_F_RECT){
                int x = getCByte(cmd_bytes[++i],cmd_bytes[++i]);
                int y = getCByte(cmd_bytes[++i],cmd_bytes[++i]);
                int w = getCByte(cmd_bytes[++i],cmd_bytes[++i]);
                int h = getCByte(cmd_bytes[++i],cmd_bytes[++i]);
                int fill = getCByte(cmd_bytes[++i],cmd_bytes[++i]);

                if(fill)
                  gfx_fillSection(x,y,w,h);
                else
                  gfx_drawRectSize(x,y,w,h);
            }
            else if(cmd_bytes[i] == CMD_SYMBOL_F_STRING){
                int x = getCByte(cmd_bytes[++i],cmd_bytes[++i]);
                int y = getCByte(cmd_bytes[++i],cmd_bytes[++i]);
                const char* str = (char*)&cmd_bytes[++i];
                gfx_drawString(str,x,y); 
                int j = 0;
                while(cmd_bytes[i+j] != 0) j++;
                i+=j;
            }
            else if(cmd_bytes[i] == CMD_SYMBOL_F_BITMAP){
                int x = getCByte(cmd_bytes[++i],cmd_bytes[++i]);
                int y = getCByte(cmd_bytes[++i],cmd_bytes[++i]);
                int w = getCByte(cmd_bytes[++i],cmd_bytes[++i]);
                int h = getCByte(cmd_bytes[++i],cmd_bytes[++i]);
                int start = getCByte(cmd_bytes[++i],cmd_bytes[++i]);
                int bytes_per_col = getCByte(cmd_bytes[++i],cmd_bytes[++i]);
                int llen = getCByte(cmd_bytes[++i],cmd_bytes[++i]);
                gfx_drawBitmap(x,y,w,h,bytes_per_col,llen,data_buf+start);
            }
            
        }
        else if(cmd_mode == CMD_SYMBOL_MODE_SYS){
            if(cmd_bytes[i] == CMD_SYMBOL_F_SLEEP){
                cmd_sys_on_sleep(cmd_bytes[++i]);
            }
            else if(cmd_bytes[i] == CMD_SYMBOL_F_VAR){
                int slot = cmd_bytes[++i];
                int lsb = cmd_bytes[++i];
                int msb = cmd_bytes[++i];
                setCByte(slot,lsb,msb);
            }
        }
    }

    return 0;
}

