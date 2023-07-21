#include "../include/surface.h"
#include "../include/api.h"

#define CHECK(X) if(!_isArgsValid(& midiBytes[i+1],midiBytesCount-i)) {\
        if(check.hook) {check.hook(check.env, X"_arg_error",offset+i);}\
        return -i;\
    }\
    if(check.logger) {check.logger(X);} \
    if(check.hook) {check.hook(check.env,X,offset+i); i+=midiBytes[i+1]+1; continue;}
            

bool API::ControlSurfaceAPI5::_isArgsValid(const unsigned char* midiBytes, unsigned int remain){
    unsigned int count = midiBytes[0];
    if(count > remain) return false;
    for(unsigned int i=0; i<count; i++){
        if( midiBytes[i+1] & 0x80 ) return false;
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

int API::ControlSurfaceAPI5::parseMidiCommands(unsigned int offset, const unsigned char* midiBytes, const int midiBytesCount, API::ParseArgs& check){

    auto list = &this->cmdList[this->context];
    if(!check.hook) list->clear();
    int i;
    for(i=0; i<midiBytesCount; i++){
        if( midiBytes[i] & 0x80 ) {
            if(check.logger) check.logger("status");
            if(check.hook) {
                if(midiBytes[i] == CMD_SYSEX_ID[0]
                && midiBytes[i+1] == CMD_SYSEX_ID[1]
                && midiBytes[i+2] == CMD_SYSEX_ID[2]
                && midiBytes[i+3] == CMD_SYSEX_ID[3] ){
                    check.hook(check.env,"start_i",offset+i); i+=4; continue;
                }
                else if(midiBytes[i] == CMD_SYSEX_END){
                    check.hook(check.env,"end_i",offset+i); continue;
                }
            }
            else 
                return i;
        }
        if( midiBytes[i] == CMD_SYMBOL_LINK){
            CHECK("link");
            int atHigh = midiBytes[i+2]; 
            int atLow  = midiBytes[i+3];
            int argListAt = atLow | (atHigh<<7);

            //find argument address to link ommiting command code itself
            //i.e. argListAt refers to argument order not absolute list contents
                int argCount=0;
                for(int i=1; i<list->getCount(); i++){
                    int c = list->getCommandAt(i++);
                    for(int j=0; j<c; j++){
                        if(argListAt == argCount){
                            argListAt = i+j;
                            i=list->getCount();
                            break;
                        }
                        argCount+=1;
                    }
                    i += c;
                } 
            //

            list->autoLink(argListAt);
            i+=3;
            continue;
        }
        else if( midiBytes[i] == CMD_SYMBOL_LINE){
            CHECK("line");
            int c = midiBytes[i+1]+2;
            for(int j=0; j<c; j++)
                list->add(midiBytes[j+i]);  
            i+=c-1;
            continue;
        }
        else if( midiBytes[i] == CMD_SYMBOL_RECT){
            CHECK("rect");
            int c = midiBytes[i+1]+2;
            for(int j=0; j<c; j++)
                list->add(midiBytes[j+i]);  
            i+=c-1;
            continue;
        }
        else {
            return -i;
        }
    }
    return i;
}

int API::ControlSurfaceAPI5::parseMidiStream(const unsigned char* midiStreamBytes, int midiStreamBytesLength){
    ParseArgs args = {0,0,0};
    return parseMidiStream(midiStreamBytes,midiStreamBytesLength,args);
}
int API::ControlSurfaceAPI5::parseMidiStream(const unsigned char* midiStreamBytes, int midiStreamBytesLength, API::ParseArgs& check){
    
    int draws = 0;
    for(int i=0; i<midiStreamBytesLength; i++){
        if(midiStreamBytes[i] == CMD_SYSEX_START){
            this->sysex = true;
            if(midiStreamBytes[i] == CMD_SYSEX_ID[0]
            && midiStreamBytes[i+1] == CMD_SYSEX_ID[1]
            && midiStreamBytes[i+2] == CMD_SYSEX_ID[2]
            && midiStreamBytes[i+3] == CMD_SYSEX_ID[3] ){
                //mode: write display list to context
                i+=4;
                auto context = midiStreamBytes[i++];
                if(context > CONTEXT_MAX-1) context = CONTEXT_MAX-1;
                this->context = context;
                
                if(midiStreamBytes[i+1] & 0x80){
                    if(check.logger) check.logger("end partial");
                    if(check.hook) check.hook(check.env,"end_partial",i);
                    this->sysex = false;
                    i+=1;
                    this->updateContextsFlag |= (1<<context);
                    continue;
                }

                if(check.hook) check.hook(check.env,"start_main",i);
                auto parsed = this->parseMidiCommands(context, &midiStreamBytes[i], midiStreamBytesLength-i, check);

                if(parsed < 0){
                    if(check.logger) check.logger("parse error");
                    if(check.hook) check.hook(check.env,"parse_error",i);
                    //error parsing, close to byte -parsed
                    this->errorContextsFlag |= (1<<context);
                    this->errorLocation[context] = (-parsed);
                    this->cmdList[context].clear();
                }
                else {
                    i += parsed-1;
                    if(check.logger) check.logger("parse ok");
                    if(check.hook) check.hook(check.env,"parse_ok",i);
                    this->updateContextsFlag |= (1<<context);
                    this->errorContextsFlag &= ~(1<<context);
                    this->errorLocation[context] = -1;
                }
        
                draws += 1;
            }
        }
        if(midiStreamBytes[i] & 0x80){
                if(check.logger) check.logger("have end");
            this->sysex = false;
            if(check.hook) check.hook(check.env,"end_full",i);
        }
        if(((midiStreamBytes[i]>>4) == 0x9) && !this->sysex){
            unsigned char context = (midiStreamBytes[i] & 0x0F) % CONTEXT_MAX;
            unsigned char note    = midiStreamBytes[i+1];
            unsigned char vel     = midiStreamBytes[i+2];
            if(check.hook) check.hook(check.env,"var",i);
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
int API::ControlSurfaceAPI5::updateContext(int context){
    if(this->updateContextsFlag & (1<<context)){
        return this->parseDisplayList(context);
    }
    return 0;
}


int API::ControlSurfaceAPI5::parseDisplayList(unsigned int context){
    if(context > CONTEXT_MAX) context = CONTEXT_MAX;
    
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
            gfx.scale = list->getCommandAt(i+2);
            if(gfx.scale <= 0) gfx.scale = 1;
            i+=2;
        }
        else if(list->getCommandAt(i) == CMD_SYMBOL_XOR){
            commandCount++;
            gfx.modexor = list->getCommandAt(i+2);
            i+=2;
        }
        else if(list->getCommandAt(i) == CMD_SYMBOL_LINE){
            commandCount++;
            int x = list->getCommandAt(i+2);
            int y = list->getCommandAt(i+3);
            int x2 = list->getCommandAt(i+4);
            int y2 = list->getCommandAt(i+5);
            gfx.drawLine(x,y,x2,y2);
            i+=5;
        }
        else if(list->getCommandAt(i) == CMD_SYMBOL_RECT){
            commandCount++;
            int x = list->getCommandAt(i+2);
            int y = list->getCommandAt(i+3);
            int w = list->getCommandAt(i+4);
            int h = list->getCommandAt(i+5);
            int fill = list->getCommandAt(i+6);
            i+=6;

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
