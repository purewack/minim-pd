#include "surface.h"
#include "api.h"
#include <cstring>

bool API::ControlSurfaceAPI5::_isArgsValid(const unsigned char* midiBytes, unsigned int count){
    for(unsigned int i=0; i<count; i++){
        if( midiBytes[i] & 0x80 ) return false;
    }
    return true;
}
int API::ControlSurfaceAPI5::_MidiStreamHasSysex(const unsigned char* midiStreamBytes, int midiStreamBytesLength){
    if(midiStreamBytesLength < 4) return -1;
    for(int i=0; i<midiStreamBytesLength; i++){
        if(midiStreamBytes[i] == CMD_SYSEX_START) return 1;
    }
    return 0;
}

int API::ControlSurfaceAPI5::_commitContextStream(unsigned int context, const unsigned char* midiBytes, const int midiBytesCount, API::ParseArgs& check){
    if(context > 5) return 0;
    
    auto list = &this->cmdList[context];
    list->clear();
    int i;
    for(i=0; i<midiBytesCount; i++){
        if( midiBytes[i] & 0x80 ) return i;
        if( midiBytes[i] == CMD_SYMBOL_LINK){
            if(!_isArgsValid(& midiBytes[i+1],2)) {
                if(check.onParseCommand) {check.onParseCommand("link_arg_error",check.onParseData);}
                return -i;
            }
            if(check.onParseCommand) {check.onParseCommand("link",check.onParseData); i+=2; continue;}
            int atHigh = midiBytes[i+1]; 
            int atLow  = midiBytes[i+2];
            int listByte = atLow | (atHigh<<7); 
            list->autoLink(listByte);
            i+=2;
            continue;
        }
        else if( midiBytes[i] == CMD_SYMBOL_LINE){
            if(!_isArgsValid(& midiBytes[i+1],4)) {
                if(check.onParseCommand) {check.onParseCommand("line_arg_error",check.onParseData);}
                return -i;
            }
            if(check.onParseCommand) {check.onParseCommand("line",check.onParseData); i+=4; continue;}
            list->add(midiBytes[i]);   //line
            list->add(midiBytes[i+1]); //x
            list->add(midiBytes[i+2]); //y
            list->add(midiBytes[i+3]); //x2
            list->add(midiBytes[i+4]); //y2
            i+=4;
            continue;
        }
        else if( midiBytes[i] == CMD_SYMBOL_RECT){
            if(!_isArgsValid(& midiBytes[i+1],5)) {
                if(check.onParseCommand) {check.onParseCommand("rect_arg_error",check.onParseData);}
                return -i;
            }
            if(check.onParseCommand) {check.onParseCommand("rect",check.onParseData); i+=4; continue;}
            list->add(midiBytes[i]);   //rect
            list->add(midiBytes[i+1]); //x
            list->add(midiBytes[i+2]); //y
            list->add(midiBytes[i+3]); //w
            list->add(midiBytes[i+4]); //h
            list->add(midiBytes[i+5]); //fill
            i+=5;
            continue;
        }
        else {
            return -i;
        }
    }
    return i;
}

int API::ControlSurfaceAPI5::parseMidiStream(const unsigned char* midiStreamBytes, int midiStreamBytesLength){
    ParseArgs args = {0,0};
    return parseMidiStream(midiStreamBytes,midiStreamBytesLength,args);
}
int API::ControlSurfaceAPI5::parseMidiStream(const unsigned char* midiStreamBytes, int midiStreamBytesLength, API::ParseArgs& check){
    int draws = 0;
    for(int i=0; i<midiStreamBytesLength; i++){
        if(midiStreamBytes[i] == CMD_SYSEX_START){
            this->sysex = true;
            if(memcmp(&midiStreamBytes[i],CMD_SYSEX_ID,4) == 0){
                
                //mode: write display list to context
                i+=4;
                auto context = midiStreamBytes[i++];
                if(context > 5) context = 5;
                
                if(midiStreamBytes[i+1] & 0x80){
                    if(check.onParseCommand) check.onParseCommand("end_partial",check.onParseData);
                    this->sysex = false;
                    i+=1;
                    this->updateContextsFlag |= (1<<context);
                    continue;
                }

                if(check.onParseCommand) check.onParseCommand("start",check.onParseData);
                auto parsed = this->_commitContextStream(context, &midiStreamBytes[i], midiStreamBytesLength-i, check);
                
                if(parsed < 0){
                    //error parsing, close to byte -parsed
                    this->errorContextsFlag |= (1<<context);
                    this->errorLocation[context] = -parsed;
                    this->cmdList[context].clear();
                }
                else {
                    i += parsed-1;
                    this->updateContextsFlag |= (1<<context);
                    this->errorContextsFlag &= ~(1<<context);
                    this->errorLocation[context] = -1;
                }
        
                this->context = context;
                draws += 1;
            }
        }
        if(midiStreamBytes[i] & 0x80){
            this->sysex = false;
            if(check.onParseCommand) check.onParseCommand("end",check.onParseData);
        }
        if(((midiStreamBytes[i]>>4) == 0x9) && !this->sysex){
            unsigned char context = (midiStreamBytes[i] & 0x0F) % 6;
            unsigned char note    = midiStreamBytes[i+1];
            unsigned char vel     = midiStreamBytes[i+2];
            i += 2;
            //modify variable
            this->cmdList[context].modifyAt(note,vel);
            //update display for context
            this->updateContextsFlag |= (1<<context); 
            this->context = context;
            draws += 1;
            if(check.onParseCommand) check.onParseCommand("var",check.onParseData);
        }
    }

    return draws;
}
void API::ControlSurfaceAPI5::updateRequiredContexts(){
    for(int i=0; i<6; i++){
        if(this->updateContextsFlag & (1<<i))
            this->parseDisplayList(i);
    }
    this->updateContextsFlag = 0;
}


int API::ControlSurfaceAPI5::parseDisplayList(unsigned int context){
    if(context > 5) context = 5;
    
    int commandCount = 0;
    this->gfx.clear();
    this->gfx.resetScaleRotate();
    this->gfx.rotated = (context == 0) ? 0 : 1;
    auto list = &this->cmdList[context];
    auto count = list->getCount();
    this->updateContextsFlag &= ~(1<<context);

    for(unsigned int i=0; i<count; i++){
        if(list->getCommandAt(i) == CMD_SYMBOL_SCALE){
            commandCount++;
            gfx.scale = list->getCommandAt(i++);
            if(gfx.scale <= 0) gfx.scale = 1;
        }
        else if(list->getCommandAt(i) == CMD_SYMBOL_XOR){
            commandCount++;
            gfx.modexor = list->getCommandAt(i+1);
            i+=1;
        }
        else if(list->getCommandAt(i) == CMD_SYMBOL_LINE){
            commandCount++;
            int x = list->getCommandAt(i+1);
            int y = list->getCommandAt(i+2);
            int x2 = list->getCommandAt(i+3);
            int y2 = list->getCommandAt(i+4);
            gfx.drawLine(x,y,x2,y2);
            i+=4;
        }
        else if(list->getCommandAt(i) == CMD_SYMBOL_RECT){
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
        // else if(cmds[i] == CMD_SYMBOL_STRING){
        //     i++;
        //     int x = getCByte(cmds,&i);
        //     int y = getCByte(cmds,&i);
        //     const char* str = (char*)&cmds[++i];
        //     gfx_drawString(str,x,y); 
        //     int j = 0;
        //     while(cmds[i+j] != 0) j++;
        //     i+=j;
        // }
        // else if(cmds[i] == CMD_SYMBOL_BITMAP){
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
