#ifndef UART_H
#define UART_H

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "sifive_uart.h"

//typedef ns16550_pio_t uart_t;
typedef sifive_uart_pio_t uart_t;

static uart_t* const pex_uart = (uart_t*)(SIFIVE_UART0_CTRL_ADDR);

/* The pex uart must be initialized before any other function is called! */
void init_pex_uart();

/**
   Reads a 32bit word from the uart.  This method will block if no input is pending.
 */
uint32_t read_uint32();
uint32_t fake_read_uint32(uint8_t* fake_input, int* loc);

// For BSP
void uart0_init();
uint8_t uart0_rxchar();
void uart0_txchar(uint8_t c);

#endif
