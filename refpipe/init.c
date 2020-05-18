#include <stdint.h>
#include "uart.h"

extern int main(void);

void _init(void)
{
  int result;

  uart0_init();

  // set up floating point computation
  uintptr_t misa, mstatus;
  asm volatile("csrr %0, misa" : "=r"(misa));
  if (misa & (1 << ('f' - 'a'))) {
    asm volatile("csrr %0, mstatus" : "=r"(mstatus));
    mstatus |= 0x6000;
    asm volatile("csrw mstatus, %0; csrw frm, 0" :: "r"(mstatus));
  }

  result = main();

  // isp_run_app should exit upon seeing this message
  printf("Program exited with code: %d\n", result);

  for(;;);
}
