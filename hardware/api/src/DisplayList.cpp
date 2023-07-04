#include "../include/gfx.h"
#include "../include/api.h"

API::DisplayList::DisplayList(uint8_t* buffer){
    this->commands.buf = this->_buf;
    this->commands.count = 0;
    this->commands.lim = CMD_BYTE_COUNT_MAX;
    this->unlinkAll();
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

int API::DisplayList::accessBuffer(uint8_t* buf){
    unsigned int i=0;
    for(i=0; i<this->commands.count; i++)
        buf[i] = (this->commands.buf[i]);
    return i;
}
int API::DisplayList::accessLinkBuffer(int16_t* buf){
    unsigned int i = 0;
    for(unsigned int i=0; i<CMD_LINK_COUNT_MAX; i++)
        buf[i] = (this->links[i]);
    return i;
}
