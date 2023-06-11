#pragma once
#include "gfx.h"
#include <stdbool.h>

#include "../emulator/native/src/friend.h"
namespace API{
class ControlSurfaceAPI5 {
    friend class MINIM::ControlSurface;
private:
    bool sysex = false;
    uint8_t context = 0;
    uint8_t updateContextsFlag = 0;
    API::DisplayList cmdList[6];
    API::BufferPainter gfx;
    int writeContextStream(int context, const unsigned char* midiBytes, const int midiBytesCount);
    int parseCommandList(int forContext);
    int MidiStreamHasSysex(const unsigned char* midiStreamBytes, int midiStreamBytesLength);
public:
    int parseMidiStream(const unsigned char* midiStreamBytes, int midiStreamBytesLength);
    void updateRequiredContexts();
};
}