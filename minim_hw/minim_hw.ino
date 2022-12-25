#include "include/libdarray.h"
#include "include/io.h"
#include "include/common.h"
#include "api/api.h"
#include <libmaple/gpio.h>
#include <libmaple/i2c.h>
#include <libmaple/spi.h>
#include <USBMIDI.h>
#include <USBComposite.h>
#include <USBCompositeSerial.h>
#include <EEPROM.h>

uint8 midi_base = 35;
USBMIDI usbmidi;
USBCompositeSerial logger;

gfx_t gfx;
int ctx = 0;
int ctx_add = 0;
i2c_dev* ctx_dev;
i2c_msg imsg;
sarray_t<uint8> imsgdata;

sarray_t<char> sysex_string;
bool states[2][5];
bool sysex = false;

int boot_byte_count = 0;

void cmd_sys_on_upload_boot_begin(){
  Serial.println("begin boot cmd upload");
  boot_byte_count = 0;
}

void cmd_sys_on_upload_boot_byte(unsigned char byte){
  EEPROM.write(1+boot_byte_count,byte);
  boot_byte_count++;
}

void cmd_sys_on_upload_boot_end(){
  Serial.println("begin boot cmd upload");
  EEPROM.write(0,boot_byte_count);
  Serial.println("recieved bytes");
  Serial.println(boot_byte_count);
}

void cmd_sys_on_sleep(int ms){
  delay(ms);
}

