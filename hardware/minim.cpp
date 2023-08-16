#include <libmaple/gpio.h>
#include <libmaple/i2c.h>
#include <libmaple/spi.h>
#include "include/common.h"
#include "include/util.h"
#include "api/gfx/fonttiny.h"
#include "api/gfx/fonttiny.cpp"

bool MINIM::ControlSurface::isSysex(){
  return sysex;
}

void MINIM::ControlSurface::initDisplays(){
    spi_init(SPI2);
    spi_master_enable(
      SPI2, 
      SPI_BAUD_PCLK_DIV_8, 
      SPI_MODE_0, 
      SPI_FRAME_MSB | SPI_DFF_8_BIT | SPI_SW_SLAVE | SPI_SOFT_SS 
    );

    i2c_init(I2C1);
    i2c_init(I2C2);
    i2c_master_enable(
      I2C1, 
      I2C_FAST_MODE, 
      1000000
    );
    i2c_master_enable(
      I2C2, 
      I2C_FAST_MODE, 
      1000000
    );
    
    delay(50);

    gfx.clear();

    for(int i=0; i<CONTEXT_MAX; i++){
      context = i;
      beginOledSingle();
      populateContextBuffer();
      delay(50);
    }
    context = 0;
}

void MINIM::ControlSurface::initMemory(){
  gfx.setFont(fonttiny_wide, fonttiny_tall, fonttiny_data);

  // data_buf = (uint8_t*)malloc(sizeof(uint8_t)*512);

  midiString.buf = (uint8_t*)malloc(sizeof(uint8_t)*CMD_BYTE_COUNT_MAX);
  midiString.lim = CMD_BYTE_COUNT_MAX;
  sarray_clear(midiString);

  displayCommands.buf = (uint8_t*)malloc(sizeof(uint8_t)*(130));
  displayCommands.lim = 130;
  sarray_clear(displayCommands);
    
}

