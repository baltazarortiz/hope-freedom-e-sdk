#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include "weak_under_alias.h"
#include "uart.h"

ssize_t __wrap_read(int fd, void* ptr, size_t len)
{
  char *p = ptr;
  size_t jj;

  if (isatty(fd)) {
    for (jj = 0; jj < len; jj++) {
      p[jj] = uart0_rxchar();
      if (p[jj] == '\r') {
        break;
      }
    }
    return jj;
  }

  return -1;
}
weak_under_alias(read);
