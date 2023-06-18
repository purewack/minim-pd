#pragma once

#include <vector>

#include <stdint.h>
#include "libdarray.h"

#include "friend.h"
namespace API{
class BufferPainter {
private:
  int inited = 0;
  uint32_t fbuf_top[128]; // 128 vertical columns of upper 32 bits
  uint32_t fbuf_bot[128]; // 128 vertical columns of lower 32 bits

public:
  int rotated = 0;
  int scale = 1;
  int modexor = 0;
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
  friend class MINIM::DisplayList;

private:
    sarray_t<uint8_t> commands;
    int8_t links[128];    
    int lastLink = 0;
    void unlinkAll();
public:
    DisplayList(uint8_t* buffer = nullptr);
    ~DisplayList();
    void clear();
    void add(unsigned char byte);
    void link(uint32_t listAt, uint8_t linkAt);
    void unlink(uint32_t linkAt);
    int autoLink(uint32_t linkAt);
    void modifyAt(uint8_t slot, uint8_t value);
 
    std::vector<uint8_t> getBufferCopy();
    std::vector<uint8_t> getLinkBufferCopy();
    unsigned int getCommandAt(unsigned int i);
    unsigned int getLinkCount();
    unsigned int getCount();
};
}