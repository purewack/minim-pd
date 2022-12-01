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

sarray_t<char> sysex_string;
bool states[2][5];
bool sysex = false;
const int MODE_SYS = 1;
const int MODE_GFX = 2;
const int MODE_DATA = 3;
int cmd_mode = 0;

uint8_t data_buf[512];

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

    gfx.modexor = 1;
    gfx.rotated = 1;
    gfx_clear();
    //gfx_drawRectSize(0,0,64,128);
    for(int i=0; i<5; i++){
      ctx_switch(i);
      begin_ssd1306();
      draw_ssd1306();
      delay(100);
    }

    Serial.println("ready");
} 



int parseCommand(const char* cmd_bytes, int len){
    for(int i=0; i<len; i++){
        if(cmd_bytes[i] == 'x'){
        	cmd_mode = cmd_bytes[++i];
        }
        if(cmd_mode == 0){
          	draw_ssd1306();
        }
        else if(cmd_mode == MODE_DATA){
          if(cmd_bytes[i] == 'l'){
              int start = cmd_bytes[++i];
              int bytes = cmd_bytes[++i];
              int nibbles = bytes*2;
              Serial.println("loading LE_bitmap into");
              Serial.println(start);
              Serial.println(nibbles);
              Serial.println(bytes);
              const char* buf = &cmd_bytes[++i];
              if(start+bytes < 512){
                  int d = 0;
                  for(int j=0; j<nibbles; j+=2){
                    data_buf[start+d] = uint8_t(buf[j+0]) << 0;
                    data_buf[start+d] |= uint8_t(buf[j+1]) << 4;
                    d++;
                  }
                Serial.println("loaded LE_bitmap data");
                for(int i=0; i<bytes; i++)
                  Serial.println(data_buf[start+i],HEX);
              }
              i+=nibbles;
          }
        }
        else if(cmd_mode == MODE_GFX){
            if(cmd_bytes[i] == 'G'){
                int ctx = cmd_bytes[++i];
                ctx_switch(ctx);
            }
            else if(cmd_bytes[i] == 'S'){
                int scale = cmd_bytes[++i];
                gfx.scale = scale; 
            }
			      else if(cmd_bytes[i] == 'I'){
                int m = cmd_bytes[++i];;
                gfx.modexor = m; 
            }

            else if(cmd_bytes[i] == 'c'){
                gfx_clear();
            }
            else if(cmd_bytes[i] == 'u'){
                draw_ssd1306();
            }

            else if(cmd_bytes[i] == 'l'){
                int x = cmd_bytes[++i];
                int y = cmd_bytes[++i];
                int x2 = cmd_bytes[++i];
                int y2 = cmd_bytes[++i];
                gfx_drawLine(x,y,x2,y2);
            }
            else if(cmd_bytes[i] == 'r'){
                int x = cmd_bytes[++i];
                int y = cmd_bytes[++i];
                int w = cmd_bytes[++i];
                int h = cmd_bytes[++i];
                int fill = cmd_bytes[++i];
                if(fill)
                  gfx_fillSection(x,y,w,h);
                else
                  gfx_drawRectSize(x,y,w,h);
                
            }
            else if(cmd_bytes[i] == 's'){
                int x = cmd_bytes[++i];
                int y = cmd_bytes[++i];
                const char* str = &cmd_bytes[++i];
                gfx_drawString(str,x,y); 
                int j = 0;
                while(cmd_bytes[i+j] != 0) j++;
                i+=j;
            }
            else if(cmd_bytes[i] == 'b'){
                int x = cmd_bytes[++i];
                int y = cmd_bytes[++i];
                int nw = cmd_bytes[++i];
                int w = cmd_bytes[++i];
                int nh = cmd_bytes[++i];
                int h = cmd_bytes[++i];
                int start = cmd_bytes[++i];
                int bytes_per_col = cmd_bytes[++i];
                int len = cmd_bytes[++i];
                if(nw) w = -w;
                if(nh) h = -h;
                Serial.println("drawing bitmap ");
                Serial.println(start);
                Serial.println(bytes_per_col);
                Serial.println(len);
                gfx_drawBitmap(x,y,w,h,bytes_per_col,len,data_buf+start);
            }
        }
        else{
            return -1;
        }
    }

    return 0;
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
      parseCommand(sysex_string.buf,sysex_string.count);
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
        ctx_switch(ctx%5);
        draw_ssd1306();
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
