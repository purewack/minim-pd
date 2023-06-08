#pragma once
#include "gfx.h"
#include <stdbool.h>

class GraphicsResponder {
private:
    bool sysex = false;
    bool uploadFrame = false;
    BufferPainter gfx;
    DisplayList cmdList[6];
    uint8_t tagLag[4];
    uint8_t context = 0;
    int writeContextStream(int context, const unsigned char* midiBytes);

public:
    uint8_t midi_base = 35;
    int parseMidiStream(const unsigned char* midiStreamBytes, int midiStreamBytesLength);
};