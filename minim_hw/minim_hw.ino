#include <libmaple/gpio.h>
#include <libmaple/i2c.h>
#include "libdarray.h"
#include "gfx.h"
#include "io.h"
#include <USBMIDI.h>
#include <USBComposite.h>

uint8 midi_base = 35;
USBMIDI usbmidi;

gfx_t gfx;

int ctx = 0;
int ctx_add = 0;
i2c_dev* ctx_dev;
i2c_msg imsg;
sarray_t<uint8> imsgdata;


void ctx_switch(int c){
    ctx = c;
    if(ctx == 0 || ctx == 1) {
      ctx_add = ctx == 0 ? 0x3C : 0x3D; 
      C_BIT(AFIO_BASE->MAPR,1);
      ctx_dev = I2C1;
    }
    else if(ctx == 2){
      ctx_add = 0x3C; 
      ctx_dev = I2C2;
    } 
    else if(ctx == 3 || ctx == 4) {
      ctx_add = ctx == 3 ? 0x3C : 0x3D; 
      S_BIT(AFIO_BASE->MAPR,1);
      ctx_dev = I2C1;
    }
}

void post_ssd1306(){
  imsg.addr = ctx_add;
  imsg.length = imsgdata.count;
  imsg.data = (uint8*)imsgdata.buf;
  i2c_master_xfer(ctx_dev,&imsg,1,0);
  while(ctx_dev->state == I2C_STATE_BUSY){}
}

void begin_ssd1306(){
    
    //multiplex
    sarray_clear(imsgdata);
    sarray_push(imsgdata,(uint8)0x00);//cmd byte
    sarray_push(imsgdata,(uint8)0xAE);//display off
    sarray_push(imsgdata,(uint8)0xD5);//clock div
    sarray_push(imsgdata,(uint8)0x80);
    sarray_push(imsgdata,(uint8)0xA8);// ration
    sarray_push(imsgdata,(uint8)0x3F);
    post_ssd1306();
    
    sarray_clear(imsgdata);
    sarray_push(imsgdata,(uint8)0x00);//cmd byte
    sarray_push(imsgdata,(uint8)0xD3);//disp offset
    sarray_push(imsgdata,(uint8)0x40);//start line
    sarray_push(imsgdata,(uint8)0x8D);//sharge
    sarray_push(imsgdata,(uint8)0x14);
    post_ssd1306();
 
    sarray_clear(imsgdata);
    sarray_push(imsgdata,(uint8)0x00);
    sarray_push(imsgdata,(uint8)0x20);
    sarray_push(imsgdata,(uint8)0x00);
    sarray_push(imsgdata,(uint8)0xA1);
    sarray_push(imsgdata,(uint8)0xC8);
    post_ssd1306();
    
    sarray_clear(imsgdata);
    sarray_push(imsgdata,(uint8)0x00);//cmd byte
    sarray_push(imsgdata,(uint8)0xDA);//coms pins set
    sarray_push(imsgdata,(uint8)0x12);
    sarray_push(imsgdata,(uint8)0x81);//contrast set
    sarray_push(imsgdata,(uint8)0xCF);
    post_ssd1306();

    sarray_clear(imsgdata);
    sarray_push(imsgdata,(uint8)0x00);//cmd byte
    sarray_push(imsgdata,(uint8)0xd9);//precharge
    sarray_push(imsgdata,(uint8)0xF1);
    post_ssd1306();

    sarray_clear(imsgdata);
    sarray_push(imsgdata,(uint8)0x00);//cmd byte
    sarray_push(imsgdata,(uint8)0x21);//col add
    sarray_push(imsgdata,(uint8)0x00);
    sarray_push(imsgdata,(uint8)0x7F);
    sarray_push(imsgdata,(uint8)0x22);//page add
    sarray_push(imsgdata,(uint8)0x00);
    sarray_push(imsgdata,(uint8)0x07);
    post_ssd1306();
    
    sarray_clear(imsgdata);
    sarray_push(imsgdata,(uint8)0x00);//cmd byte
    sarray_push(imsgdata,(uint8)0xD8);
    sarray_push(imsgdata,(uint8)0x40);
    sarray_push(imsgdata,(uint8)0xA4);
    sarray_push(imsgdata,(uint8)0xA6);
    sarray_push(imsgdata,(uint8)0x2E);
    sarray_push(imsgdata,(uint8)0xAF);
    post_ssd1306();
    
}

