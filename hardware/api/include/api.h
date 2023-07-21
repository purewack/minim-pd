#pragma once

#define MINIM_API_VER 5

#define CONTEXT_MAX 6

#define CMD_BYTE_COUNT_MAX 1024
#define CMD_MAP_COUNT_MAX  128
#define CMD_LINK_COUNT_MAX 128

//sysex manID = 00 7f 7f
#define CMD_ANY_STATUS_BYTE 0x80
#define CMD_SYSEX_START 0xF0
#define CMD_SYSEX_END   0xF7
#define CMD_SYSEX_DEBUG 0x7E
#define CMD_SYSEX_STRING "\xf0\0\x7f\x7f" 
#define CMD_SYSEX_STRING_DEBUG "\xf0\0\x7f\x7e" 
const unsigned char CMD_SYSEX_ID[4] = {CMD_SYSEX_START,0x00,0x7f,0x7f};

#define NOTE_ON  0x90
#define NOTE_OFF 0x80

#define CMD_SYMBOL_ALTER 0x90

#define CMD_SYMBOL_LINK 'V'

#define CMD_SYMBOL_SCALE 'S'


#define CMD_SYMBOL_XOR 'X'


#define CMD_SYMBOL_LINE 'L'

#define CMD_SYMBOL_RECT 'R'


#define CMD_SYMBOL_STRING 'T'


#define CMD_SYMBOL_BITMAP 'B'
