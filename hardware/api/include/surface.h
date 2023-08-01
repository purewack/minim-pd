#pragma once
#include "gfx.h"
#include <stdbool.h>

#include "friend.h"
namespace API{

struct ParseArgs {
    void(*hook)(void* env, const char* command, int where);
    void* env;
    void(*logger)(const char* message);
};
class ControlSurfaceAPI5 {
    friend class MINIM::ControlSurface;
private:
    bool sysex = false;
    uint8_t context = 0;
    uint8_t updateContextsFlag = 0;
    uint8_t errorContextsFlag = 0;
    int32_t errorLocation[CONTEXT_MAX];
    API::DisplayList cmdList[CONTEXT_MAX];
    uint8_t sharedBuffer[SHARED_BUF_MAX];
    // uint8_t icons[ICONS_MAX];
    bool _isArgsValid(const unsigned char* midiBytes, unsigned int count);
    int _MidiStreamHasSysex(const unsigned char* midiStreamBytes, int midiStreamBytesLength);
   
    
public:
    API::BufferPainter gfx;
    int parseDisplayList(unsigned int forContext);
    int parseMidiCommands(
        unsigned int offset, 
        const unsigned char* midiBytes, 
        const int midiBytesCount, 
        ParseArgs& parseArgs
    );
    int parseMidiStream(
        const unsigned char* midiStreamBytes, 
        int midiStreamBytesLength
    );
    int parseMidiStream(
        const unsigned char* midiStreamBytes, 
        int midiStreamBytesLength, 
        ParseArgs& parseArgs
    );
    int updateContext(int context);
};
}