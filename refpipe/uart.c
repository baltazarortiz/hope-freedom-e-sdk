#include "uart.h"

void init_pex_uart() {
  //init_ns16550(pex_uart);
  init_sifive_uart(pex_uart);
}

void uart0_init() {
	init_pex_uart();
}

/**
   Reads a 32bit word from the uart.  This method will block if no input is pending.
 */
uint32_t read_uint32() {
  int i;
  uint32_t res = 0;
  for (i = 0; i < 4; i++) {
    printf("Word %d/4\n", i);
    //uint8_t b;
    //while (!rx_ready(pex_uart));
    uint8_t b = (uint8_t)rx(pex_uart);
    res |= b << (i * 8);
  }
  return res;
}

uint32_t fake_read_uint32(uint8_t* fake_input, int* loc) {
  int i;
  uint32_t res = 0;
  for (i = 0; i < 4; i++) {
    //uint8_t b;
    //while (!rx_ready(pex_uart));
    uint8_t b = fake_input[*loc];
    (*loc)++;
    res |= b << (i * 8);
  }
  return res;
}

uint8_t uart0_rxchar() {
	rx(pex_uart);
}

void uart0_txchar(uint8_t c) {
	tx(pex_uart, c);
}
