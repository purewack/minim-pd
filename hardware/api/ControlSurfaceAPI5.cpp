#include "surface.h"
#include "api.h"
#include <cstring>

int API::ControlSurfaceAPI5::writeContextStream(int context, const unsigned char* midiBytes, const int midiBytesCount){
    if(context < 0 || context > 5) return 0;
    
    this->cmdList[context].clear();
    int i;
    for(i=0; i<midiBytesCount; i++){
        if( midiBytes[i] & 0x80 ) return i;
        this->cmdList[context].add(midiBytes[i]);
    }
    return i;
}

int API::ControlSurfaceAPI5::MidiStreamHasSysex(const unsigned char* midiStreamBytes, int midiStreamBytesLength){
    if(midiStreamBytesLength < 4) return -1;
    for(int i=0; i<midiStreamBytesLength; i++){
        if(midiStreamBytes[i] == CMD_SYSEX_START) return 1;
    }
    return 0;
}
int API::ControlSurfaceAPI5::parseMidiStream(const unsigned char* midiStreamBytes, int midiStreamBytesLength){
    int draws = 0;
    for(int i=0; i<midiStreamBytesLength; i++){
        if(midiStreamBytes[i] == CMD_SYSEX_START){
            this->sysex = true;
            if(memcmp(&midiStreamBytes[i],CMD_SYSEX_ID,4) == 0){
                //mode: write display list to context
                i+=4;
                int context = midiStreamBytes[i++];
                if(context > 5) context = 0;
                i += this->writeContextStream(context, &midiStreamBytes[i], midiStreamBytesLength-i) - 1;
                this->updateContextsFlag |= (1<<context); 
                this->context = context;
                draws += 1;
            }
        }
        if(midiStreamBytes[i] & 0x80){
            this->sysex = false;
        }
        if(midiStreamBytes[i] == NOTE_ON && !this->sysex){
            char context = midiStreamBytes[i] & 0x0F;
            char note    = midiStreamBytes[i+1];
            char vel     = midiStreamBytes[i+2];
            i += 2;
            //modify variable
            this->cmdList[context].modifyAt(note,vel);
            //update display for context
            this->updateContextsFlag |= (1<<context); 
            this->context = context;
            draws += 1;
        }
    }

    return draws;
}
void API::ControlSurfaceAPI5::updateRequiredContexts(){
    for(int i=0; i<6; i++){
        if(this->updateContextsFlag & (1<<i))
            this->parseCommandList(i);
    }
    this->updateContextsFlag = 0;
}



int API::ControlSurfaceAPI5::parseCommandList(int context){
    if(context > 5) return 0;
    
    int commandCount = 0;
    this->gfx.clear();
    this->gfx.resetScaleRotate();
    auto list = &this->cmdList[context];
    auto count = list->getCount();

    for(int i=0; i<count; i++){
        if(list->getCommandAt(i) == CMD_SYMBOL_C_SCALE){
            commandCount++;
            gfx.scale = list->getCommandAt(i++);
            if(gfx.scale <= 0) gfx.scale = 1;
        }
        else if(list->getCommandAt(i) == CMD_SYMBOL_C_XOR){
            commandCount++;
            gfx.modexor = list->getCommandAt(i+1);
            i+=1;
        }
        else if(list->getCommandAt(i) == CMD_SYMBOL_F_LINE){
            commandCount++;
            int x = list->getCommandAt(i+1);
            int y = list->getCommandAt(i+2);
            int x2 = list->getCommandAt(i+3);
            int y2 = list->getCommandAt(i+4);
            gfx.drawLine(x,y,x2,y2);
            i+=4;
        }
        else if(list->getCommandAt(i) == CMD_SYMBOL_F_RECT){
            commandCount++;
            int x = list->getCommandAt(i+1);
            int y = list->getCommandAt(i+2);
            int w = list->getCommandAt(i+3);
            int h = list->getCommandAt(i+4);
            int fill = list->getCommandAt(i+5);
            i+=5;

            if(fill)
                gfx.fillSection(x,y,w,h);
            else
                gfx.drawRectSize(x,y,w,h);
        }
        // else if(cmds[i] == CMD_SYMBOL_F_STRING){
        //     i++;
        //     int x = getCByte(cmds,&i);
        //     int y = getCByte(cmds,&i);
        //     const char* str = (char*)&cmds[++i];
        //     gfx_drawString(str,x,y); 
        //     int j = 0;
        //     while(cmds[i+j] != 0) j++;
        //     i+=j;
        // }
        // else if(cmds[i] == CMD_SYMBOL_F_BITMAP){
        //     i++;
        //     int x = getCByte(cmds,&i);
        //     int y = getCByte(cmds,&i);
        //     int w = getCByte(cmds,&i);
        //     int h = getCByte(cmds,&i);
        //     int start = getCByte(cmds,&i);
        //     int bytes_per_col = getCByte(cmds,&i);
        //     int llen = getCByte(cmds,&i);
        //     gfx_drawBitmap(x,y,w,h,bytes_per_col,llen,data_buf+start);
        // }
    }

    return commandCount;
}
