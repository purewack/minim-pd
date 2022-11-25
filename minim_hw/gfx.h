#pragma once

#include <stdint.h>

struct gfx_t {
  int inited = 0;
  int rotated = 0;
  volatile int transferring = 0;

  uint32_t fbuf_top[128]; // 128 vertical columns
  uint32_t fbuf_bot[128]; // 128 vertical columns
};
extern gfx_t gfx;

void gfx_clear();
void gfx_drawHline(int x, int y, int w);
void gfx_drawVline(int x, int y, int h);
void gfx_drawLine(int x, int y, int x2, int y2);
void gfx_drawRectSize(int x, int y, int w, int h);
void gfx_fillSection(int yoff, int ylen, int xoff, int xlen, int fill);