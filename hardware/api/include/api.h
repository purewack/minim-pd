#pragma once

#define MINIM_API_VER 5

#define CONTEXT_MAX 6

#define CMD_BYTE_COUNT_MAX 1024
#define CMD_MAP_COUNT_MAX  128
#define CMD_LINK_COUNT_MAX 128

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
#define CMD_SYMBOL_ALTER_TYPE "status"
#define CMD_SYMBOL_ALTER_DESC "((channel=context), slot, value): Alter a variable from the DisplayList without resending the whole code"
#define CMD_SYMBOL_ALTER_ARG 2
#define CMD_SYMBOL_ALTER_ARGV {\
    "Linked variable slot",\
    "Value to alter slot with"\
}\

#define CMD_SYMBOL_LINK 'V'
#define CMD_SYMBOL_LINK_ABR "link"
#define CMD_SYMBOL_LINK_TYPE "variable"
#define CMD_SYMBOL_LINK_DESC "(msb, lsb): Link a midi note to a display list variable"
#define CMD_SYMBOL_LINK_ARG 2
#define CMD_SYMBOL_LINK_ARGV {\
    "Higher byte of address of display list byte",\
    "Lower byte of address of display list byte"\
}\

#define CMD_SYMBOL_SCALE 'S'
#define CMD_SYMBOL_SCALE_ABR "scale"
#define CMD_SYMBOL_SCALE_TYPE "constant"
#define CMD_SYMBOL_SCALE_DESC "(number): Drawing scalar directive"
#define CMD_SYMBOL_SCALE_ARG 1
#define CMD_SYMBOL_SCALE_ARGV {\
    "Non 0 integer scalar"\
}\

#define CMD_SYMBOL_XOR 'X'
#define CMD_SYMBOL_XOR_ABR "fill"
#define CMD_SYMBOL_XOR_TYPE "constant"
#define CMD_SYMBOL_XOR_DESC "(boolean): XOR fill directive"
#define CMD_SYMBOL_XOR_ARG 1
#define CMD_SYMBOL_XOR_ARGV {\
    "Flag signifying whether preceeding draw commands use XOR fill or not"\
}\

#define CMD_SYMBOL_LINE 'L'
#define CMD_SYMBOL_LINE_ABR "line"
#define CMD_SYMBOL_LINE_TYPE "function"
#define CMD_SYMBOL_LINE_DESC "(x, y, x2, y2): Draw a line from (x,y) -> (x2,y2)"
#define CMD_SYMBOL_LINE_ARG 4
#define CMD_SYMBOL_LINE_ARGV {\
    "x start coordinate",\
    "y start coordinate",\
    "x2 end coordinate",\
    "y2 end coordinate"\
}\

#define CMD_SYMBOL_RECT 'R'
#define CMD_SYMBOL_RECT_ABR "rect"
#define CMD_SYMBOL_RECT_TYPE "function"
#define CMD_SYMBOL_RECT_DESC "(x,y,w,h,F): Draw a rectngle from (x,y) of size w * h, F = fill flag"
#define CMD_SYMBOL_RECT_ARG 5 
#define CMD_SYMBOL_RECT_ARGV {\
    "x start coordinate of rectangle",\
    "y start coordinate of rectangle",\
    "width of rectangle",\
    "height of rectangle",\
    "fill rectangle in if Non-zero"\
}\

#define CMD_SYMBOL_STRING 'T'
#define CMD_SYMBOL_STRING_ABR "string"
#define CMD_SYMBOL_STRING_TYPE "function"
#define CMD_SYMBOL_STRING_DESC "(x,y, string): draw string at (x,y), scalar dependant"
#define CMD_SYMBOL_STRING_ARG 3
#define CMD_SYMBOL_STRING_ARGV {\
    "x start coordinate of string\'s top left corner",\
    "y start coordinate of string\'s top left corner",\
    "...variable length string (NULL terminated c string)"\
}\

#define CMD_SYMBOL_BITMAP 'B'
#define CMD_SYMBOL_BITMAP_ABR "bitmap"
#define CMD_SYMBOL_BITMAP_TYPE "function"
#define CMD_SYMBOL_BITMAP_DESC "Draw a bitmap (previously stored in the shared memory pool)"
#define CMD_SYMBOL_BITMAP_ARG 7
#define CMD_SYMBOL_BITMAP_ARGV {\
    "Bitmap ID",\
    "x coordinate of bitmap top left corner",\
    "y coordinate of bitmap top left corner",\
    "Visible width of bitmap",\
    "Visible height of bitmap",\
    "--",\
    "--"\
}\
