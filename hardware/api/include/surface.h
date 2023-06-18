#pragma once
#include "gfx.h"
#include <stdbool.h>

#include "friend.h"
namespace API{

struct ParseArgs {
    void(*onParseCommand)(const char* command, void* data);
    void* onParseData;
};
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
    bool _isArgsValid(const unsigned char* midiBytes, unsigned int count);
    int _MidiStreamHasSysex(const unsigned char* midiStreamBytes, int midiStreamBytesLength);
    int _commitContextStream(
        unsigned int context, 
        const unsigned char* midiBytes, 
        const int midiBytesCount, 
        ParseArgs& parseArgs
    );
    
public:
    int parseDisplayList(unsigned int forContext);
    int parseMidiStream(
        const unsigned char* midiStreamBytes, 
        int midiStreamBytesLength
    );
    int parseMidiStream(
        const unsigned char* midiStreamBytes, 
        int midiStreamBytesLength, 
        ParseArgs& parseArgs
    );
    void updateRequiredContexts();
};
}