#pragma once

#include <stdint.h>

#ifdef _MINIM_TARGET_BUILD
#include <Arduino.h>
#else
#include <stdlib.h>
#endif

// #include "gfx/fonttiny.h"

// struct gfx_t {
//   int inited = 0;
//   int rotated = 0;
//   int scale = 1;
//   int modexor = 0;
//   volatile int transferring = 0;

//   uint32_t fbuf_top[128]; // 128 vertical columns
//   uint32_t fbuf_bot[128]; // 128 vertical columns
// };
// extern gfx_t gfx;
// extern uint8_t* data_buf;

const char* gfx_dummy();
void gfx_defaults();
void gfx_clear();
void gfx_drawHline(int x, int y, int w);
void gfx_drawVline(int x, int y, int h);
void gfx_drawLine(int x, int y, int x2, int y2);
void gfx_drawRectSize(int x, int y, int w, int h);
void gfx_fillSection(int xoff, int yoff, int xlen, int ylen);
void gfx_drawBitmap(int x, int y, int w, int h, int bytes_per_col, int blen, const uint8_t* buf);
void gfx_drawChar(char ch, int x, int y);
void gfx_drawString(const char* str, int x, int y);
