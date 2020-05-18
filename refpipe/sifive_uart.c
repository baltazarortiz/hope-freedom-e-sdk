#include "sifive_uart.h"

void init_sifive_uart(sifive_uart_pio_t* uart) {
  int baud_rate = SIFIVE_UART_DEFUALT_BAUD_RATE;
  int clock_rate = SIFIVE_UART_CLOCK_RATE;

  uint32_t divisor = (clock_rate / baud_rate) - 1;

  uart->UART_REG_IE = 0;
  uart->UART_REG_DIV = divisor;
  uart->UART_REG_TXCTRL = SIFIVE_UART_TXEN;
  uart->UART_REG_RXCTRL = SIFIVE_UART_RXEN;
}

bool rx_ready(sifive_uart_pio_t* uart) {
  //XXX rx just returns -1 if nothing was ready
  return 1;
}

void tx(sifive_uart_pio_t* uart, uint8_t byte) {
  while (uart->UART_REG_TXFIFO < 0)
    ;  // nothing

  uart->UART_REG_TXFIFO = byte;
}

uint8_t rx(sifive_uart_pio_t* uart) {
  uint32_t rx_val;
  bool is_empty = true;
  do {
    rx_val = uart->UART_REG_RXFIFO;
    printf("waiting...\n");
    is_empty = (rx_val & (1<<31)) >> 31;
  } while(is_empty);

  uint8_t ch = rx_val & 0xff;

  if (ch < 0) return -1;

  return ch;
}

