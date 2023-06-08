#include "responder.h"
#include "gfx.h"
#include "api.h"
#include <cstring>

int GraphicsResponder::writeContextStream(int context, const unsigned char* midiBytes){
    if(context < 0 || context > 6) return 0;
    
    this->cmdList[context].clear();
    int i=0;
    do{
        this->cmdList[context].add(midiBytes[i++]);
    }while(i < CMD_BYTE_COUNT_MAX || !(midiBytes[i] & 0x80));
    return i;
}

int GraphicsResponder::parseMidiStream(const unsigned char* midiStreamBytes, int midiStreamBytesLength){
    int draws = 0;
    for(int i=0; i<midiStreamBytesLength; i++){
        if(midiStreamBytes[i] == CMD_SYSEX_START){
            if(memcmp(&midiStreamBytes[i],CMD_SYSEX_ID,4) == 0){
                i+=4;
                int context = midiStreamBytes[i];
                i += this->writeContextStream(context,&midiStreamBytes[i+1]);
            }
            draws += 1;
        }
        else if(midiStreamBytes[i] == CMD_SYSEX_END){
            for(int i=0; i<6; i++){
                this->cmdList[i].parseCommands(&this->gfx);
            }
        }
    }

    return draws
}