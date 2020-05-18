#ifndef __SIFIVE_UART_H__
#define __SIFIVE_UART_H__

#include <stdint.h>
#include <stdbool.h>

#define SIFIVE_UART0_CTRL_ADDR 0x54000000

#ifndef SIFIVE_UART_CLOCK_RATE
#define SIFIVE_UART_CLOCK_RATE 0x00011000
#endif
#ifndef SIFIVE_UART_DEFAULT_BAUD_RATE
#define SIFIVE_UART_DEFUALT_BAUD_RATE 115200
#endif

#define SIFIVE_UART_TXEN 1
#define SIFIVE UART_TXSTOP 2
#define SIFIVE_UART_RXEN 1

//#define ALIGN_UART
#ifdef ALIGN_UART
#define UART_ALIGNMENT __attribute__ ((aligned (4)))
#else
#define UART_ALIGNMENT
#endif

typedef struct UART_ALIGNMENT sifive_uart_pio
{
  // 0x000
  volatile int UART_REG_TXFIFO UART_ALIGNMENT;

  // 0x0004
  volatile int UART_REG_RXFIFO UART_ALIGNMENT;

  // 0x008
  volatile int UART_REG_TXCTRL UART_ALIGNMENT;

  // 0x00c
  volatile int UART_REG_RXCTRL UART_ALIGNMENT;

  // 0x010
  volatile int UART_REG_IE UART_ALIGNMENT;

  // 0x014
  volatile int UART_REG_IP UART_ALIGNMENT;

  // 0x018
  volatile uint32_t UART_REG_DIV UART_ALIGNMENT;

} sifive_uart_pio_t;


void init_sifive_uart(sifive_uart_pio_t* uart);

bool rx_ready(sifive_uart_pio_t* uart);
void tx(sifive_uart_pio_t* uart, uint8_t byte);
uint8_t rx(sifive_uart_pio_t* uart);

#endif