void MINIM::ControlSurface::initGPIO(){
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

void MINIM::ControlSurface::beginOledSingle(){ 
    //multiplex
    sarray_clear(displayCommands);
    if(context != 0)
      sarray_push(displayCommands,(uint8_t)0x00);//cmd byte
    sarray_push(displayCommands,(uint8_t)0xAE);//display off
    sarray_push(displayCommands,(uint8_t)0xD5);//clock div
    sarray_push(displayCommands,(uint8_t)0x80);
    sarray_push(displayCommands,(uint8_t)0xA8);// ration
    sarray_push(displayCommands,(uint8_t)0x3F);
    postToDisplays();
    
    if(context != 0){
    sarray_clear(displayCommands);
      sarray_push(displayCommands,(uint8_t)0x00);//cmd byte
      sarray_push(displayCommands,(uint8_t)0xD3);//disp offset
      sarray_push(displayCommands,(uint8_t)0x40);//start line
      sarray_push(displayCommands,(uint8_t)0x8D);//sharge
      sarray_push(displayCommands,(uint8_t)0x14);
    postToDisplays();
    }
 
    sarray_clear(displayCommands);
    if(context != 0)
      sarray_push(displayCommands,(uint8_t)0x00);
    sarray_push(displayCommands,(uint8_t)0x20);
    sarray_push(displayCommands,(uint8_t)0x00);
    sarray_push(displayCommands,(uint8_t)0xA1);
    sarray_push(displayCommands,(uint8_t)0xC8);
    postToDisplays();
    
    sarray_clear(displayCommands);
    if(context != 0)
      sarray_push(displayCommands,(uint8_t)0x00);//cmd byte
    sarray_push(displayCommands,(uint8_t)0xDA);//coms pins set
    sarray_push(displayCommands,(uint8_t)0x12);
    sarray_push(displayCommands,(uint8_t)0x81);//contrast set
    sarray_push(displayCommands,(uint8_t)0xCF);
    postToDisplays();

    sarray_clear(displayCommands);
    if(context != 0)
      sarray_push(displayCommands,(uint8_t)0x00);//cmd byte
    sarray_push(displayCommands,(uint8_t)0xd9);//precharge
    sarray_push(displayCommands,(uint8_t)0xF1);
    postToDisplays();

    sarray_clear(displayCommands);
    if(context != 0)
      sarray_push(displayCommands,(uint8_t)0x00);//cmd byte
    sarray_push(displayCommands,(uint8_t)0x21);//col add
    sarray_push(displayCommands,(uint8_t)0x00);
    sarray_push(displayCommands,(uint8_t)0x7F);
    sarray_push(displayCommands,(uint8_t)0x22);//page add
    sarray_push(displayCommands,(uint8_t)0x00);
    sarray_push(displayCommands,(uint8_t)0x07);
    postToDisplays();
    
    sarray_clear(displayCommands);
    if(context != 0)
      sarray_push(displayCommands,(uint8_t)0x00);//cmd byte
    sarray_push(displayCommands,(uint8_t)0xD8);
    sarray_push(displayCommands,(uint8_t)0x40);
    sarray_push(displayCommands,(uint8_t)0xA4);
    sarray_push(displayCommands,(uint8_t)0xA6);
    sarray_push(displayCommands,(uint8_t)0x2E);
    sarray_push(displayCommands,(uint8_t)0xAF);
    postToDisplays();
    
}


void MINIM::ControlSurface::populateContextBuffer(){

  sarray_clear(displayCommands);
    sarray_push(displayCommands,(uint8_t)0x00);//cmd byte
    sarray_push(displayCommands,(uint8_t)0x21);//col add
    sarray_push(displayCommands,(uint8_t)0x00);
    sarray_push(displayCommands,(uint8_t)0x7F);
    sarray_push(displayCommands,(uint8_t)0x22);//page add
    sarray_push(displayCommands,(uint8_t)0x00);
    sarray_push(displayCommands,(uint8_t)0x07);
  postToDisplays();
  
  for(int page=0; page<8; page++){
    auto bb = page<4 ? gfx.fbuf_top : gfx.fbuf_bot;
    auto pp = (page%4)*8;
    sarray_clear(displayCommands);
    if(context != 0)
      sarray_push(displayCommands,(uint8_t)0x40);//cmd byte

    for(int column=0; column<128; column++){
      uint32_t dd = bb[column];
      dd &= (0xff<<pp);
      dd >>= pp;
      sarray_push(displayCommands,(uint8_t)dd);
    }

    if(context == 0){
      gpio_write_bit(GPIOB,14,1);
      postToDisplays();
      gpio_write_bit(GPIOB,14,0);
    }
    else
      postToDisplays();
  }
}

void MINIM::ControlSurface::postToDisplays(){

  if(context == 0){
    gpio_write_bit(GPIOB, 12, 0);
    spi_tx(SPI2, displayCommands.buf, displayCommands.count);
    while(spi_is_busy(SPI2)){
      delay(2);
    }
    gpio_write_bit(GPIOB, 12, 1);
    return;
  }

  if(context == 1 || context == 2) {
    contextAddress = context == 1 ? 0x3C : 0x3D;
    contextDevice = I2C1; 
    C_BIT(AFIO_BASE->MAPR,1);
  }
  else if(context == 3){
    contextAddress = 0x3C; 
    contextDevice = I2C2;
  } 
  else if(context == 4 || context == 5) {
    contextAddress = context == 4 ? 0x3C : 0x3D; 
    contextDevice = I2C1;
    S_BIT(AFIO_BASE->MAPR,1);
  }
  
  imsg.addr = contextAddress;
  imsg.length = displayCommands.count;
  imsg.data = (uint8_t*)displayCommands.buf;
  i2c_master_xfer(contextDevice,&imsg,1,0);
  while(contextDevice->state == I2C_STATE_BUSY){
    // io_mux_irq();
    delay(2);
  }
  
}

void MINIM::ControlSurface::forceDrawContext(int ctx){
  context = ctx % CONTEXT_MAX;
  populateContextBuffer();
}

void parseHook(void* env, const char *command, int where){
  LOG("onParse @");
  LOG(where);
  LOG(command);
  LOG(" ");
}
void parseLogger(const char* message){
  LOG(message);
}

void MINIM::ControlSurface::collectMidi(uint8_t* b, int blen, int offset){
  
  LOG(">");
  for(int i = offset; i<blen; i++){
    LOGI("[");
    LOGI(i);
    LOGI("]");
    LOGI("(");
    LOGI(midiString.count);
    LOGI(")");
    LOGI(":");
    LOG(b[i]);

    unsigned char cbyte = b[i];

    //auto begin sysex collect
    if(cbyte == CMD_SYSEX_START){
      sysex = true;
      altering = 0;
      sarray_push(midiString, cbyte);
      continue;
    }
    
    //auto cancel sysex collect and parse string
    if((cbyte & CMD_ANY_STATUS_BYTE) && sysex){
      sysex = false;
      altering = 0; 
      // if(cbyte & CMD_SYMBOL_ALTER && !altering && !sysex){
      //   altering = 1;
      //   sarray_clear(midiString);
      //   sarray_push(midiString, cbyte);
      // }
      if(cbyte == CMD_SYSEX_END){
        sarray_push(midiString, cbyte);
      }
      LOG("display list");
      for(int jj=0; jj<midiString.count; jj++){
        LOG(midiString.buf[jj]);
      }

      API::ParseArgs parseargs = {0};
      parseargs.logger = parseLogger;
      int a = cs.parseMidiStream((uint8_t*)midiString.buf,midiString.count,parseargs);
      LOG("list done");
      LOG(a);
      sarray_clear(midiString);
      continue;
    }



    if((cbyte & CMD_SYMBOL_ALTER) && !altering && !sysex){
      altering = 1;
      LOG("Altering start");
      sarray_clear(midiString);
      sarray_push(midiString, cbyte);
    }
    else if(sysex || (altering > 0 && altering < 3)){
      sarray_push(midiString, cbyte);
      if(altering)
        altering++;
    }
    if(altering == 3){
      LOG("Altering parse");
      cs.parseMidiStream((uint8_t*)midiString.buf,midiString.count);
      altering = 0;
    }
    
  }
}

void MINIM::ControlSurface::pollDisplays() {
  for(int i=0; i<CONTEXT_MAX; i++){
    if(cs.updateContext(i)){
      forceDrawContext(i);
    }
  }
}

void MINIM::ControlSurface::pollControls(hw_t & ctrls){
  if(ctrls.bscan_down){
    auto n = midiBase;
    for(int i=0; i<11; i++){
      if((1<<i)&ctrls.bscan_down){
        n+=i;
        usbmidi.sendNoteOn(0,n,127);
          Serial.write(0x90);
          Serial.write(n);
          Serial.write(127);
        break;
      }
    }
    ctrls.bscan_down = 0;
  }
  if(ctrls.bscan_up){
    auto n = midiBase;
    for(int i=0; i<11; i++){
      if((1<<i)&ctrls.bscan_up){
        n+=i;
        usbmidi.sendNoteOff(0,n,0);
          Serial.write(0x80);
          Serial.write(n);
          Serial.write(0);
        break;
      }
    }
    ctrls.bscan_up = 0;
  }
  if(io.turns_right){
    io.turns_right = 0;
    usbmidi.sendControlChange(0,1,65);
      Serial.write(0xB0);
      Serial.write(1);
      Serial.write(65);
  }
  if(io.turns_left){
    io.turns_left = 0;
    usbmidi.sendControlChange(0,1,63);
      Serial.write(0xB0);
      Serial.write(1);
      Serial.write(63);
  }
}

void MINIM::ControlSurface::logDisplayList(int ctx){
  LOG("---");
  for(int i=0; i<cs.cmdList[ctx].getCount(); i++){
    LOG(cs.cmdList[ctx].getCommandAt(i));
  }
  LOG("---");
}