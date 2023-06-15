#pragma once

#define MINIM_API_VER 5

#define CMD_BYTE_COUNT_MAX 1024

//sysex manID = 00 7f 7f
#define CMD_SYSEX_START 0xF0
#define CMD_SYSEX_END   0xF7
#define CMD_SYSEX_DEBUG 0x7E
#define CMD_SYSEX_STRING "\xf0\0\x7f\x7f" 
#define CMD_SYSEX_STRING_DEBUG "\xf0\0\x7f\x7e" 
const unsigned char CMD_SYSEX_ID[4] = {CMD_SYSEX_START,0x00,0x7f,0x7f};

#define NOTE_ON  0x90
#define NOTE_OFF 0x80

#define CMD_SYMBOL_ALTER 0x90
#define CMD_SYMBOL_ALTER_ABR "alter"
#define CMD_SYMBOL_ALTER_ARG 2

#define CMD_SYMBOL_LINK 'V'
#define CMD_SYMBOL_LINK_ABR "link"
#define CMD_SYMBOL_LINK_ARG 2

#define CMD_SYMBOL_SCALE 'S'
#define CMD_SYMBOL_SCALE_ABR "scale"
#define CMD_SYMBOL_SCALE_ARG 1

#define CMD_SYMBOL_XOR 'X'
#define CMD_SYMBOL_XOR_ABR "fill"
#define CMD_SYMBOL_XOR_ARG 1

#define CMD_SYMBOL_LINE 'L'
#define CMD_SYMBOL_LINE_ABR "line"
#define CMD_SYMBOL_LINE_ARG 4

#define CMD_SYMBOL_RECT 'R'
#define CMD_SYMBOL_RECT_ABR "rect"
#define CMD_SYMBOL_RECT_ARG 5

#define CMD_SYMBOL_STRING 'S'
#define CMD_SYMBOL_STRING_ABR "string"
#define CMD_SYMBOL_STRING_ARG 3

#define CMD_SYMBOL_BITMAP 'B'
#define CMD_SYMBOL_BITMAP_ABR "bitmap"
#define CMD_SYMBOL_BITMAP_ARG 7
