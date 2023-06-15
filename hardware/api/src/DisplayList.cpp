#include "gfx.h"
#include "api.h"

API::DisplayList::DisplayList(uint8_t* buffer){
    this->commands.buf = buffer ? buffer : (uint8_t*)malloc(CMD_BYTE_COUNT_MAX * sizeof(uint8_t));
    this->commands.count = 0;
    this->commands.lim = CMD_BYTE_COUNT_MAX;
    this->unlinkAll();
}

API::DisplayList::~DisplayList(){
    if(this->commands.buf)
    free(this->commands.buf);
}

void API::DisplayList::unlinkAll(){
    for(int i=0; i<128; i++) this->links[i] = -1;
    this->lastLink = 0;
}

void API::DisplayList::clear(){
    sarray_clear(this->commands);
    this->unlinkAll();
}
void API::DisplayList::add(unsigned char byte){
    sarray_push(this->commands, byte);
}
unsigned int API::DisplayList::getCount(){
    return this->commands.count;
}
unsigned int API::DisplayList::getLinkCount(){
    return this->lastLink;
}
int API::DisplayList::autoLink(uint32_t listByte){
    this->link(listByte,this->lastLink);
    return ++this->lastLink;
}
void API::DisplayList::link(uint32_t listAt, uint8_t linkAt){
    if(listAt > this->commands.lim) return;
    if(linkAt >= 128) return;
    this->links[linkAt] = listAt;
}
void API::DisplayList::unlink(uint32_t linkAt){
    if(linkAt >= 128) return;
    this->links[linkAt] = -1;
}

void API::DisplayList::modifyAt(uint8_t slot, uint8_t value){
    if(slot >= 128) return;
    if(this->links[slot] == -1) return;
    this->commands.buf[this->links[slot]] = value;
}


unsigned int API::DisplayList::getCommandAt(unsigned int i){
    if(i > this->commands.count) return -1;
    return this->commands.buf[i];
}

std::vector<uint8_t> API::DisplayList::getBufferCopy(){
    auto list = std::vector<uint8_t>();
    for(unsigned int i=0; i<this->commands.count; i++)
        list.push_back(this->commands.buf[i]);
    return list;
}
std::vector<uint8_t> API::DisplayList::getLinkBufferCopy(){
    auto list = std::vector<uint8_t>();
    for(int i=0; i<128; i++)
        list.push_back(this->links[i]);
    return list;
}
