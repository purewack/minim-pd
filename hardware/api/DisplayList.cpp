#include "gfx.h"
#include "api.h"

DisplayList::DisplayList(){
    this->commands.buf = (uint8_t*)malloc(CMD_BYTE_COUNT_MAX * sizeof(uint8_t));
    this->commands.count = 0;
    this->commands.lim = CMD_BYTE_COUNT_MAX;
    this->unlinkAll();
}

DisplayList::~DisplayList(){
    free(this->commands.buf);
}

void DisplayList::unlinkAll(){
    for(int i=0; i<CMD_VAR_COUNT_MAX; i++) this->links[i] = -1;
}

void DisplayList::clear(){
    sarray_clear(this->commands);
    this->unlinkAll();
}
void DisplayList::add(unsigned char byte){
    sarray_push(this->commands, byte);
}

void DisplayList::link(uint32_t listAt, uint8_t byteAt){
    if(listAt > this->commands.lim) return;
    if(byteAt >= CMD_VAR_COUNT_MAX) return;
    this->links[byteAt] = listAt;
}
void DisplayList::unlink(uint32_t byteAt){
    if(byteAt >= CMD_VAR_COUNT_MAX) return;
    this->links[byteAt] = -1;
}

void DisplayList::modifyAt(uint32_t byte, uint8_t value){
    if(byte >= CMD_VAR_COUNT_MAX) return;
    if(this->links[byte] == -1) return;
    this->commands.buf[this->links[byte]] = value;
}

int DisplayList::accessRawBuffer(uint8_t* pointer){
    pointer = this->commands.buf;
    return this->commands.count;
}

int DisplayList::parseCommands(BufferPainter* gfx){

    int commandCount = 0;
    gfx->clear();
    auto cmds = this->commands.buf;

    for(int i=0; i<this->commands.count; i++){
        if(cmds[i++] == CMD_SYMBOL_C_SCALE){
            commandCount++;
            gfx->scale = cmds[i];
            if(gfx->scale <= 0) gfx->scale = 1;
        }
        else if(cmds[i++] == CMD_SYMBOL_C_XOR){
            commandCount++;
            gfx->modexor = cmds[i];
        }
        else if(cmds[i++] == CMD_SYMBOL_F_LINE){
            commandCount++;
            int x = cmds[i++];
            int y = cmds[i++];
            int x2 = cmds[i++];
            int y2 = cmds[i++];
            gfx->drawLine(x,y,x2,y2);
        }
        else if(cmds[i++] == CMD_SYMBOL_F_RECT){
            commandCount++;
            int x = cmds[i++];
            int y = cmds[i++];
            int w = cmds[i++];
            int h = cmds[i++];
            int fill = cmds[i++];

            if(fill)
                gfx->fillSection(x,y,w,h);
            else
                gfx->drawRectSize(x,y,w,h);
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