void cmd_gfx_on_context(int c){
    ctx = c;
    gfx.rotated = ctx == 5 ? 0 : 1;
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

void post_display(){
  if(ctx == 5){
    gpio_write_bit(GPIOB, 12, 0);
    spi_tx(SPI2, imsgdata.buf, imsgdata.count);
    while(spi_is_busy(SPI2)){
      delay(2);
    }
    gpio_write_bit(GPIOB, 12, 1);
    io_mux_irq();
    return;
  }
  
  imsg.addr = ctx_add;
  imsg.length = imsgdata.count;
  imsg.data = (uint8*)imsgdata.buf;
  i2c_master_xfer(ctx_dev,&imsg,1,0);
  while(ctx_dev->state == I2C_STATE_BUSY){
    io_mux_irq();
    delay(2);
  }
  
}

void begin_oled(){
    
    //multiplex
    sarray_clear(imsgdata);
    if(ctx != 5)
      sarray_push(imsgdata,(uint8)0x00);//cmd byte
    sarray_push(imsgdata,(uint8)0xAE);//display off
    sarray_push(imsgdata,(uint8)0xD5);//clock div
    sarray_push(imsgdata,(uint8)0x80);
    sarray_push(imsgdata,(uint8)0xA8);// ration
    sarray_push(imsgdata,(uint8)0x3F);
    post_display();
    
    if(ctx != 5){
    sarray_clear(imsgdata);
      sarray_push(imsgdata,(uint8)0x00);//cmd byte
      sarray_push(imsgdata,(uint8)0xD3);//disp offset
      sarray_push(imsgdata,(uint8)0x40);//start line
      sarray_push(imsgdata,(uint8)0x8D);//sharge
      sarray_push(imsgdata,(uint8)0x14);
    post_display();
    }
 
    sarray_clear(imsgdata);
    if(ctx != 5)
      sarray_push(imsgdata,(uint8)0x00);
    sarray_push(imsgdata,(uint8)0x20);
    sarray_push(imsgdata,(uint8)0x00);
    sarray_push(imsgdata,(uint8)0xA1);
    sarray_push(imsgdata,(uint8)0xC8);
    post_display();
    
    sarray_clear(imsgdata);
    if(ctx != 5)
      sarray_push(imsgdata,(uint8)0x00);//cmd byte
    sarray_push(imsgdata,(uint8)0xDA);//coms pins set
    sarray_push(imsgdata,(uint8)0x12);
    sarray_push(imsgdata,(uint8)0x81);//contrast set
    sarray_push(imsgdata,(uint8)0xCF);
    post_display();

    sarray_clear(imsgdata);
    if(ctx != 5)
      sarray_push(imsgdata,(uint8)0x00);//cmd byte
    sarray_push(imsgdata,(uint8)0xd9);//precharge
    sarray_push(imsgdata,(uint8)0xF1);
    post_display();

    sarray_clear(imsgdata);
    if(ctx != 5)
      sarray_push(imsgdata,(uint8)0x00);//cmd byte
    sarray_push(imsgdata,(uint8)0x21);//col add
    sarray_push(imsgdata,(uint8)0x00);
    sarray_push(imsgdata,(uint8)0x7F);
    sarray_push(imsgdata,(uint8)0x22);//page add
    sarray_push(imsgdata,(uint8)0x00);
    sarray_push(imsgdata,(uint8)0x07);
    post_display();
    
    sarray_clear(imsgdata);
    if(ctx != 5)
      sarray_push(imsgdata,(uint8)0x00);//cmd byte
    sarray_push(imsgdata,(uint8)0xD8);
    sarray_push(imsgdata,(uint8)0x40);
    sarray_push(imsgdata,(uint8)0xA4);
    sarray_push(imsgdata,(uint8)0xA6);
    sarray_push(imsgdata,(uint8)0x2E);
    sarray_push(imsgdata,(uint8)0xAF);
    post_display();
    
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
  post_display();
  
  for(int j=0; j<8; j++){
    auto bb = j<4 ? gfx.fbuf_top : gfx.fbuf_bot;
    auto pp = (j%4)*8;
    sarray_clear(imsgdata);
      if(ctx != 5)
        sarray_push(imsgdata,(uint8)0x40);//cmd byte

      for(int d=0; d<128; d++){
        uint32_t dd = bb[d];
        dd &= (0xff<<pp);
        dd >>= pp;
        sarray_push(imsgdata,(uint8)dd);
      }

      if(ctx == 5){
        gpio_write_bit(GPIOB,14,1);
        post_display();
        gpio_write_bit(GPIOB,14,0);
      }
      else
        post_display();
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

  gpio_set_mode(GPIOB, 12, GPIO_OUTPUT_PP); //cs
  gpio_set_mode(GPIOB, 13, GPIO_AF_OUTPUT_PP); //scl
  gpio_set_mode(GPIOB, 14, GPIO_OUTPUT_PP); //dc
  gpio_set_mode(GPIOB, 15, GPIO_AF_OUTPUT_PP); //sda
  gpio_set_mode(GPIOA, 8, GPIO_OUTPUT_PP); //rst

  //cs
  gpio_write_bit(GPIOB, 12, 1);
  //dc
  gpio_write_bit(GPIOB, 14, 0);
  //rst
  gpio_write_bit(GPIOA, 8, 1);
  delay(10);
  gpio_write_bit(GPIOA, 8, 0);
  delay(10);
  gpio_write_bit(GPIOA, 8, 1);
}



void setup(){
  
    USBComposite.clear();
    USBComposite.setProductId(0x0031);
    USBComposite.setManufacturerString("M O T I F");
    USBComposite.setProductString("MINIM-5-1");
    usbmidi.registerComponent();
    logger.registerComponent();
    USBComposite.begin();
    Serial.begin(31250);

    data_buf = (uint8_t*)malloc(sizeof(uint8_t)*512);

    sysex_string.buf = (char*)malloc(sizeof(char)*1024);
    sysex_string.lim = 1024;
    sarray_clear(sysex_string);
    sysex = false;

    imsgdata.buf = (uint8*)malloc(sizeof(uint8)*(2 + 128*8));
    imsgdata.lim = 130;
    sarray_clear(imsgdata);
    
    spi_init(SPI2);
    spi_init(SPI2);
    spi_master_enable(SPI2, 
      SPI_BAUD_PCLK_DIV_8, 
      SPI_MODE_0, 
      SPI_FRAME_MSB | SPI_DFF_8_BIT | SPI_SW_SLAVE | SPI_SOFT_SS 
    );
    
    begin_gpio();
    io_mux_init();
    timer_pause(TIMER3);

    i2c_init(I2C1);
    i2c_init(I2C2);
    i2c_master_enable(I2C1, I2C_FAST_MODE, 1000000);
    i2c_master_enable(I2C2, I2C_FAST_MODE, 1000000);
    
    delay(200);

    gfx_defaults();
    gfx_clear();
    for(int i=0; i<6; i++){
      cmd_gfx_on_context(i);
      begin_oled();
      cmd_gfx_on_draw();
      delay(100);
    }

    cmd_gfx_on_context(5);
    gfx.modexor = 1;
    gfx.scale = 1;
    char version_str[32];
    snprintf(version_str,32,"%s - API:%d",VERSION,MINIM_API_VER);
    for(int i=0; i<32; i++){
      gfx_clear();
      gfx_drawString(version_str,0,64-8);
      gfx_fillSection(0,64-8,i*4,8);
      cmd_gfx_on_draw();
      delay(1);
    }
    gfx_clear();
    cmd_gfx_on_draw();
        
    int boot_cmd = EEPROM.read(0) ;
    if(boot_cmd != 0xffff && boot_cmd){
      for(int i=0; i<boot_cmd; i++)
        sarray_push(sysex_string,(char)EEPROM.read(i+1));
      parseCommand((unsigned char*)sysex_string.buf,sysex_string.count);
      sarray_clear(sysex_string);
    }
    else
      logger.println("no boot info found");

    logger.println("ready");
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
      parseCommand((unsigned char*)sysex_string.buf,sysex_string.count);
    }
    else
      sarray_push(sysex_string,b[1+i]);
  }
}

void loop(){
  // auto tt = millis();
  if(int a = usbmidi.available()){
    uint32_t aa = usbmidi.readPacket();
    char *b = (char*)&aa;

    if(!sysex){
      if(b[1] &= 0xB0){
        parseVariable( b[1]&0x0f, b[2], b[3]);
      }
      else if(b[1] == 0xf0 && !sysex){
        sysex = true;
        sarray_clear(sysex_string);
        collectSysex(b, 1);
      }
    }
    else if(sysex){
      collectSysex(b, 0);
    }
  }

  for(int f=0; f<6; f++){
    if(frames[f].isFramed)
      parseCommand(frames[f].cmd_bytes,frames[f].cmd_count);
  }

  // auto dt = millis() - tt;
  // dt /= 1000;

  io_mux_irq();
  delay(5);
  io_mux_irq();
  delay(5);
  io_mux_irq();
  delay(5);
  io_mux_irq();
  delay(4);
  // delay(dt);
}
