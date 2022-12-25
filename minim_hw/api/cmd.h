#pragma once

#define CMD_SYMBOL_F_SLEEP 's'

#define CMD_SYMBOL_F_MODE 'x'
#define CMD_SYMBOL_MODE_SYS 1
#define CMD_SYMBOL_MODE_GFX 2
#define CMD_SYMBOL_MODE_DATA 3
#define CMD_SYMBOL_MODE_FRAME 4
#define CMD_SYMBOL_MODE_VAR 5
#define CMD_SYMBOL_F_UPLOAD 'l'

#define CMD_SYMBOL_C_CONTEXT 'G'
#define CMD_SYMBOL_C_SCALE 'S'
#define CMD_SYMBOL_C_XOR 'X'

#define CMD_SYMBOL_F_CLEAR 'c'
#define CMD_SYMBOL_F_DRAW 'u'

#define CMD_SYMBOL_F_RECT 'r'
#define CMD_SYMBOL_F_LINE 'l'
#define CMD_SYMBOL_F_STRING 's'
#define CMD_SYMBOL_F_BITMAP 'b'

#define MIDI_BASE 35

#define CMD_SYMBOL_F_VAR 'v'
#define FRAME_VAR_LIMIT 32
#define FRAME_BYTE_LIMIT 128
struct frame_t
{
    bool isFramed;
    uint8_t cmd_bytes[FRAME_BYTE_LIMIT];
    uint8_t cmd_count;
    uint8_t var_bytes[FRAME_VAR_LIMIT];
    uint8_t var_count;
};
extern frame_t frames[6];

extern void cmd_gfx_on_context(int ctx);
extern void cmd_gfx_on_draw();

extern void cmd_sys_on_upload_boot_begin();
extern void cmd_sys_on_upload_boot_byte(unsigned char byte);
extern void cmd_sys_on_upload_boot_end();
extern void cmd_sys_on_sleep(int ms);

extern int parseCommand(const unsigned char* cmd_bytes, int len);
extern int parseVariable(unsigned char context, unsigned char slot, unsigned char value);