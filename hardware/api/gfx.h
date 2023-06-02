#pragma once

#include <stdint.h>

#ifdef _MINIM_TARGET_BUILD
#include <Arduino.h>
#else
#include <stdlib.h>
#endif

#include "gfx/fonttiny.h"

class BufferPainter {
private:
  int inited = 0;
  int rotated = 0;
  int scale = 1;
  int modexor = 0;
  volatile int transferring = 0;

public:
  uint32_t fbuf_top[128]; // 128 vertical columns
  uint32_t fbuf_bot[128]; // 128 vertical columns

    void reset();
    void clear();
    void drawPixel(int x, int y, int tx, int ty);
    void drawHline(int x, int y, int w);
    void drawVline(int x, int y, int h);
    void drawLine(int x, int y, int x2, int y2);
    void drawRectSize(int x, int y, int w, int h);
    void fillSection(int xoff, int yoff, int xlen, int ylen);
    void drawBitmap(int x, int y, int w, int h, int bytes_per_col, int blen, const uint8_t* buf);
    void drawChar(char ch, int x, int y);
    void drawString(const char* str, int x, int y);
};