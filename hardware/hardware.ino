#include "include/io.h"
#include "include/common.h"
#include "include/util.h"
#include "api/include/api.h"
#include "api/include/gfx.h"
#include "api/include/surface.h"
#include "api/src/ControlSurfaceAPI5.cpp"
#include "api/src/BufferPainter.cpp"
#include "api/src/DisplayList.cpp"

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

// #define SELFTEST
#define INTERNAL_TESTS


#include <USBMIDI.h>
#include <USBComposite.h>
#include <USBCompositeSerial.h>
#include <EEPROM.h>

USBMIDI usbmidi;
USBCompositeSerial logger;
MINIM::ControlSurface cs;

void setup(){
  
    USBComposite.clear();
    USBComposite.setProductId(0x0031);
    USBComposite.setManufacturerString("M O T I V");
    USBComposite.setProductString("MINIM");
    usbmidi.registerComponent();
    logger.registerComponent();
    USBComposite.begin();

#ifdef INTERNAL_TESTS
    Serial.begin(19200);
#else
    Serial.begin(31250);
#endif

    cs.initGPIO();
    cs.initMemory();
    cs.initDisplays();
    
#ifdef SELFTEST
    selfTestOnInit();
#else


    cs.gfx.drawString("MINIM - API " STR(MINIM_API_VER),0,64-8);

    cs.forceDrawContext(0);   

    const char logo[5] = {'M','O','T','I','V'};
    for(int i=0; i<5; i++){
      cs.gfx.clear();
      cs.gfx.scale = 10; 
      cs.gfx.rotated = 1;  
      cs.gfx.drawChar(logo[i],-6,20);
      cs.forceDrawContext(1+i);
    }
  
    // delay(1000);
    // for(int i=0; i<5; i++){
    //   cs.gfx.clear();
    //   cs.forceDrawContext(i);
    // }
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
    //     sarray_push(midiString,(char)EEPROM.read(i+1));
    //   parseCommand((unsigned char*)midiString.buf,midiString.count);
    //   sarray_clear(midiString);
    // }
    // else
    //   logger.println("no boot info found");
#endif
    io_mux_init();
    timer_resume(TIMER3);

    LOG("ready");
} 


unsigned char cb = 0;
void loop(){
#ifdef SELFTEST
  selfTestOnLoop();
#else
  cs.pollControls(io);
  
  // delay(20);
  // // auto tt = millis();
  if(int a = usbmidi.available()){
    uint32_t aa = usbmidi.readPacket();
    uint8_t *b = (uint8_t*)&aa;
    cs.collectMidi(b,4,1);
  }


  if(Serial.available()){
    auto cc = Serial.read();
    if(cc == 'T'){
      LOG("send test midi string");
      unsigned char testMidi[] = {
        240,0,127,127,0,
          76,4,0,0,20,20,
          86,2,0,0,
        240,0,127,127,1,
          76,4,0,0,20,20,
          86,2,0,0,
        247
      };
      cs.collectMidi(testMidi, sizeof(testMidi), 0); 
    }
    else if(cc == 'S'){
      LOG("send test midi string");
      unsigned char testMidi[] = {
        240,0,127,127,1,
          82,5,0,0,120,20,0,
          'S',1,2,
          84,8,5,5,'h','e','l','l','o',0,
        240,0,127,127,0,
          82,5,0,0,120,20,0,
          84,8,5,5,'h','e','l','l','#',0,
        247
      };
      cs.collectMidi(testMidi, sizeof(testMidi), 0); 
    }
    else if(cc == 'B'){
      LOG("send test bitmap string");
      unsigned char testMidi[] = {
        240,0,127,CMD_GFX,
          0,0,
          8*2,0,
          0xF,0xF,
          0x5,0x5,
          0xA,0xA,
          0x5,0x5,
          0xA,0xA,
          0x5,0x5,
          0xA,0xA,
          0x5,0x5,
        240,0,127,127,0,
          'R',5,0,0,40,40,0,
          'B',8,
          2,2,36,36, //x,y,w,h
          0,0,//sstart
          8,//sh
          16,//sw
        247
      };
      cs.collectMidi(testMidi, sizeof(testMidi), 0); 
    }
    else if(cc == 'L'){
      cs.logDisplayList(0);
    }
  }

  cs.pollDisplays();

  // // auto dt = millis() - tt;
  // // dt /= 1000;
  
  // io_mux_irq();
  // delay(5);
  // // delay(dt);
#endif
}
