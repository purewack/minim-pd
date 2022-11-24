#pragma once

#include <stdint.h>

struct lcd_t {
  int inited = 0;
  volatile int transferring = 0;

  uint32_t fbuf_top[128]; // 128 vertical columns
  uint32_t fbuf_bot[128]; // 128 vertical columns
};
extern lcd_t lcd;

void lcd_clear();
void lcd_drawHline(int x, int y, int w);
void lcd_drawVline(int x, int y, int h);
void lcd_drawLine(int x, int y, int x2, int y2);
void lcd_drawRectSize(int x, int y, int w, int h);