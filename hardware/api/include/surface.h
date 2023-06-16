#pragma once
#include "gfx.h"
#include <stdbool.h>

#include "friend.h"
namespace API{
class ControlSurfaceAPI5 {
    friend class MINIM::ControlSurface;
private:
    bool sysex = false;
    uint8_t context = 0;
    uint8_t updateContextsFlag = 0;
    uint8_t errorContextsFlag = 0;
    int32_t errorLocation[6];
    API::DisplayList cmdList[6];
    API::BufferPainter gfx;
    bool isArgsValid(const unsigned char* midiBytes, unsigned int count);
    int MidiStreamHasSysex(const unsigned char* midiStreamBytes, int midiStreamBytesLength);
    int parseCommandList(int forContext);
    int parseContextStream(
        int context, 
        const unsigned char* midiBytes, 
        const int midiBytesCount, 
        void(*onParseCommand)(const char* command, void* data),
        void* onParseData
    );
    
public:
    int parseMidiStream(
        const unsigned char* midiStreamBytes, 
        int midiStreamBytesLength, 
        void(*onParseCommand)(const char* command, void* data) = nullptr,
        void* onParseData = nullptr
    );
    void updateRequiredContexts();
};
}