#ifndef UART_H
#define UART_H

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#ifdef __ASSEMBLER__
#define _AC(X,Y)        X
#define _AT(T,X)        X
#else
#define _AC(X,Y)        (X##Y)
#define _AT(T,X)        ((T)(X))
#endif /* !__ASSEMBLER__*/

#define _BITUL(x)       (_AC(1,UL) << (x))
#define _BITULL(x)      (_AC(1,ULL) << (x))

#define UART0_CTRL_ADDR _AC(0x54000000,UL)

#define SIFIVE_UART_CLOCK_RATE 0x00011000
#define SIFIVE_UART_DEFUALT_BAUD_RATE 115200

/* Register offsets */
#define UART_REG_TXFIFO         0x00
#define UART_REG_RXFIFO         0x04
#define UART_REG_TXCTRL         0x08
#define UART_REG_RXCTRL         0x0c
#define UART_REG_IE             0x10
#define UART_REG_IP             0x14
#define UART_REG_DIV            0x18

/* TXCTRL register */
#define UART_TXEN               0x1
#define UART_TXWM(x)            (((x) & 0xffff) << 16)

/* RXCTRL register */
#define UART_RXEN               0x1
#define UART_RXWM(x)            (((x) & 0xffff) << 16)

/* IP register */
#define UART_IP_TXWM            0x1
#define UART_IP_RXWM            0x2


// Helper functions
#define _REG32(p, i) (*(volatile uint32_t *) ((p) + (i)))
#define UART0_REG(offset) _REG32(UART0_CTRL_ADDR, offset)

void uart0_init();

#endif