void draw_ssd1306(){

  sarray_clear(imsgdata);
    sarray_push(imsgdata,(uint8)0x00);//cmd byte
    sarray_push(imsgdata,(uint8)0x21);//col add
    sarray_push(imsgdata,(uint8)0x00);
    sarray_push(imsgdata,(uint8)0x7F);
    sarray_push(imsgdata,(uint8)0x22);//page add
    sarray_push(imsgdata,(uint8)0x00);
    sarray_push(imsgdata,(uint8)0x07);
  post_ssd1306();
  
  for(int j=0; j<8; j++){
    auto bb = j<4 ? gfx.fbuf_top : gfx.fbuf_bot;
    auto pp = (j%4)*8;
    sarray_clear(imsgdata);
      sarray_push(imsgdata,(uint8)0x40);//cmd byte
      for(int d=0; d<128; d++){
        uint32_t dd = bb[d];
        dd &= (0xff<<pp);
        dd >>= pp;
        sarray_push(imsgdata,(uint8)dd);
      }
      post_ssd1306();
  }
}

void begin_gpio(){
  afio_init();
  afio_cfg_debug_ports(AFIO_DEBUG_NONE);
  gpio_set_mode(GPIOB,7,GPIO_AF_OUTPUT_OD);
  gpio_set_mode(GPIOB,6,GPIO_AF_OUTPUT_OD);
  gpio_set_mode(GPIOB,11,GPIO_AF_OUTPUT_OD);
  gpio_set_mode(GPIOB,10,GPIO_AF_OUTPUT_OD);
  gpio_set_mode(GPIOB,9,GPIO_AF_OUTPUT_OD);
  gpio_set_mode(GPIOB,8,GPIO_AF_OUTPUT_OD);
}

void setup(){
  
    USBComposite.setProductId(0x0030);
    // put your setup code here, to run once:
    Serial.begin(115200);
    usbmidi.begin();

    imsgdata.buf = (uint8*)malloc(sizeof(uint8)*(2 + 128*8));
    imsgdata.lim = 130;
    sarray_clear(imsgdata);
    
    begin_gpio();
    io_mux_init();

    i2c_init(I2C1);
    i2c_init(I2C2);
    i2c_master_enable(I2C1, I2C_FAST_MODE, 1000000);
    i2c_master_enable(I2C2, I2C_FAST_MODE, 1000000);
    // i2c_peripheral_enable(I2C1);
    // i2c_peripheral_enable(I2C2);
    delay(200);
    
    uint32_t pat[9] = {
      0b10000000000000001111111111111111,
      0b11111111111111110000000110110000,
      0b10000000000000000000001010001100,
      0b10000000000000000000010010000011,
      0b10000000000000000000000000000000,
      0b10000000000000000000000000000000,
      0b10000000000000000000000000000000,
      0b10000000000000000000000000000000,
      0b10000000000000000000000000000000
    };
    //gfx_fillSection(2,30,2,30,1);

    for(int i=0; i<5; i++){
      ctx_switch(i);
      begin_ssd1306();
      gfx.rotated = 1;
      gfx_clear();
      gfx_drawRectSize(0,0,64,128);
      gfx_drawLine(0,0,20,10);
      gfx_drawBitmap32(20, 10, 15, 64, i+1, 9, pat);
      draw_ssd1306();
      delay(100);
    }
    

    Serial.println("ready");
    while(1){}
} 

bool states[2][5];

void loop(){
  if(io.bscan_down){
    auto n = midi_base;
    for(int i=0; i<10; i++){
      if((1<<i)&io.bscan_down){
        n+=i;
        break;
      }
    }
    usbmidi.sendNoteOn(0,n,127);
    io.bscan_down = 0;
  }
  if(io.bscan_up){
    auto n = midi_base;
    for(int i=0; i<10; i++){
      if((1<<i)&io.bscan_up){
        n+=i;
        break;
      }
    }
    usbmidi.sendNoteOn(0,n,0);
    io.bscan_up = 0;
  }

  if(int a = usbmidi.available()){
    uint32 aa = usbmidi.readPacket();
    char *b = (char*)&aa;
    Serial.println(b[1],DEC);
    Serial.println(b[2],DEC);
    Serial.println(b[3],DEC);

    if(b[1] == 0x90){
      int ctx = b[2]-midi_base;
      bool bot = (ctx < 5);
      ctx %= 5;

      Serial.println(ctx,DEC);
      Serial.println(bot,DEC);
      states[1-bot][ctx] = b[3];
      gfx.rotated = 1;
      gfx_clear();
      gfx_drawRectSize(0,0,64,128);
      if(states[1][ctx])
        gfx_fillSection(4,56,4,56,1);
      if(states[0][ctx])
        gfx_fillSection(4+64,56,4,56,1);
      ctx_switch(ctx%5);
      draw_ssd1306();
    }
    Serial.println("----");
  }

  delay(1);
}


