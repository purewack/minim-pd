#pragma once
#include <stdint.h>
#include <libmaple/libmaple_types.h>
#include <libmaple/timer.h>
#include <libmaple/dma.h>
#include <libmaple/gpio.h>
#include <libmaple/spi.h>
#include <libmaple/delay.h>

struct hw_t
{
  #define S_BIT(n,b) n |= (1<<(b))
  #define C_BIT(n,b) n &= ~(1<<(b))
  #define BUTTON(b) (1<<(b))
  uint32_t bstate;
  uint32_t bstate_old;
  uint32_t bscan_down;
  uint32_t bscan_up;
  uint8_t ok;
  int turns_left;
  int turns_right;
  int turns_state;
  const uint8_t seq_row[6] = {14,15,3,4,8,9};
  uint8_t row;
  uint8_t op;
  uint8_t inited = 0;
};

extern hw_t io;

void io_mux_init();
void io_mux_irq();