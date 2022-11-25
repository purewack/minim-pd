#include "io.h"

hw_t io;

void io_mux_init(){
  if(io.inited) return;
  io.inited = 1;
  gpio_set_mode(GPIOA, 14, GPIO_OUTPUT_PP);
  gpio_set_mode(GPIOA, 15, GPIO_OUTPUT_PP);
  gpio_set_mode(GPIOB, 3, GPIO_OUTPUT_PP);
  gpio_set_mode(GPIOB, 4, GPIO_OUTPUT_PP);
  gpio_set_mode(GPIOB, 8, GPIO_OUTPUT_PP);
  gpio_set_mode(GPIOB, 9, GPIO_OUTPUT_PP);

  gpio_set_mode(GPIOB, 11, GPIO_INPUT_PD);
  gpio_set_mode(GPIOB, 10, GPIO_INPUT_PD);
  gpio_set_mode(GPIOB, 1, GPIO_INPUT_PD);
  gpio_set_mode(GPIOB, 0, GPIO_INPUT_PD);

  gpio_set_mode(GPIOA, 8, GPIO_INPUT_PD);

  //48M / 48 / 2000 = 25hz*20keys
  timer_set_prescaler(TIMER3, 48);
  timer_set_reload(TIMER3, 1000);
  timer_set_compare(TIMER3, TIMER_CH1, 0);
  timer_attach_interrupt(TIMER3, TIMER_UPDATE_INTERRUPT, io_mux_irq);
  timer_enable_irq(TIMER3, TIMER_UPDATE_INTERRUPT);
  timer_resume(TIMER3);
}

void io_mux_irq(){

  if(io.op == 0){
    io.ok = gpio_read_bit(GPIOA, 8)>>8;
    gpio_write_bit(io.row < 2 ? GPIOA : GPIOB, io.seq_row[io.row], 1);
    uint32_t a = GPIOB->regs->IDR;
    gpio_write_bit(io.row < 2 ? GPIOA : GPIOB, io.seq_row[io.row], 0);
    uint32_t ii = io.row*4;
    uint32_t readbyte = ((a&0x800)>>11) | ((a&0x400)>>9) | ((a&0x2)<<1) | ((a&0x1)<<3);
    io.bstate_old = io.bstate;
    io.bstate &= ~(0xf<<ii);
    io.bstate |= (readbyte<<ii);
    io.bscan_down |= (io.bstate & (~io.bstate_old));
    io.bscan_up |= (io.bstate_old & (~io.bstate));
    
    io.row = (io.row+1)%5;
    io.op = 1;
    return;
  }

  if(io.op == 1){
    //encoder
    gpio_write_bit(GPIOB, 9, 1);
    uint32_t a = GPIOB->regs->IDR;
    gpio_write_bit(GPIOB, 9, 0);
    io.turns_state = (io.turns_state<<2) | ((a&0x800)>>11) | ((a&0x400)>>9);
    if((io.turns_state&0xf) == 0b1011) io.turns_right++;
    if((io.turns_state&0xf) == 0b0111) io.turns_left++;

    io.op = 0;
  }

  io.op = 0;
  // auto a = GPIOB->regs->IDR;
  // io.io[20].state = (a&(1<<11))>>11;
  // io.io[21].state = (a&(1<<10))>>10;
  // gpio_write_bit(GPIOB, 9, 0);

  // io.turns_state = (io.io[20].state<<0) | (io.io[21].state<<1) | (io.io[20].state_old<<2) | (io.io[21].state_old<<3);
  // io.io[20].state_old = io.io[20].state;
  // io.io[21].state_old = io.io[21].state;

  // if(io.turns_state == 0b0001){
  //   if(io.turns < 0) io.turns = 0;
  //   io.turns++;
  // }
  // else if(io.turns_state == 0b1101){
  //   if(io.turns > 0) io.turns = 0;
  //   io.turns--;
  // }
}