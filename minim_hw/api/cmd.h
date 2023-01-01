#pragma once

#define CMD_SYMBOL_F_MODE 'x'        //120

#define CMD_SYMBOL_MODE_SYS 1
    #define CMD_SYMBOL_F_SLEEP 's'   //115
    #define CMD_SYMBOL_F_VAR 'v'   //118

#define CMD_SYMBOL_MODE_GFX 2
    #define CMD_SYMBOL_F_CLEAR 'c'   //99
    #define CMD_SYMBOL_F_DRAW 'u'    //117
    #define CMD_SYMBOL_F_RECT 'r'    //114
    #define CMD_SYMBOL_F_LINE 'l'    //108
    #define CMD_SYMBOL_F_STRING 's'  //115
    #define CMD_SYMBOL_F_BITMAP 'b'  //98
    #define CMD_SYMBOL_C_CONTEXT 'G' //71
    #define CMD_SYMBOL_C_SCALE 'S'   //83
    #define CMD_SYMBOL_C_XOR 'X'     //88

#define CMD_SYMBOL_MODE_DATA 3
    #define CMD_SYMBOL_F_UPLOAD 'l'  //108
    #define CMD_SYMBOL_F_FRAME_SET 's' //115
    #define CMD_SYMBOL_F_FRAME_CLEAR 'c' //99
    #define CMD_SYMBOL_F_FRAME_TOGGLE 't' //116
    #define CMD_SYMBOL_F_BOOTCMD 'b'       //98

#define MIDI_BASE 35

#define FRAME_VAR_LIMIT 128
#define FRAME_BYTE_LIMIT 128

struct frame_t
{
    bool isFramed;
    unsigned char cmd_bytes[FRAME_BYTE_LIMIT];
    unsigned char cmd_count;
    int fps_counter;
};
extern frame_t frames[6];
extern int var_bytes[FRAME_VAR_LIMIT];

extern void cmd_gfx_on_context(int ctx);
extern void cmd_gfx_on_draw();

extern void cmd_sys_on_upload_boot_begin();
extern void cmd_sys_on_upload_boot_byte(unsigned char byte);
extern void cmd_sys_on_upload_boot_end();
extern void cmd_sys_on_sleep(int ms);

extern int parseCommand(const unsigned char* cmd_bytes, int len);
extern int getCByte(unsigned char cc, unsigned char val);
extern int setCByte(unsigned char ch, unsigned char cc, unsigned char val);
