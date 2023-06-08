#pragma once

#define MINIM_API_VER 5
#define HW_VER "H/W/D/1"

#define CMD_BYTE_COUNT_MAX 1024
#define CMD_VAR_COUNT_MAX 128

//sysex manID = 00 7f 7f
#define CMD_SYSEX_START '\xf0'
#define CMD_SYSEX_END   '\xf7'
const char CMD_SYSEX_ID[4] = {CMD_SYSEX_START,0x00,0x7f,0x7f};

#define CMD_SYMBOL_C_SCALE 's'   //83
#define CMD_SYMBOL_C_XOR 'x'     //88
#define CMD_SYMBOL_F_DRAW 'u'    //117
#define CMD_SYMBOL_F_RECT 'r'    //114
#define CMD_SYMBOL_F_LINE 'l'    //108
#define CMD_SYMBOL_F_STRING 's'  //115
#define CMD_SYMBOL_F_BITMAP 'b'  //98

#define CMD_SYMBOL_F_BIND 'V' //86
#define CMD_SYMBOL_F_UNBIND 'v' //118
#define CMD_SYMBOL_F_ALTER 'A' //65
#define CMD_SYMBOL_F_UPLOAD 'U' //85