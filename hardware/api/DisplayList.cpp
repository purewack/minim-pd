#include "gfx.h"
#include "api.h"

API::DisplayList::DisplayList(uint8_t* buffer){
    this->commands.buf = buffer ? buffer : (uint8_t*)malloc(CMD_BYTE_COUNT_MAX * sizeof(uint8_t));
    this->commands.count = 0;
    this->commands.lim = CMD_BYTE_COUNT_MAX;
    this->unlinkAll();
}

API::DisplayList::~DisplayList(){
    free(this->commands.buf);
}

void API::DisplayList::unlinkAll(){
    for(int i=0; i<CMD_VAR_COUNT_MAX; i++) this->links[i] = -1;
}

void API::DisplayList::clear(){
    sarray_clear(this->commands);
    this->unlinkAll();
}
void API::DisplayList::add(unsigned char byte){
    sarray_push(this->commands, byte);
}
int API::DisplayList::getCount(){
    return this->commands.count;
}

void API::DisplayList::link(uint32_t listAt, uint8_t byteAt){
    if(listAt > this->commands.lim) return;
    if(byteAt >= CMD_VAR_COUNT_MAX) return;
    this->links[byteAt] = listAt;
}
void API::DisplayList::unlink(uint32_t byteAt){
    if(byteAt >= CMD_VAR_COUNT_MAX) return;
    this->links[byteAt] = -1;
}

void API::DisplayList::modifyAt(uint32_t byte, uint8_t value){
    if(byte > CMD_VAR_COUNT_MAX) return;
    if(this->links[byte] == -1) return;
    this->commands.buf[this->links[byte]] = value;
}

int API::DisplayList::getCommandAt(int i){
    if(i < 0 || i > this->commands.count) return -1;
    return this->commands.buf[i];
}

std::vector<uint8_t> API::DisplayList::getBufferCopy(){
    auto list = std::vector<uint8_t>();
    for(int i=0; i<this->commands.count; i++)
        list.push_back(this->commands.buf[i]);
    return list;
}
