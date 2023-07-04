#include "include/io.h"
#include "include/common.h"
#include "include/util.h"
#include <USBCompositeSerial.h>
#include <libmaple/timer.h>
#include <libmaple/dma.h>
#include <libmaple/gpio.h>
#include <libmaple/spi.h>
#include <libmaple/delay.h>

hw_t io;
extern USBCompositeSerial logger;

void io_mux_init(){
  
  io.row = 0;
  io.op = 0;
  disableDebugPorts();
  gpio_set_mode(GPIOB, 3, GPIO_OUTPUT_PP);//TOP
  gpio_set_mode(GPIOB, 4, GPIO_OUTPUT_PP);//BOT
  gpio_set_mode(GPIOB, 5, GPIO_OUTPUT_PP);//Encoder

  gpio_set_mode(GPIOA, 0, GPIO_INPUT_PD);//RHS
  gpio_set_mode(GPIOA, 1, GPIO_INPUT_PD);
  gpio_set_mode(GPIOA, 2, GPIO_INPUT_PD);
  gpio_set_mode(GPIOA, 3, GPIO_INPUT_PD);
  gpio_set_mode(GPIOA, 4, GPIO_INPUT_PD);//LHS

  //48M / 48 / 2000 = 25hz*20keys
  timer_set_prescaler(TIMER3, 48);
  timer_set_reload(TIMER3, 2000);
  timer_set_compare(TIMER3, TIMER_CH1, 0);
  timer_attach_interrupt(TIMER3, TIMER_UPDATE_INTERRUPT, io_mux_irq);
  timer_enable_irq(TIMER3, TIMER_UPDATE_INTERRUPT);
  timer_resume(TIMER3);
}

void io_mux_irq(){
  //3 = top
  //4 = bot
  gpio_write_bit(GPIOB, io.row+3, 1);
  uint32_t aa = GPIOA->regs->IDR & 0x1f;
  //software order swap of button wiring, left to right with uncommented
  aa = ((aa&1)<<4) | ((aa&2)<<2) | (aa&4) | ((aa&8)>>2) | ((aa&16)>>4);
  gpio_write_bit(GPIOB, io.row+3, 0);
  
  if(io.row != 2){
    auto ii = (5*(1-io.row));
    io.bstate_old = io.bstate;
    io.bstate &= ~(0x1f<<ii);
    io.bstate |= (aa<<ii);
    io.bscan_down |= (io.bstate & (~io.bstate_old));
    io.bscan_up |= (io.bstate_old & (~io.bstate));
  }
  else{
    io.turns_state = (io.turns_state << 2) | (aa&0x3);
    if((io.turns_state&0xf) == 0b1011){
      io.turns_right++;
      // usbmidi.sendControlChange(0,1,65);
    }
    if((io.turns_state&0xf) == 0b0111){
      io.turns_left++;
      // usbmidi.sendControlChange(0,1,63);
    }
    io.bstate_old = io.bstate;
    io.bstate &= ~(0x400);
    io.bstate |= ((aa&0x4)<<8);
    io.bscan_down |= (io.bstate & (~io.bstate_old));
    io.bscan_up |= (io.bstate_old & (~io.bstate));
  }

  // if(io.bscan_down){
  //   auto n = 36;
  //   for(int i=0; i<11; i++){
  //     if((1<<i)&io.bscan_down){
  //       n+=i;
  //       usbmidi.sendNoteOn(0,n,127);
  //       break;
  //     }
  //   }
  //   io.bscan_down = 0;
  // }
  // if(io.bscan_up){
  //   auto n = 36;
  //   for(int i=0; i<11; i++){
  //     if((1<<i)&io.bscan_up){
  //       n+=i;
  //       usbmidi.sendNoteOff(0,n,0);
  //       break;
  //     }
  //   }
  //   io.bscan_up = 0;
  // }
  io.row = (io.row+1) % 3;
}
