#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include "weak_under_alias.h"
#include "uart.h"
//#include "stub.h"

ssize_t __wrap_write(int fd, const void* ptr, size_t len)
{
  const uint8_t* current = (const uint8_t*)ptr;

  if (isatty(fd)) {
    for (size_t jj = 0; jj < len; jj++) {
      while (UART0_REG(UART_REG_TXFIFO) & 0x80000000) ;
      UART0_REG(UART_REG_TXFIFO) = current[jj];

      if (current[jj] == '\n') {
        while (UART0_REG(UART_REG_TXFIFO) & 0x80000000) ;
        UART0_REG(UART_REG_TXFIFO) = '\r';
      }
    }
    return len;
  }

  return -1;
  //return _stub(EBADF);
}
weak_under_alias(write);