//#include <USBComposite.h>
//USBMIDI midi;
//int leds[4] = {
//  PB12,PB14,PB15,PA8
//};
//int pins[9] = {
//  PA0,PA1,PA2,PA3,PA4,PA5,PA6,PA7,PB0
//};
//struct Button {
//    byte inreg = 0;
//    byte inregOld = 0;
//    byte changeType = 0;
//    byte changeTypeOld = 0;
//    long timeOld = 0;
//    long timeOldDouble = 0;
//    long timeDelta = 0;
//    long timeDeltaDouble = 0;
//} buttonData[9]; 
//
//enum OpMode {
//  SIMPLE, //basic midi controller
//  SIMPLE2,
//  REALTIME,
//  BUFFERED,
//  MOTIF
//};
//OpMode currentMode = SIMPLE;
//
//#define DT 20 //time pause per update in ms
//#define DOUBLE_PRESS_TIME 100
//#define HOLD_TIME 350
//#define HOLD_TIME_LONG 1000
//#define HOLD_TIME_VERY_LONG 5000
//#define RELEASE 0
//#define PRESS_UP RELEASE
//#define PRESS_DOWN 1
//#define PRESS_DOUBLE 2
//#define PRESS 3
//#define HOLD_SHORT 4
//#define HOLD_LONG 5
//#define HOLD_VERY_LONG 6
//long timeCounter = 0l;
//
//#define BASE_NOTE 60
//
//void setup() {
//  // put your setup code here, to run once:
//  Serial.begin(9600);
//  Serial.println("   ");
//  Serial.print("Setup begin...");
//  for(int i=0; i<9; i++){
//    pinMode(pins[i],INPUT_PULLUP);
//  }
//  for(int i=0; i<4; i++){
//    pinMode(leds[i],OUTPUT);
//  }
//
//  USBComposite.setVendorId(0x4d54); //ven "MT"
//  USBComposite.setProductId(0x0001);
//  USBComposite.setProductString("M O T I F - Foot Pad 8");
//  midi.begin();
//
//  Serial.println("done");
//
//  int seq[8][4] = {
//    {1,0,0,0},
//    {0,1,0,0},
//    {0,0,1,0},
//    {0,0,0,1},
//    {0,0,0,1},
//    {0,0,1,0},
//    {0,1,0,0},
//    {1,0,0,0},  
//  };
//  
//  for(int s=0; s<8; s++){
//  digitalWrite(leds[0],seq[s][0]);
//  digitalWrite(leds[1],seq[s][1]);
//  digitalWrite(leds[2],seq[s][2]);
//  digitalWrite(leds[3],seq[s][3]);
//  delay(50);
//  }
//}
//
//void loop() {
//  // put your main code here, to run repeatedly:
//  for(int I=0; I<9; I++){
//  
//    buttonData[I].changeTypeOld = buttonData[I].changeType;
//    buttonData[I].inregOld = buttonData[I].inreg;
//    buttonData[I].inreg = !digitalRead(pins[I]);
//    
//    //button DOWN event
//    if(buttonData[I].inreg && !buttonData[I].inregOld){
//        
//        buttonData[I].timeOld = timeCounter;
//        buttonData[I].timeDeltaDouble = buttonData[I].timeOld - buttonData[I].timeOldDouble;
//        if(buttonData[I].timeDeltaDouble < DOUBLE_PRESS_TIME and I > 6){
//            buttonData[I].changeType = PRESS_DOUBLE;
//            buttonData[I].timeOldDouble = 0;
//        }
//        else{
//            buttonData[I].changeType = PRESS_DOWN;
//        }
//    }
//    
//    //button HOLD event
//    else if(buttonData[I].inreg && buttonData[I].inregOld && buttonData[I].changeType != PRESS_DOUBLE){
//        buttonData[I].timeDelta = timeCounter - buttonData[I].timeOld;
//        
//        if(buttonData[I].timeDelta > HOLD_TIME_VERY_LONG) buttonData[I].changeType = HOLD_VERY_LONG;
//        else if(buttonData[I].timeDelta > HOLD_TIME_LONG) buttonData[I].changeType = HOLD_LONG;
//        else if(buttonData[I].timeDelta > HOLD_TIME) buttonData[I].changeType = HOLD_SHORT;
//        else 
//        {
//            buttonData[I].changeType = PRESS; 
//            buttonData[I].timeOldDouble = timeCounter;
//        }
//    
//    }
//    
//    //button UP event
//    if(!buttonData[I].inreg && buttonData[I].inregOld){
//        buttonData[I].timeDelta = 0;
//        buttonData[I].changeTypeOld = buttonData[I].changeType;
//        buttonData[I].changeType = PRESS_UP;
//    }
//
//
//
//    if(buttonData[I].changeType != buttonData[I].changeTypeOld){
//
//      if (I == 8){
//        if (buttonData[I].changeType == HOLD_SHORT){
//            currentMode = SIMPLE;
//        }
//        else if (buttonData[I].changeType == HOLD_VERY_LONG){
//            if (currentMode != MOTIF) {
//            currentMode = MOTIF;
//            }
//        }
//        else if (buttonData[I].changeType == PRESS){
//          switch (currentMode) {
//            case SIMPLE:
//              currentMode = SIMPLE2;
//            break;
//            
//            case SIMPLE2:
//              currentMode = REALTIME;
//            break;
//            
//            case REALTIME:
//              currentMode = BUFFERED;
//            break;
//            
//            case BUFFERED:
//              currentMode = SIMPLE;
//            break;
//
//            default:
//              currentMode = SIMPLE;
//              break;
//          }
//        }
//
//        if(buttonData[I].changeType != PRESS_UP){
//          const int modes[5][4] = {
//            {1,0,0,0},
//            {0,1,0,0},
//            {0,0,1,0},
//            {0,0,0,1},
//            {1,1,1,1}
//          };
//          
//          digitalWrite(leds[0],modes[currentMode][0]);
//          digitalWrite(leds[1],modes[currentMode][1]);
//          digitalWrite(leds[2],modes[currentMode][2]);
//          digitalWrite(leds[3],modes[currentMode][3]);
//          
//          Serial.print("current mode:");
//          Serial.println(currentMode);
//        }
//      }
//      else if (currentMode == SIMPLE or currentMode == SIMPLE2) {
//        
//        int N = (currentMode == SIMPLE ? BASE_NOTE+I : BASE_NOTE+I+12);
//        
//        switch(buttonData[I].changeType){
//            case PRESS_UP:        
//                midi.sendNoteOff(0,N,0);
//                Serial.print("note off ");
//                Serial.print(N);
//                Serial.print(" @");
//                Serial.println(I);
//             break;
//    
//            case PRESS_DOWN: 
//                midi.sendNoteOn(0,N,127);
//                Serial.print("note on ");
//                Serial.print(N);
//                Serial.print(" @");
//                Serial.println(I);
//            break;
//        }
//      }
//      
//      else if (currentMode == REALTIME) {
//          if(buttonData[I].changeType != PRESS_DOWN and buttonData[I].changeType != PRESS_UP){
//              midi.sendNoteOn(I,BASE_NOTE+buttonData[I].changeType,127);
//              midi.sendNoteOff(I,BASE_NOTE+buttonData[I].changeType,0);
//              Serial.print("RT Change -> ");
//              Serial.print(I); 
//              Serial.print(" ");
//              Serial.print(BASE_NOTE+I); 
//              Serial.print(" ");
//              Serial.println(buttonData[I].changeType);
//          }
//      }
//      
//      else if (currentMode == BUFFERED) {
//
//        if(buttonData[I].changeType == PRESS_UP){
//            midi.sendNoteOn(I,BASE_NOTE+buttonData[I].changeTypeOld,127);
//              midi.sendNoteOff(I,BASE_NOTE+buttonData[I].changeTypeOld,0);
//            Serial.print("BF Change -> ");
//            Serial.print(I);
//            Serial.print(" ");
//            Serial.println(buttonData[I].changeTypeOld);
//        }
//      }
//      
//      else if (currentMode == MOTIF) {
//
//        midi.sendVelocityChange(1,BASE_NOTE+I,buttonData[I].changeType);
//        Serial.print("MOTIF RT Change -> ");
//        Serial.print(" ");
//        Serial.print(I);
//        Serial.print(" ");
//        Serial.println(buttonData[I].changeType);
//        
//        switch(buttonData[I].changeType){
//            case PRESS_UP: 
//                midi.sendVelocityChange(0,BASE_NOTE+I,buttonData[I].changeTypeOld);
//                Serial.print("MOTIF Last RT Change -> ");
//                Serial.print(I);
//                Serial.print(" ");
//                Serial.println(buttonData[I].changeTypeOld);
//                
//                midi.sendNoteOff(0,BASE_NOTE+I,0);
//                Serial.print("MOTIF note off ");
//                Serial.println(I);
//             break;
//    
//            case PRESS_DOWN: 
//               
//                midi.sendNoteOn(0,BASE_NOTE+I,127);
//                Serial.print("MOTIF note on ");
//                Serial.println(I);
//            break;
//        }
//      }
//      
//    }
//  }
//
//  timeCounter += DT;
//  delay(DT);
//}
