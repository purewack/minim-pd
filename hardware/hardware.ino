#include "include/io.h"
#include "include/common.h"
#include "api/include/api.h"
#include "api/include/gfx.h"
#include "api/include/surface.h"
#include "api/src/ControlSurfaceAPI5.cpp"
#include "api/src/BufferPainter.cpp"
#include "api/src/DisplayList.cpp"

#include <USBMIDI.h>
#include <USBComposite.h>
#include <USBCompositeSerial.h>
#include <EEPROM.h>

USBMIDI usbmidi;
USBCompositeSerial logger;
MINIM::ControlSurface cs;

void setup(){
  
    // USBComposite.clear();
    // USBComposite.setProductId(0x0031);
    // USBComposite.setManufacturerString("M O T I V");
    // USBComposite.setProductString("MINIM");
    // usbmidi.registerComponent();
    // logger.registerComponent();
    // USBComposite.begin();
    Serial.begin(9600);

    cs.initGPIO();
    cs.initMemory();
    cs.initDisplays();

    // io_mux_init();
    // timer_pause(TIMER3);

    delay(1000);

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

    // onGfxContextChange(5);
    // gfx.modexor = 1;
    // gfx.scale = 1;
    // char version_str[32];
    // snprintf(version_str,32,"%s - API:%d",VERSION,MINIM_API_VER);
    // for(int i=0; i<32; i++){
    //   gfx_clear();
    //   gfx_drawString(version_str,0,64-8);
    //   gfx_fillSection(0,64-8,i*4,8);
    //   hookOnGfxDraw();
    //   delay(1);
    // }
    // gfx_clear();
    // hookOnGfxDraw();
        
    // int boot_cmd = EEPROM.read(0) ;
    // if(boot_cmd != 0xffff && boot_cmd){
    //   for(int i=0; i<boot_cmd; i++)
    //     sarray_push(sysexString,(char)EEPROM.read(i+1));
    //   parseCommand((unsigned char*)sysexString.buf,sysexString.count);
    //   sarray_clear(sysexString);
    // }
    // else
    //   logger.println("no boot info found");

    LOG("ready");
} 


unsigned char cb = 0;
void loop(){
  cs.gfx.clear();
  cs.gfx.drawRectSize(0,0,60,60);
  cs.gfx.drawLine(0,0,60,60);
  cs.gfx.drawLine(0,0,60,cb);
  cs.gfx.drawString("hello",64,0);
  cs.gfx.drawString("world",64,8);

  cs.forceDrawContext(0);
  cs.forceDrawContext(1);
  cs.forceDrawContext(2);
  cs.forceDrawContext(3);
  cs.forceDrawContext(4);
  cs.forceDrawContext(5);

  cb++;
  cb %= 60;

  // delay(20);
  // // auto tt = millis();
  // if(int a = usbmidi.available()){
  //   uint32_t aa = usbmidi.readPacket();
  //   uint8_t *b = (uint8_t*)&aa;
  //   // if(!sysex){
  //   //   // if(b[1] == 0xB1){
  //   //   //   cb = b[2];
  //   //   // }
  //   //   // else if(b[1] == 0xB0){
  //   //   //   auto a = setCByte(cb,b[2],b[3]_Midi(b, 1);
  //   //   // }
  //   // }
  //   // else 
  //   if(cs.isSysex()){
  //     cs.collectMidi(b, 0);
  //   }
  // }

  // // auto dt = millis() - tt;
  // // dt /= 1000;
  
  // io_mux_irq();
  // delay(5);
  // // delay(dt);
}
