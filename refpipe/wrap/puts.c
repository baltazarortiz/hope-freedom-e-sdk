#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include "uart.h"
#include "weak_under_alias.h"

int __wrap_puts(const char *s)
{
  while (*s != '\0') {
    while (UART0_REG(UART_REG_TXFIFO) & 0x80000000) ;
    UART0_REG(UART_REG_TXFIFO) = *s;

    if (*s == '\n') {
      while (UART0_REG(UART_REG_TXFIFO) & 0x80000000) ;
      UART0_REG(UART_REG_TXFIFO) = '\r';
    }

    ++s;
  }

  return 0;
}
weak_under_alias(puts);
