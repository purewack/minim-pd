#pragma once

#define CMD_SYMBOL_F_MODE 'x'
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

extern void cmd_on_mode(int mode);
extern bool cmd_is_mode_gfx();
extern bool cmd_is_mode_data();

extern void cmd_gfx_on_context(int ctx);
extern void cmd_gfx_on_draw();
extern void cmd_gfx_on_clear();
extern void cmd_gfx_on_scale(int scale);
extern void cmd_gfx_on_xor(int xor_flag);
extern void cmd_gfx_on_rect(int x, int y, int w, int h, int fill);
extern void cmd_gfx_on_line(int x, int y, int x2, int y2);

extern int cmd_gfx_on_string(int x, int y, const char* str);
extern int cmd_gfx_on_bitmap_from_buffer(int x, int y, int w, int h, unsigned int negs, int boff, int bytes_per_col, int len);
extern int cmd_data_on_load(int start, int len, const unsigned char* nibbles);

extern int parseCommand(const char* cmd_bytes, int len);