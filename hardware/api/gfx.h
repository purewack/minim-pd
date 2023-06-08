#pragma once

#include <vector>

#include <stdint.h>
#include "libdarray.h"

namespace API{
class DisplayList;
class BufferPainter {
  friend class DisplayList;

private:
  int inited = 0;
  int rotated = 0;
  int scale = 1;
  int modexor = 0;
  uint32_t fbuf_top[128]; // 128 vertical columns of upper 32 bits
  uint32_t fbuf_bot[128]; // 128 vertical columns of lower 32 bits

public:
  std::vector<uint8_t> getBufferCopy();
  void clear();
  void resetScaleRotate();
  uint8_t getPixel(unsigned char x, unsigned char y);
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

class DisplayList {
private:
    sarray_t<uint8_t> commands;
    int32_t links[128];    
    void unlinkAll();
public:
    DisplayList();
    ~DisplayList();
    void clear();
    void add(unsigned char byte);
    void link(uint32_t listAt, uint8_t byteAt);
    void unlink(uint32_t byteAt);
    void modifyAt(uint32_t byte, uint8_t value);
    // void addAt(uint32_t at, uint8_t* bytes, uint32_t count);
    // void removeAt(uint32_t at, uint32_t count);

    int parseCommands(BufferPainter* gfx);
    int accessRawBuffer(uint8_t* pointer);
};
}