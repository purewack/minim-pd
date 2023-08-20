#include "include/util.h"
#include "include/common.h"
#include "include/io.h"

void selfTestOnInit(){
    cs.gfx.clear();
    cs.gfx.drawRectSize(0,0,60,60);
    cs.gfx.drawLine(0,0,60,60);
    cs.gfx.drawLine(0,0,60,30);
    cs.gfx.drawString("hello",64,0);
    cs.gfx.drawString("world",64,8);

    cs.forceDrawContext(0);
    cs.forceDrawContext(1);
    cs.forceDrawContext(2);
    cs.forceDrawContext(3);
    cs.forceDrawContext(4);
    cs.forceDrawContext(5);
}

void selfTestOnLoop(){
    cs.gfx.clear();

    for(int i=0; i<5; i++){
    cs.gfx.drawRectSize(i*16,0,15,15);
    if(io.bstate & (1<<i+5))
        cs.gfx.fillSection(i*16,0,15,15);
    }

    for(int i=0; i<5; i++){
    cs.gfx.drawRectSize(i*16,16,15,15);
    if(io.bstate & (1<<i))
        cs.gfx.fillSection(i*16,16,15,15);
    }

    cs.gfx.drawRectSize(5*16,8,15,15);
    if(io.bstate & (0x400))
    cs.gfx.fillSection(5*16,8,15,15);
    
    for(int i=0; i<2; i++){
        cs.gfx.drawRectSize(6*16 + i*8,8,7,7);
        cs.gfx.drawChar('0'+(1-i),6*16 + i*8,8);
        if(io.turns_state & (1<<1-i))
            cs.gfx.fillSection(6*16 + i*8,8,7,7);
    }

    char str[16];
    snprintf(str, 16,"T:%d,%d",io.turns_left,io.turns_right);

    cs.gfx.drawString(str,5*16,0);
    // for(int i=0; i<6; i++)
    // cs.forceDrawContext(i);
    cs.forceDrawContext(0);

    // if(io.bstate) {
    //     SLOG(io.bstate);
    //     delay(20);
    // }
}