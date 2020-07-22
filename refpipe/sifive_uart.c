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
