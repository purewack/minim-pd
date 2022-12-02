#include "include/libdarray.h"
#include "include/io.h"
#include "api/cmd.h"
#include "api/cmd.cpp"
#include "api/gfx.h"
#include "api/gfx.cpp"
#include <libmaple/gpio.h>
#include <libmaple/i2c.h>
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

sarray_t<char> sysex_string;
bool states[2][5];
bool sysex = false;

void cmd_gfx_on_context(int c){
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

void cmd_gfx_on_draw(){

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

    data_buf = (uint8_t*)malloc(sizeof(uint8_t)*512);

    sysex_string.buf = (char*)malloc(sizeof(char)*128);
    sysex_string.lim = 128;
    sarray_clear(sysex_string);
    sysex = false;

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

    gfx_defaults();
    gfx_clear();
    //gfx_drawRectSize(0,0,64,128);
    for(int i=0; i<5; i++){
      cmd_gfx_on_context(i);
      begin_ssd1306();
      cmd_gfx_on_draw();
      delay(100);
    }

    Serial.println("ready");
} 


void collectSysex(char* b, int offset){
  
  // Serial.println("[");
  // Serial.println(b[0],DEC);
  //   Serial.println(b[1],DEC);
  //   Serial.println(b[2],DEC);
  //   Serial.println(b[3],DEC);
  // Serial.println("]");
  for(int i = offset; i<3; i++){
    if(b[1+i] == 0xf7 && sysex){
      sysex = false;
      usbmidi.sendControlChange(0,127,1);
      parseCommand((unsigned char*)sysex_string.buf,sysex_string.count);
      usbmidi.sendControlChange(0,127,0);
    }
    else
      sarray_push(sysex_string,b[1+i]);
  }
}

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
    // Serial.println(b[1],DEC);
    // Serial.println(b[2],DEC);
    // Serial.println(b[3],DEC);

    if(!sysex){
      if(b[1] == 0x90){
        int ctx = b[2]-midi_base;
        bool bot = (ctx < 5);
        ctx %= 5;

        states[1-bot][ctx] = b[3];
        gfx.rotated = 1;
        gfx_clear();
        gfx_drawRectSize(0,0,64,128);
        if(states[1][ctx])
          gfx_fillSection(4,4,56,56);
        if(states[0][ctx])
          gfx_fillSection(4,4+64,56,56);
        cmd_gfx_on_context(ctx%5);
        cmd_gfx_on_draw();
      }
      else if(b[1] == 0xf0 && !sysex){
        sysex = true;
        sarray_clear(sysex_string);
        collectSysex(b, 1);
      }
    }
    else{
      collectSysex(b, 0);
    }
    
    // Serial.println("----");
  }

  delay(1);
}
