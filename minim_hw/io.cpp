#include "io.h"


hw_t io;

void io_mux_init(){
  
  io.row = 0;
  io.op = 0;
  disableDebugPorts();
  gpio_set_mode(GPIOB, 4, GPIO_OUTPUT_PP);
  gpio_set_mode(GPIOB, 3, GPIO_OUTPUT_PP);

  gpio_set_mode(GPIOA, 0, GPIO_INPUT_PD);
  gpio_set_mode(GPIOA, 1, GPIO_INPUT_PD);
  gpio_set_mode(GPIOA, 2, GPIO_INPUT_PD);
  gpio_set_mode(GPIOA, 3, GPIO_INPUT_PD);
  gpio_set_mode(GPIOA, 4, GPIO_INPUT_PD);

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
    
    auto ii = (5*(1-io.row));

    io.bstate_old = io.bstate;
    io.bstate &= ~(0x1f<<ii);
    io.bstate |= (aa<<ii);
    io.bscan_down |= (io.bstate & (~io.bstate_old));
    io.bscan_up |= (io.bstate_old & (~io.bstate));
    
    io.row = (io.row+1) % 2;
}